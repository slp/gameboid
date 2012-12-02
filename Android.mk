LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := user

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := GameBoid

LOCAL_JNI_SHARED_LIBRARIES := \
	libgba

include $(BUILD_PACKAGE)

# ============================================================
MY_DIR := $(LOCAL_PATH)

# Also build all of the sub-targets under this one: the shared library.
include $(call all-makefiles-under,$(LOCAL_PATH))
include $(APP_PROJECT_PATH)/../../emudroid-common/jni/libnativehelper/Android.mk
