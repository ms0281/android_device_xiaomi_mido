/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 *                         2025 KamiKaonashi
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.livedisplay-service.xiaomi_mido"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include <dlfcn.h>
#include "PictureAdjustment.h"

using ::aidl::vendor::lineage::livedisplay::sdm::PictureAdjustment;

int main() {
    void* libHandle = nullptr;
    const char* libName = "libsdm-disp-vndapis.so";
    int32_t (*disp_api_init)(uint64_t*, uint32_t) = nullptr;
    int32_t (*disp_api_deinit)(uint64_t, uint32_t) = nullptr;
    uint64_t cookie = 0;

    android::ProcessState::self()->setThreadPoolMaxThreadCount(1);
    android::ProcessState::self()->startThreadPool();

    LOG(INFO) << "LiveDisplay AIDL HAL service is starting.";

    libHandle = dlopen(libName, RTLD_NOW);
    if (!libHandle) {
        LOG(ERROR) << "Failed to load " << libName;
        return 1;
    }

    disp_api_init = reinterpret_cast<int32_t (*)(uint64_t*, uint32_t)>(
            dlsym(libHandle, "disp_api_init"));
    disp_api_deinit = reinterpret_cast<int32_t (*)(uint64_t, uint32_t)>(
            dlsym(libHandle, "disp_api_deinit"));

    if (!disp_api_init || !disp_api_deinit || disp_api_init(&cookie, 0) != 0) {
        LOG(ERROR) << "Failed to init SDM display interface";
        if (libHandle) dlclose(libHandle);
        return 1;
    }

    auto pa = ndk::SharedRefBase::make<PictureAdjustment>(libHandle, cookie);
    if (!pa->isSupported()) {
        LOG(ERROR) << "PictureAdjustment not supported, quitting.";
        disp_api_deinit(cookie, 0);
        dlclose(libHandle);
        return 1;
    }

    std::string instance = std::string() + PictureAdjustment::descriptor + "/default";
    if (AServiceManager_addService(pa->asBinder().get(), instance.c_str()) != STATUS_OK) {
        LOG(ERROR) << "Failed to register PictureAdjustment AIDL service";
        return 1;
    }

    LOG(INFO) << "LiveDisplay AIDL HAL service ready";
    ABinderProcess_joinThreadPool();

    // Cleanup if threadpool exits (shouldn’t)
    disp_api_deinit(cookie, 0);
    if (libHandle) dlclose(libHandle);

    return 1;
}
