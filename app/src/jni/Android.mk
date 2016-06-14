LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/includes/ \
	$(LOCAL_PATH)/includes/expat/ \
	$(LOCAL_PATH)/includes/scew/ \
	$(LOCAL_PATH)/includes/ckpi/ \
	$(LOCAL_PATH)/SmartSlog/

LOCAL_MODULE    := sslog
LOCAL_SRC_FILES := src/kp.c \
	src/ontology.c \
	SmartSlog/repository.c \
	SmartSlog/classes.c \
	SmartSlog/kpi_interface.c \
	SmartSlog/patterns.c \
	SmartSlog/properties.c \
	SmartSlog/property_changes.c \
	SmartSlog/ss_classes.c \
	SmartSlog/ss_func.c \
	SmartSlog/ss_multisib.c\
	SmartSlog/ss_populate.c \
	SmartSlog/ss_properties.c \
	SmartSlog/structures.c \
	SmartSlog/subscription.c \
	SmartSlog/subscription_changes.c \
	SmartSlog/utils/check_func.c \
	SmartSlog/utils/kp_error.c \
	SmartSlog/utils/list.c \
	SmartSlog/utils/util_func.c \
	SmartSlog/expat/xmlparse.c \
	SmartSlog/expat/xmlrole.c \
	SmartSlog/expat/xmltok.c \
	SmartSlog/expat/xmltok_impl.c \
	SmartSlog/expat/xmltok_ns.c \
	SmartSlog/scew/attribute.c \
	SmartSlog/scew/element_attribute.c \
	SmartSlog/scew/element.c \
	SmartSlog/scew/element_compare.c \
	SmartSlog/scew/element_copy.c \
	SmartSlog/scew/element_search.c \
	SmartSlog/scew/error.c \
	SmartSlog/scew/list.c \
	SmartSlog/scew/parser.c \
	SmartSlog/scew/printer.c \
	SmartSlog/scew/reader_buffer.c \
	SmartSlog/scew/reader.c \
	SmartSlog/scew/reader_file.c \
	SmartSlog/scew/str.c \
	SmartSlog/scew/tree.c \
	SmartSlog/scew/writer_buffer.c \
	SmartSlog/scew/writer.c \
	SmartSlog/scew/writer_file.c \
	SmartSlog/scew/xattribute.c \
	SmartSlog/scew/xerror.c \
	SmartSlog/scew/xparser.c \
	SmartSlog/ckpi/ckpi.c \
	SmartSlog/ckpi/compose_ssap_msg.c \
	SmartSlog/ckpi/parse_ssap_msg.c \
	SmartSlog/ckpi/process_ssap_cnf.c \
	SmartSlog/ckpi/sib_access_tcp.c \
	SmartSlog/ckpi/sskp_errno.c \
    
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -DMTENABLE -DHAVE_EXPAT_CONFIG_H -DHAVE_MEMMOVE

include $(BUILD_SHARED_LIBRARY)
