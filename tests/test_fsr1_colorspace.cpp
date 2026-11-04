// test_fsr1_colorspace.cpp
// CPU-only unit tests for easu_runs_this_frame logic and colorspace_perceptual gating.
// No GPU or game dump required.
#include <cassert>
#include <cstdio>

// Replicate the logic from vk_presenter.cpp so we can unit-test it in isolation.
// Parameters match: fsr1 bool, image_size (w,h), frame (w,h)
static bool easu_runs(bool fsr1, unsigned iw, unsigned ih, unsigned ow, unsigned oh) {
    return fsr1 && iw < ow && ih < oh;
}

// colorspace_perceptual as set in vk_presenter after fix:
static bool colorspace_perceptual_active(bool fsr1, unsigned iw, unsigned ih, unsigned ow, unsigned oh,
                                         bool colorspace_perceptual_config) {
    return easu_runs(fsr1, iw, ih, ow, oh) && colorspace_perceptual_config;
}

int main() {
    // --- easu_runs_this_frame logic ---

    // RCAS-only default: fsr1=false, same resolution -> easu_runs=false
    assert(!easu_runs(false, 1920, 1080, 1920, 1080));
    // RCAS-only default: fsr1=false, any size -> easu_runs=false
    assert(!easu_runs(false, 1280, 720, 1920, 1080));
    // EASU+RCAS: fsr1=true, render < output -> easu_runs=true
    assert(easu_runs(true, 1280, 720, 1920, 1080));
    // fsr1=true but same size (no upscaling needed) -> easu_runs=false
    assert(!easu_runs(true, 1920, 1080, 1920, 1080));
    // fsr1=true but output smaller than render -> easu_runs=false (would be downscale)
    assert(!easu_runs(true, 1920, 1080, 1280, 720));

    // --- colorspace_perceptual gating ---

    // RCAS-only (default config: fsr1=false, rcas=true, same res, colorspace_perceptual=true)
    // -> Sample[1] must be 0 -> no gamma decode -> no darkening
    assert(!colorspace_perceptual_active(false, 1920, 1080, 1920, 1080, true));

    // EASU+RCAS with perceptual config -> Sample[1]=1 -> gamma wrap active
    assert(colorspace_perceptual_active(true, 1280, 720, 1920, 1080, true));

    // EASU+RCAS with linear config -> Sample[1]=0 -> no gamma wrap
    assert(!colorspace_perceptual_active(true, 1280, 720, 1920, 1080, false));

    // fsr1=true but same size, perceptual config -> easu_runs=false -> Sample[1]=0
    assert(!colorspace_perceptual_active(true, 1920, 1080, 1920, 1080, true));

    printf("PASS: fsr1 colorspace easu-runs logic\n");
    return 0;
}
