// Copyright (c) 2026 local port contributors. BSD-3-Clause.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

class KernelPatcher;

namespace XeMetalLeak {

void processKernel(KernelPatcher &p);

// AppleIntelTGLGraphicsMTLDriver 16.0.32 -[MTLIGAccelCommandBuffer dealloc] (0x7ecba):
// Queue m_pPoshUsedAsTarget is never populated, so "only active" command buffers
// allocate a private 0x12010-byte _pPoshUsedAsTarget buffer while keeping
// m_isOnlyActiveCommandBuffer == 1, then skip operator delete at 0x7ed87 (75 0d).
constexpr uint64_t TargetPageOffset = 0x7E000ULL;
constexpr size_t PageBytes = 0x1000U;
constexpr size_t SiteOffsetInPage = 0xD6BU;
constexpr size_t BranchOffsetInSite = 28U;

constexpr uint8_t ExpectedSite[] {
    0x48, 0x8B, 0x1D, 0x3E, 0xB1, 0x41, 0x00, // movq _pPoshUsedAsTarget(%rip), %rbx
    0x49, 0x8B, 0x3C, 0x1E,                   // movq (%r14,%rbx), %rdi
    0x48, 0x85, 0xFF,                         // testq %rdi, %rdi
    0x74, 0x1B,                               // je 0x7ed96
    0x48, 0x8B, 0x05, 0x26, 0xB1, 0x41, 0x00, // movq m_isOnlyActiveCommandBuffer(%rip), %rax
    0x41, 0x80, 0x3C, 0x06, 0x00,             // cmpb $0x0, (%r14,%rax)
    0x75, 0x0D,                               // jne 0x7ed96 -> patched to 90 90
    0xE8, 0x7A, 0xFA, 0x01, 0x00,             // callq __ZdlPv
    0x49, 0xC7, 0x04, 0x1E, 0x00, 0x00, 0x00, 0x00 // movq $0x0, (%r14,%rbx)
};

static_assert(sizeof(ExpectedSite) == 43, "exact MTLIGAccelCommandBuffer dealloc site length");
static_assert(ExpectedSite[BranchOffsetInSite] == 0x75 && ExpectedSite[BranchOffsetInSite + 1] == 0x0D,
    "expected jne rel8 at branch offset");

inline bool eligiblePage(uint64_t pageOffset, int validated, int tainted) {
    return pageOffset == TargetPageOffset && validated != 0 && tainted == 0;
}

inline bool targetPath(const char *path) {
    if (!path) return false;
    constexpr const char data[] = "/System/Volumes/Data";
    if (!strncmp(path, data, sizeof(data) - 1)) path += sizeof(data) - 1;
    return !strcmp(path,
        "/Library/GPUBundles/AppleIntelTGLGraphicsMTLDriver.bundle/Contents/MacOS/AppleIntelTGLGraphicsMTLDriver");
}

inline bool matchesUnpatchedSite(const void *page, size_t pageSize = PageBytes) {
    if (!page || pageSize < SiteOffsetInPage + sizeof(ExpectedSite)) return false;
    return !memcmp(static_cast<const uint8_t *>(page) + SiteOffsetInPage, ExpectedSite, sizeof(ExpectedSite));
}

inline bool matchesPatchedSite(const void *page, size_t pageSize = PageBytes) {
    if (!page || pageSize < SiteOffsetInPage + sizeof(ExpectedSite)) return false;
    const auto *site = static_cast<const uint8_t *>(page) + SiteOffsetInPage;
    return !memcmp(site, ExpectedSite, BranchOffsetInSite) &&
        site[BranchOffsetInSite] == 0x90 && site[BranchOffsetInSite + 1] == 0x90 &&
        !memcmp(site + BranchOffsetInSite + 2, ExpectedSite + BranchOffsetInSite + 2,
            sizeof(ExpectedSite) - BranchOffsetInSite - 2);
}

inline void applySitePatch(uint8_t *page) {
    page[SiteOffsetInPage + BranchOffsetInSite] = 0x90;
    page[SiteOffsetInPage + BranchOffsetInSite + 1] = 0x90;
}

} // namespace XeMetalLeak
