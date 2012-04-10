/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// Parts of this file derived from Chromium's base/cpu.cc.

#include "cpu_features_wrapper.h"

#include "typedefs.h"

// No CPU feature is available => straight C path.
int GetCPUInfoNoASM(CPUFeature feature) {
  (void)feature;
  return 0;
}

// Default to straight C for other platforms.
static int GetCPUInfo(CPUFeature feature) {
  (void)feature;
  return 0;
}

WebRtc_CPUInfo WebRtc_GetCPUInfo = GetCPUInfo;
WebRtc_CPUInfo WebRtc_GetCPUInfoNoASM = GetCPUInfoNoASM;
