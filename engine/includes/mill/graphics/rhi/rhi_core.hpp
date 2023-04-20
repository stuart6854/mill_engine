#pragma once

namespace mill::rhi
{
    bool initialise();
    void shutdown();

    /* Should be called at the start of a frame before any contexts calls begin. */
    void begin_frame();
    /*
        Should be called at the end of a frame once any context calls have been completed.
        This will submit all contexts that have been recorded to.
    */
    void end_frame();

    struct MemoryStats
    {
        /* Total Device-Local memory in Bytes. */
        u64 DeviceTotalBudget{};
        /* Used Device-Local memory in Bytes. */
        u64 DeviceTotalUsage{};
    };
    auto get_memory_stats() -> MemoryStats;
}