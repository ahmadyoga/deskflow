/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2022 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */
#pragma once

#include <Carbon/Carbon.h>
#include <dispatch/dispatch.h>
#include <memory>
#include <pthread.h>
#include <type_traits>

using CFDeallocator = decltype(&CFRelease);
using AutoCFArray = std::unique_ptr<const __CFArray, CFDeallocator>;
using AutoCFDictionary = std::unique_ptr<const __CFDictionary, CFDeallocator>;
using AutoTISInputSourceRef = std::unique_ptr<__TISInputSource, CFDeallocator>;

/**
 * @brief Runs @p fn on the main dispatch queue and waits for it to finish.
 * Text Input Source (TIS/TSM) functions trap when called off the main queue since macOS 27.
 * Running every call on the main queue also serializes them.
 */
// ponytail: deadlocks if the main thread blocks waiting on the calling thread; the core's main
// thread only waits for the core thread after it has finished.
template <typename Fn> void runOnMainQueue(Fn &&fn)
{
  if (pthread_main_np() != 0) {
    fn();
    return;
  }
  dispatch_sync_f(dispatch_get_main_queue(), &fn, [](void *context) {
    (*static_cast<std::remove_reference_t<Fn> *>(context))();
  });
}
