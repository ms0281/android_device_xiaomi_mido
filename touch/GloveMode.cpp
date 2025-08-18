/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch-service.glovemode"

#include "GloveMode.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

using ::android::base::ReadFileToString;
using ::android::base::Trim;
using ::android::base::WriteStringToFile;

namespace aidl {
namespace vendor {
namespace lineage {
namespace touch {

static constexpr const char kControlPath[] =
        "/sys/class/tp_glove/device/glove_enable";

GloveMode::GloveMode() {}

ndk::ScopedAStatus GloveMode::getEnabled(bool* _aidl_return) {
    std::string buf;
    if (!ReadFileToString(kControlPath, &buf, true)) {
        LOG(ERROR) << "Failed to read from " << kControlPath;
        *_aidl_return = false;
        return ndk::ScopedAStatus::ok();
    }
    *_aidl_return = (std::stoi(Trim(buf)) == 1);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus GloveMode::setEnabled(bool enabled) {
    if (!WriteStringToFile(enabled ? "1" : "0", kControlPath, true)) {
        LOG(ERROR) << "Failed to write to " << kControlPath;
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    return ndk::ScopedAStatus::ok();
}

}  // namespace touch
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
