include $(top_srcdir)/PATHS

rusage_SOURCES = rusage.c 
rusage_INCLUDES = -I$(INCLUDE_DIR)

mallinfo_SOURCES = mallinfo.c 
mallinfo_INCLUDES = -I$(INCLUDE_DIR)

meminfo_SOURCES = meminfo.c 
meminfo_INCLUDES = -I$(INCLUDE_DIR)

cpuinfo_SOURCES = cpuinfo.c
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
	mkdir -p $(libdir)/plugins
	cp $(srcdir)/cpuinfo.c $(DESTDIR)$(datadir)/plugins
	cp $(srcdir)/mallinfo.c $(DESTDIR)$(datadir)/plugins
	cp $(srcdir)/meminfo.c $(DESTDIR)$(datadir)/plugins
	cp $(srcdir)/rusage.c $(DESTDIR)$(datadir)/plugins
	cp $(libdir)/libmallinfo.so $(libdir)/plugins
	cp $(libdir)/libmeminfo.so $(libdir)/plugins
	cp $(libdir)/librusage.so $(libdir)/plugins
	cp $(libdir)/libcpuinfo.so $(libdir)/plugins