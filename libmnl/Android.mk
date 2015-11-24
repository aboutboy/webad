LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_SRC_FILES := src/socket.c  \
                   src/callback.c \
		   src/nlmsg.c \
		   src/attr.c 
LOCAL_MODULE    :=libmnl
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
