LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := user

LOCAL_ARM_MODE := arm

# This is the target being built.
LOCAL_MODULE := libemusound

# All of the source files that we will compile.
LOCAL_SRC_FILES := audioplayer.cpp

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := libmedia

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../jni/android

# Special compiler flags.
LOCAL_CFLAGS += -fvisibility=hidden

# Don't prelink this library.  For more efficient code, you may want
# to add this library to the prelink map and set this to true. However,
# it's difficult to do this for applications that are not supplied as
# part of a system image.

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

