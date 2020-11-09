include $(top_srcdir)/PATHS

rusage_plugin_SOURCES = \
   rusage.c rusage.h extrae_plugins.h

rusage_plugin_INCLUDES = \
   -I$(INCLUDE_DIR)


memusage_plugin_SOURCES = \
   memusage.c memusage.h extrae_plugins.h

memusage_plugin_INCLUDES = \
   -I$(INCLUDE_DIR)

meminfo_plugin_SOURCES = \
   xtr_plugin_meminfo.c xtr_plugin_meminfo.h extrae_plugins.h

meminfo_plugin_INCLUDES = \
   -I$(INCLUDE_DIR)

lib_LTLIBRARIES = librusage.la
lib_LTLIBRARIES += libmemusage.la
lib_LTLIBRARIES += libmeminfo.la


libmemusage_la_SOURCES = $(memusage_plugin_SOURCES)
libmemusage_la_CFLAGS  = $(memusage_plugin_INCLUDES)
# libmemusage_la_LIBADD = $(TRACER_LIB)/interfaces/API/libiface_api.la
# libmemusage_la_LDFLAGS = -release $(PACKAGE_VERSION)


librusage_la_SOURCES = $(rusage_plugin_SOURCES)
librusage_la_CFLAGS  = $(rusage_plugin_INCLUDES)
# librusage_la_LIBADD = $(TRACER_LIB)/interfaces/API/libiface_api.la
# librusage_la_LDFLAGS = -release $(PACKAGE_VERSION)

libmeminfo_la_SOURCES = $(meminfo_plugin_SOURCES)
libmeminfo_la_CFLAGS  = $(meminfo_plugin_INCLUDES)
# libmeminfo_la_LIBADD = $(TRACER_LIB)/interfaces/API/libiface_api.la
# libmeminfo_la_LDFLAGS = -release $(PACKAGE_VERSION)