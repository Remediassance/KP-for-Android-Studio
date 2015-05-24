LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := app
LOCAL_SRC_FILES := \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\Android.mk \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\android_4.2.2.cpp \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\Application.mk \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SRClient.cpp \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\srclient_4.2.2.cpp \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SRClient_4_2_2.cpp \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\sslog.cpp \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\classes.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\kpi_interface.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\Makefile.am \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\Makefile.in \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\patterns.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\properties.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\property_changes.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\repository.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ss_classes.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ss_func.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ss_multisib.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ss_populate.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ss_properties.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\structures.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\subscription.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\subscription_changes.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ckpi\ckpi.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ckpi\compose_ssap_msg.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ckpi\parse_ssap_msg.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ckpi\process_ssap_cnf.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ckpi\sib_access_tcp.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\ckpi\sskp_errno.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\expat\xmlparse.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\expat\xmlrole.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\expat\xmltok.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\expat\xmltok_impl.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\expat\xmltok_ns.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\attribute.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\element.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\element_attribute.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\element_compare.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\element_copy.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\element_search.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\error.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\list.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\parser.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\printer.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\reader.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\reader_buffer.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\reader_file.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\str.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\tree.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\writer.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\writer_buffer.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\writer_file.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\xattribute.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\xerror.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\scew\xparser.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\utils\check_func.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\utils\kp_error.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\utils\kp_errors_define_decl.txt \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\utils\list.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\SmartSlog\utils\util_func.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\src\kp.c \
	C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni\src\ontology.c \

LOCAL_C_INCLUDES += C:\Users\Андрей\AndroidstudioProjects\KP\app\src\main\jni
LOCAL_C_INCLUDES += C:\Users\Андрей\AndroidstudioProjects\KP\app\src\debug\jni

include $(BUILD_SHARED_LIBRARY)
