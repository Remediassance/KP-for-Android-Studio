#SUBDIRS =  utils 

AM_CFLAGS = $(CFLAGS) -Wall $(kpilow_CFLAGS)
AM_LDFLAGS = $(kpilow_LDLAGS)

INCLUDES = -I$(KPI_LOW_SRC_FOLDER)

lib_LTLIBRARIES = libsmartslog.la
#lib_LTIBRARIES = libsmartslog.la

libsmartslog_la_SOURCES = \
    classes.c \
    classes.h \
    generic.h \
    kpi_interface.c \
    kpi_interface.h \
    properties.c \
    properties.h \
    ss_classes.c \
    ss_classes.h \
    ss_func.c \
    ss_func.h \
    ss_populate.c \
    ss_populate.h \
    ss_properties.c \
    ss_properties.h \
    subscription.c \
    subscription.h \
	subscription_internal.h \
    structures.c \
    structures.h \
    patterns.c \
    patterns.h \
    ss_multisib.c \
    ss_multisib.h \
    subscription_changes.c \
	subscription_changes.h \
	subscription_changes_internal.h \
    property_changes.c \
    property_changes.h \
    repository.c \
    repository.h \
 \
    utils/check_func.c \
    utils/check_func.h \
    utils/kp_bool.h \
    utils/kp_debug.h \
    utils/kp_error.c \
    utils/kp_error.h \
    utils/kp_errors_define_decl.txt \
    utils/list.c \
    utils/list.h \
    utils/util_func.c \
    utils/util_func.h

libsmartslogincludedir = $(includedir)/smartslog/
libsmartsloginclude_HEADERS = \
    classes.h \
    generic.h \
    kpi_interface.h \
    properties.h \
    ss_classes.h \
    ss_func.h \
    ss_multisib.h \
    ss_populate.h \
    ss_properties.h \
    subscription.h \
    structures.h \
    patterns.h \
    property_changes.h \
	repository.h \
    subscription_changes.h



 
 
libsmartslogutilsincludedir = $(includedir)/smartslog/utils/
libsmartslogutilsinclude_HEADERS = \
    utils/check_func.h \
    utils/kp_bool.h \
    utils/kp_debug.h \
    utils/kp_error.h \
    utils/list.h \
    utils/util_func.h \
    utils/kp_errors_define_decl.txt 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ../smartslog.pc 

#libsmartslog_a_LDFLAGS = -version-info 0:0:0

ACLOCAL_AMFLAGS = -I m4

#EXTRA_DIST = mkinstalldirs  config.rpath m4/ChangeLog\
#	     intltool-extract.in \
#             intltool-merge.in \
#	     intltool-update.in
#test:
#	cd src && $(MAKE) $(AM_MAKEFLAGS) rtest

#.PHONY: test

