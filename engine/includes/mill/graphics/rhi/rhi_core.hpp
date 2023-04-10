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
}