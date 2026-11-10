// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <utility>

#include "core/hle/service/nvdrv/devices/nx2_stub_device.h"

TEST_CASE("NX2 presentation resolves parent handles and image offsets", "[video_core][nx2_mapping]") {
    Service::Nvidia::Devices::Nx2GpuMemoryRegistry registry;
    registry.MapAt(7, 0x1000, 0x8000, 0x4000, 0x3000);
    registry.RecordDeviceRange(0x4000, 0x3000, 0x10000);
    REQUIRE(registry.GpuAddressOf(7, 0x1800) == 0x8800U);
    REQUIRE(registry.DeviceAddressOfGpu(0x8800) == 0x10800U);
    REQUIRE_FALSE(registry.GpuAddressOf(7, 0xfff));
    REQUIRE_FALSE(registry.GpuAddressOf(7, 0x4000));
    REQUIRE_FALSE(registry.GpuAddressOf(8, 0x1800));
    REQUIRE_FALSE(registry.DeviceAddressOfGpu(0xb000));
}

TEST_CASE("NX2 partial unmaps preserve remaining parent offsets", "[video_core][nx2_mapping]") {
    Service::Nvidia::Devices::Nx2GpuMemoryRegistry registry;
    registry.MapAt(7, 0x1000, 0x8000, 0x4000, 0x3000);
    registry.RecordDeviceRange(0x4000, 0x3000, 0x10000);
    registry.Unmap(0x9000, 0x1000);
    REQUIRE(registry.GpuAddressOf(7, 0x1000) == 0x8000U);
    REQUIRE_FALSE(registry.GpuAddressOf(7, 0x2000));
    REQUIRE_FALSE(registry.DeviceAddressOfGpu(0x9000));
    REQUIRE(registry.GpuAddressOf(7, 0x3000) == 0xa000U);
    REQUIRE(registry.DeviceAddressOfGpu(0xa000) == 0x12000U);
}

TEST_CASE("NX2 remaps and resets discard stale parent handles", "[video_core][nx2_mapping]") {
    Service::Nvidia::Devices::Nx2GpuMemoryRegistry registry;
    registry.MapAt(7, 0, 0x8000, 0x4000, 0x1000);
    registry.MapAt(8, 0x1000, 0x8000, 0x6000, 0x1000);
    registry.RecordDeviceRange(0x6000, 0x1000, 0x20000);
    REQUIRE_FALSE(registry.GpuAddressOf(7, 0));
    REQUIRE(registry.GpuAddressOf(8, 0x1000) == 0x8000U);
    REQUIRE(registry.DeviceAddressOfGpu(0x8000) == 0x20000U);
    registry.Reset();
    REQUIRE_FALSE(registry.GpuAddressOf(8, 0x1000));
    REQUIRE_FALSE(registry.DeviceAddressOfGpu(0x8000));
}

TEST_CASE("NX2 destroyed memory gets a new device range when registered again",
          "[video_core][nx2_mapping]") {
    Service::Nvidia::Devices::Nx2GpuMemoryRegistry registry;
    // What one address space's listener keeps between maps: device range per CPU range.
    std::map<std::pair<u64, u64>, u64> ranges;
    u64 next_device = 0x200000;
    const auto allocate = [&next_device] {
        const u64 address = next_device;
        next_device += 0x20000;
        return address;
    };
    registry.AddListener(
        &ranges,
        [&](u64 gpu_address, u64 cpu_address, u64 size) {
            const std::pair key{cpu_address, size};
            if (!ranges.contains(key)) {
                ranges[key] = registry.AcquireDeviceRange(1, cpu_address, size, allocate);
            }
            registry.RecordDeviceRange(cpu_address, size, ranges[key]);
            (void)gpu_address;
        },
        [](u64, u64) {},
        [&](u64 cpu_address, u64 size) {
            if (ranges.erase({cpu_address, size}) != 0) {
                registry.ReleaseDeviceRange(1, cpu_address, size);
            }
        });
    registry.Register(0x10000, 0x4000);
    REQUIRE(registry.DeviceAddressOf(0x10000) == 0x200000U);
    REQUIRE(registry.DeviceAddressOf(0x13000) == 0x203000U);

    // Destroyed: the GPU mapping goes first, then the backing is released.
    registry.Unmap(0x10000, 0x4000);
    registry.ReleaseBacking(0x10000, 0x4000);
    REQUIRE_FALSE(registry.DeviceAddressOf(0x10000));

    // New pages at the same CPU address: a reused device range would still read the old ones.
    registry.Register(0x10000, 0x4000);
    REQUIRE(registry.DeviceAddressOf(0x10000) == 0x220000U);
    REQUIRE(registry.DeviceAddressOf(0x13000) == 0x223000U);
}
