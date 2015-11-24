LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libnfnetlink/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libmnl/include/
LOCAL_SRC_FILES := src/libnetfilter_queue.c 
LOCAL_MODULE    :=libnetfilter_queue
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
