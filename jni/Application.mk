APP_BUILD_SCRIPT = $(APP_PROJECT_PATH)/Android.mk

APP_ABI := armeabi

ifeq ($(APP_OPTIM),release)
APP_ABI += armeabi-v7a
endif

EMU_LIBRARY_PATH = $(APP_PROJECT_PATH)/../emudroid-common/