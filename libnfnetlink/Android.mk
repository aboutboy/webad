LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_SRC_FILES := src/iftable.c  \
                   src/libnfnetlink.c \
		   src/rtnl.c 
LOCAL_MODULE    :=libnfnetlink
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
