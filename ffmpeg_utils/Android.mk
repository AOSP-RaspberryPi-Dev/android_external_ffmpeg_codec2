#
# Copyright (C) 2017 The Android-x86 Open Source Project
# Copyright (C) 2023 KonstaKANG
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libffmpeg_utils
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    ffmpeg_hwaccel.c \
    ffmpeg_utils.cpp

LOCAL_SHARED_LIBRARIES += \
    libavcodec \
    libavformat \
    libavutil \
    libcutils \
    liblog \
    libstagefright_foundation \
    libswresample \
    libswscale \
    libutils

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
