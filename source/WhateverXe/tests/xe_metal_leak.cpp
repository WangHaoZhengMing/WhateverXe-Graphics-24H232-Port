// Copyright (c) 2026 local port contributors. BSD-3-Clause.
#include "kern_xe_metal_leak.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>

int main() {
    using namespace XeMetalLeak;

    // 1. Offset, validation and taint gates.
    assert(eligiblePage(0x7E000ULL, 1, 0));
    assert(eligiblePage(0x7E000ULL, 3, 0));
    assert(!eligiblePage(0x7D000ULL, 1, 0));
    assert(!eligiblePage(0x7F000ULL, 1, 0));
    assert(!eligiblePage(0x7E000ULL, 0, 0));
    assert(!eligiblePage(0x7E000ULL, 1, 1));

    // 2. Exact path admission across root firmlink and Data mount, rejecting siblings/copies.
    assert(!targetPath(nullptr));
    assert(targetPath(
        "/Library/GPUBundles/AppleIntelTGLGraphicsMTLDriver.bundle/Contents/MacOS/AppleIntelTGLGraphicsMTLDriver"));
    assert(targetPath(
        "/System/Volumes/Data/Library/GPUBundles/AppleIntelTGLGraphicsMTLDriver.bundle/Contents/MacOS/AppleIntelTGLGraphicsMTLDriver"));
    for (const char *rejected : {
        "",
        "/Library/GPUBundles/AppleIntelTGLGraphicsMTLDriver.bundle/Contents/MacOS/impostor.dylib",
        "/Library/GPUBundles/AppleIntelTGLGraphicsMTLDriver.bundle/Contents/MacOS/descriptor.dylib",
        "/Library/GPUBundles/AppleIntelTGLGraphicsMTLDriver.bundle/Contents/MacOS/AppleIntelTGLGraphicsMTLDriver.bak",
        "/tmp/AppleIntelTGLGraphicsMTLDriver",
        "/System/Volumes/Data/tmp/AppleIntelTGLGraphicsMTLDriver",
    }) {
        assert(!targetPath(rejected));
    }

    // 3. Exact 43-byte instruction signature verification and 2-byte NOP replacement.
    std::array<uint8_t, PageBytes> page {};
    assert(!matchesUnpatchedSite(nullptr));
    assert(!matchesPatchedSite(nullptr));
    assert(!matchesUnpatchedSite(page.data()));
    assert(!matchesPatchedSite(page.data()));

    std::memcpy(page.data() + SiteOffsetInPage, ExpectedSite, sizeof(ExpectedSite));
    assert(matchesUnpatchedSite(page.data()));
    assert(!matchesPatchedSite(page.data()));
    assert(!matchesUnpatchedSite(page.data(), SiteOffsetInPage + sizeof(ExpectedSite) - 1));

    for (size_t i = 0; i < sizeof(ExpectedSite); ++i) {
        page[SiteOffsetInPage + i] ^= 0x01U;
        assert(!matchesUnpatchedSite(page.data()));
        page[SiteOffsetInPage + i] ^= 0x01U;
    }

    applySitePatch(page.data());
    assert(!matchesUnpatchedSite(page.data()));
    assert(matchesPatchedSite(page.data()));
    assert(page[0xD87] == 0x90 && page[0xD88] == 0x90);
    // Surrounding instructions (cmpb before, callq __ZdlPv after) remain untouched.
    assert(page[0xD86] == 0x00 && page[0xD89] == 0xE8);

    // 4. If installed on host, verify the real driver page at 0x7E000 matches.
    std::ifstream hostDriver(
        "/Library/GPUBundles/AppleIntelTGLGraphicsMTLDriver.bundle/Contents/MacOS/AppleIntelTGLGraphicsMTLDriver",
        std::ios::binary);
    if (hostDriver.good()) {
        std::array<uint8_t, PageBytes> realPage {};
        hostDriver.seekg(static_cast<std::streamoff>(TargetPageOffset));
        hostDriver.read(reinterpret_cast<char *>(realPage.data()), static_cast<std::streamsize>(realPage.size()));
        assert(hostDriver.gcount() == static_cast<std::streamsize>(realPage.size()));
        assert(matchesUnpatchedSite(realPage.data()));
        applySitePatch(realPage.data());
        assert(matchesPatchedSite(realPage.data()));
    }

    std::puts("PASS: Metal POSH command-buffer leak runtime page patch contracts verified");
    return 0;
}
