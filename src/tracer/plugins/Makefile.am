include $(top_srcdir)/PATHS

rusage_SOURCES = rusage.c extrae_plugins.h
rusage_INCLUDES = -I$(INCLUDE_DIR)

mallinfo_SOURCES = mallinfo.c extrae_plugins.h
mallinfo_INCLUDES = -I$(INCLUDE_DIR)

meminfo_SOURCES = meminfo.c extrae_plugins.h
meminfo_INCLUDES = -I$(INCLUDE_DIR)

cpuinfo_SOURCES = cpuinfo.c extrae_plugins.h
cpuinfo_INCLUDES = -I$(INCLUDE_DIR)

lib_LTLIBRARIES = librusage.la
lib_LTLIBRARIES += libmallinfo.la
lib_LTLIBRARIES += libmeminfo.la
lib_LTLIBRARIES += libcpuinfo.la

libmallinfo_la_SOURCES = $(mallinfo_SOURCES)
libmallinfo_la_CFLAGS  = $(mallinfo_INCLUDES)
# libmallinfo_la_LDFLAGS = -module -avoid-version

librusage_la_SOURCES = $(rusage_SOURCES)
librusage_la_CFLAGS  = $(rusage_INCLUDES)

libmeminfo_la_SOURCES = $(meminfo_SOURCES)
libmeminfo_la_CFLAGS  = $(meminfo_INCLUDES)

libcpuinfo_la_SOURCES = $(cpuinfo_SOURCES)
libcpuinfo_la_CFLAGS  = $(cpuinfo_INCLUDES)

install-exec-hook:
	mkdir -p $(DESTDIR)$(datadir)/plugins
	cp $(includedir)/extrae_plugins.h $(DESTDIR)$(datadir)/plugins
	cp $(srcdir)/cpuinfo.c $(DESTDIR)$(datadir)/plugins
	cp $(srcdir)/mallinfo.c $(DESTDIR)$(datadir)/plugins
	cp $(srcdir)/meminfo.c $(DESTDIR)$(datadir)/plugins
	cp $(srcdir)/rusage.c $(DESTDIR)$(datadir)/plugins
	cp $(libdir)/libmallinfo.so $(DESTDIR)$(datadir)/plugins
	cp $(libdir)/libmeminfo.so $(DESTDIR)$(datadir)/plugins
	cp $(libdir)/librusage.so $(DESTDIR)$(datadir)/plugins
	cp $(libdir)/libcpuinfo.so $(DESTDIR)$(datadir)/plugins