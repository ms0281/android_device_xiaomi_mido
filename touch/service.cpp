/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch-service.xiaomi_mido"

#include "GloveMode.h"
#include "KeyDisabler.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using aidl::vendor::lineage::touch::GloveMode;
using aidl::vendor::lineage::touch::KeyDisabler;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    std::shared_ptr<GloveMode> gloveMode = ndk::SharedRefBase::make<GloveMode>();
    std::shared_ptr<KeyDisabler> keyDisabler = ndk::SharedRefBase::make<KeyDisabler>();

    const std::string instanceGlove =
            std::string(GloveMode::descriptor) + "/default";
    const std::string instanceKey =
            std::string(KeyDisabler::descriptor) + "/default";

    CHECK_EQ(AServiceManager_addService(gloveMode->asBinder().get(),
                                        instanceGlove.c_str()), STATUS_OK)
        << "Failed to register GloveMode HAL service.";

    CHECK_EQ(AServiceManager_addService(keyDisabler->asBinder().get(),
                                        instanceKey.c_str()), STATUS_OK)
        << "Failed to register KeyDisabler HAL service.";

    LOG(INFO) << "Touch HAL AIDL service is ready.";
    ABinderProcess_joinThreadPool();

    return EXIT_FAILURE;
}
