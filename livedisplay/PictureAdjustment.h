/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 *                         2025 KamiKaonashi
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/livedisplay/BnPictureAdjustment.h>

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {
namespace sdm {

class PictureAdjustment : public BnPictureAdjustment {
public:
    PictureAdjustment(void* libHandle, uint64_t cookie);
    bool isSupported();

    ndk::ScopedAStatus getHueRange(FloatRange* _aidl_return) override;
    ndk::ScopedAStatus getSaturationRange(FloatRange* _aidl_return) override;
    ndk::ScopedAStatus getIntensityRange(FloatRange* _aidl_return) override;
    ndk::ScopedAStatus getContrastRange(FloatRange* _aidl_return) override;
    ndk::ScopedAStatus getSaturationThresholdRange(FloatRange* _aidl_return) override;

    ndk::ScopedAStatus getPictureAdjustment(HSIC* _aidl_return) override;
    ndk::ScopedAStatus getDefaultPictureAdjustment(HSIC* _aidl_return) override;
    ndk::ScopedAStatus setPictureAdjustment(const HSIC& hsic) override;

private:
    void* mLibHandle;
    uint64_t mCookie;

    int32_t (*disp_api_get_feature_version)(uint64_t, uint32_t, void*, uint32_t*);
    int32_t (*disp_api_get_global_pa_range)(uint64_t, uint32_t, void*);
    int32_t (*disp_api_get_global_pa_config)(uint64_t, uint32_t, uint32_t*, void*);
    int32_t (*disp_api_set_global_pa_config)(uint64_t, uint32_t, uint32_t, void*);
};

} // namespace sdm
} // namespace livedisplay
} // namespace lineage
} // namespace vendor
} // namespace aidl
