/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch-service.keydisabler"

#include "KeyDisabler.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>
#include <unistd.h>

using ::android::base::ReadFileToString;
using ::android::base::Trim;
using ::android::base::WriteStringToFile;

namespace aidl {
namespace vendor {
namespace lineage {
namespace touch {

static constexpr const char kControlPath[] =
        "/proc/touchpanel/capacitive_keys_disable";

KeyDisabler::KeyDisabler() {
    has_key_disabler_ = (access(kControlPath, F_OK) == 0);
}

ndk::ScopedAStatus KeyDisabler::getEnabled(bool* _aidl_return) {
    if (!has_key_disabler_) {
        *_aidl_return = false;
        return ndk::ScopedAStatus::ok();
    }

    std::string buf;
    if (!ReadFileToString(kControlPath, &buf, true)) {
        LOG(ERROR) << "Failed to read from " << kControlPath;
        *_aidl_return = false;
        return ndk::ScopedAStatus::ok();
    }

    *_aidl_return = (Trim(buf) == "1");
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus KeyDisabler::setEnabled(bool enabled) {
    if (!has_key_disabler_) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

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
