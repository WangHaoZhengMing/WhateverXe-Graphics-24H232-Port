// Copyright (c) 2026 local port contributors. BSD-3-Clause.
#include "kern_xe_metal_leak.hpp"
#include <Headers/kern_api.hpp>
#include <IOKit/IORegistryEntry.h>
#include <libkern/c++/OSData.h>
#include <libkern/c++/OSSerialize.h>

namespace XeMetalLeak {
namespace {

int (*getPath)(void *, char *, int *);
mach_vm_address_t originalValidatePage;
// schema=1: [offsetSeen, targetPathSeen, alreadyPatched, patched]
uint32_t counters[4] {};

bool supportedOS() {
    const auto v = getKernelVersion();
    return v >= KernelVersion::BigSur && v <= KernelVersion::Tahoe;
}

void wrapValidatePage(void *vp, void *pager, uint64_t pageOffset, const void *data,
    int *validated, int *tainted, int *nx) {
    FunctionCast(wrapValidatePage, originalValidatePage)(vp, pager, pageOffset, data, validated, tainted, nx);
    if (pageOffset != TargetPageOffset) return;
    __atomic_fetch_add(&counters[0], 1U, __ATOMIC_RELAXED);
    if (!vp || !data || !getPath || !validated || !eligiblePage(pageOffset, *validated, tainted ? *tainted : 0))
        return;
    char path[256];
    int length = sizeof(path);
    if (getPath(vp, path, &length) || !targetPath(path)) return;
    __atomic_fetch_add(&counters[1], 1U, __ATOMIC_RELAXED);
    if (!matchesUnpatchedSite(data)) {
        if (matchesPatchedSite(data)) __atomic_fetch_add(&counters[2], 1U, __ATOMIC_RELAXED);
        return;
    }
    if (MachInfo::setKernelWriting(true, KernelPatcher::kernelWriteLock) == KERN_SUCCESS) {
        auto *mutablePage = static_cast<uint8_t *>(const_cast<void *>(data));
        if (matchesUnpatchedSite(mutablePage)) {
            applySitePatch(mutablePage);
            if (matchesPatchedSite(mutablePage))
                __atomic_fetch_add(&counters[3], 1U, __ATOMIC_RELAXED);
        }
        MachInfo::setKernelWriting(false, KernelPatcher::kernelWriteLock);
    }
}

bool serializeCounters(void *, void *, OSSerialize *serializer) {
    uint32_t out[5] {1U};
    for (unsigned i = 0; i < 4; ++i)
        out[i + 1] = __atomic_load_n(&counters[i], __ATOMIC_RELAXED);
    auto *bytes = OSData::withBytes(out, sizeof(out));
    if (!bytes) return false;
    const bool ok = bytes->serialize(serializer);
    bytes->release();
    return ok;
}

} // namespace

void processKernel(KernelPatcher &p) {
    if (!supportedOS()) return;
    getPath = reinterpret_cast<decltype(getPath)>(p.solveSymbol(KernelPatcher::KernelID, "_vn_getpath"));
    p.clearError();
    bool ready = false;
    if (getPath) {
        KernelPatcher::RouteRequest route("_cs_validate_page", wrapValidatePage, originalValidatePage);
        ready = p.routeMultipleLong(KernelPatcher::KernelID, &route, 1);
        p.clearError();
    }
    auto *root = IORegistryEntry::getRegistryRoot();
    if (root) {
        root->setProperty("WEXLocalMetalLeakFixReady", ready);
        if (auto *live = OSSerializer::forTarget(nullptr, serializeCounters)) {
            root->setProperty("WEXLocalMetalLeakFixLive", live);
            live->release();
        }
    }
    SYSLOG("wex", "Metal POSH leak runtime patch ready=%d", ready);
}

} // namespace XeMetalLeak
