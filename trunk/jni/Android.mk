LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

LOCAL_MODULE := ZgeOuya
LOCAL_SRC_FILES := ZgeOuya.cpp
LOCAL_C_INCLUDES := $(NDK_ROOT)/sources/cxx-stl/stlport/stlport

#comment if logging is edisabled
#LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)
