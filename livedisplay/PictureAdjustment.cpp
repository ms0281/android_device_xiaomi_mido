/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 *                         2025 KamiKaonashi
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PictureAdjustment.h"

#include <android-base/logging.h>
#include <dlfcn.h>
#include <algorithm>

namespace {

struct sdm_feature_version {
    uint8_t x, y;
    uint16_t z;
};

struct hsic_data {
    int32_t hue;
    float saturation;
    float intensity;
    float contrast;
    float saturationThreshold;
};

struct hsic_config {
    uint32_t unused;
    hsic_data data;
};

struct hsic_int_range {
    int32_t max;
    int32_t min;
    uint32_t step;
};

struct hsic_float_range {
    float max;
    float min;
    float step;
};

struct hsic_ranges {
    uint32_t unused;
    struct hsic_int_range hue;
    struct hsic_float_range saturation;
    struct hsic_float_range intensity;
    struct hsic_float_range contrast;
    struct hsic_float_range saturationThreshold;
};

static inline void sanitize_float_range(float& min, float& max, float& step, float default_step = 1.0f) {
    if (min > max) std::swap(min, max);
    if (!(step > 0.f)) step = default_step;
}

static inline void sanitize_int_range(int32_t& min, int32_t& max, float& step_out, uint32_t step_in, float default_step = 1.0f) {
    if (min > max) std::swap(min, max);
    step_out = (step_in > 0) ? static_cast<float>(step_in) : default_step;
}

}

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {
namespace sdm {

PictureAdjustment::PictureAdjustment(void* libHandle, uint64_t cookie)
      : mLibHandle(libHandle), mCookie(cookie) {
    disp_api_get_feature_version =
            reinterpret_cast<int32_t (*)(uint64_t, uint32_t, void*, uint32_t*)>(
                    dlsym(mLibHandle, "disp_api_get_feature_version"));
    disp_api_get_global_pa_range =
            reinterpret_cast<int32_t (*)(uint64_t, uint32_t, void*)>(
                    dlsym(mLibHandle, "disp_api_get_global_pa_range"));
    disp_api_get_global_pa_config =
            reinterpret_cast<int32_t (*)(uint64_t, uint32_t, uint32_t*, void*)>(
                    dlsym(mLibHandle, "disp_api_get_global_pa_config"));
    disp_api_set_global_pa_config =
            reinterpret_cast<int32_t (*)(uint64_t, uint32_t, uint32_t, void*)>(
                    dlsym(mLibHandle, "disp_api_set_global_pa_config"));
}

bool PictureAdjustment::isSupported() {
    sdm_feature_version version{};
    hsic_ranges r{};
    uint32_t flags = 0;

    static int supported = -1;
    if (supported >= 0) return supported;

    if (disp_api_get_feature_version == nullptr ||
        disp_api_get_feature_version(mCookie, 1, &version, &flags) != 0) {
        supported = 0;
        return supported;
    }

    if (version.x <= 0 && version.y <= 0 && version.z <= 0) {
        supported = 0;
        return supported;
    }

    if (disp_api_get_global_pa_range == nullptr ||
        disp_api_get_global_pa_range(mCookie, 0, &r) != 0) {
        supported = 0;
        return supported;
    }

    supported = r.hue.max != 0 && r.hue.min != 0 &&
                r.saturation.max != 0.f && r.saturation.min != 0.f &&
                r.intensity.max != 0.f && r.intensity.min != 0.f &&
                r.contrast.max != 0.f && r.contrast.min != 0.f;
    return supported;
}

ndk::ScopedAStatus PictureAdjustment::getHueRange(FloatRange* _aidl_return) {
    hsic_ranges r{};
    if (disp_api_get_global_pa_range && disp_api_get_global_pa_range(mCookie, 0, &r) == 0) {
        int32_t min_i = r.hue.min;
        int32_t max_i = r.hue.max;
        float step_f;
        sanitize_int_range(min_i, max_i, step_f, r.hue.step, 1.0f);
        _aidl_return->min = static_cast<float>(min_i);
        _aidl_return->max = static_cast<float>(max_i);
        _aidl_return->step = step_f;
    } else {
        // Provide a safe default to avoid crashes
        _aidl_return->min = 0.f;
        _aidl_return->max = 0.f;
        _aidl_return->step = 1.f;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PictureAdjustment::getSaturationRange(FloatRange* _aidl_return) {
    hsic_ranges r{};
    if (disp_api_get_global_pa_range && disp_api_get_global_pa_range(mCookie, 0, &r) == 0) {
        float min = r.saturation.min, max = r.saturation.max, step = r.saturation.step;
        sanitize_float_range(min, max, step, 0.01f);
        _aidl_return->min = min; _aidl_return->max = max; _aidl_return->step = step;
    } else {
        _aidl_return->min = 0.f; _aidl_return->max = 0.f; _aidl_return->step = 0.01f;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PictureAdjustment::getIntensityRange(FloatRange* _aidl_return) {
    hsic_ranges r{};
    if (disp_api_get_global_pa_range && disp_api_get_global_pa_range(mCookie, 0, &r) == 0) {
        float min = r.intensity.min, max = r.intensity.max, step = r.intensity.step;
        sanitize_float_range(min, max, step, 0.01f);
        _aidl_return->min = min; _aidl_return->max = max; _aidl_return->step = step;
    } else {
        _aidl_return->min = 0.f; _aidl_return->max = 0.f; _aidl_return->step = 0.01f;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PictureAdjustment::getContrastRange(FloatRange* _aidl_return) {
    hsic_ranges r{};
    if (disp_api_get_global_pa_range && disp_api_get_global_pa_range(mCookie, 0, &r) == 0) {
        float min = r.contrast.min, max = r.contrast.max, step = r.contrast.step;
        sanitize_float_range(min, max, step, 0.01f);
        _aidl_return->min = min; _aidl_return->max = max; _aidl_return->step = step;
    } else {
        _aidl_return->min = 0.f; _aidl_return->max = 0.f; _aidl_return->step = 0.01f;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PictureAdjustment::getSaturationThresholdRange(FloatRange* _aidl_return) {
    hsic_ranges r{};
    if (disp_api_get_global_pa_range && disp_api_get_global_pa_range(mCookie, 0, &r) == 0) {
        float min = r.saturationThreshold.min, max = r.saturationThreshold.max, step = r.saturationThreshold.step;
        sanitize_float_range(min, max, step, 0.01f);
        _aidl_return->min = min; _aidl_return->max = max; _aidl_return->step = step;
    } else {
        _aidl_return->min = 0.f; _aidl_return->max = 0.f; _aidl_return->step = 0.01f;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PictureAdjustment::getPictureAdjustment(HSIC* _aidl_return) {
    hsic_config config{};
    uint32_t enable = 0;
    if (disp_api_get_global_pa_config &&
        disp_api_get_global_pa_config(mCookie, 0, &enable, &config) == 0) {
        _aidl_return->hue = static_cast<float>(config.data.hue);
        _aidl_return->saturation = config.data.saturation;
        _aidl_return->intensity = config.data.intensity;
        _aidl_return->contrast = config.data.contrast;
        _aidl_return->saturationThreshold = config.data.saturationThreshold;
    } else {
        *_aidl_return = HSIC{};
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PictureAdjustment::getDefaultPictureAdjustment(HSIC* _aidl_return) {
    *_aidl_return = HSIC{};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PictureAdjustment::setPictureAdjustment(const HSIC& hsic) {
    hsic_config config = {
            0,
            {static_cast<int32_t>(hsic.hue), hsic.saturation, hsic.intensity,
             hsic.contrast, hsic.saturationThreshold}
    };
    if (disp_api_set_global_pa_config &&
        disp_api_set_global_pa_config(mCookie, 0, 1, &config) == 0) {
        return ndk::ScopedAStatus::ok();
    }
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

} // namespace sdm
} // namespace livedisplay
} // namespace lineage
} // namespace vendor
} // namespace aidl
