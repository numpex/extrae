include $(top_srcdir)/PATHS

event_timer_API_SRCS = \
 event_timer.c event_timer.h \
 timer_manager.c timer_manager.h \
 plugin_manager.c plugin_manager.h

iface_API_INCLUDES = \
  -I$(COMMON_INC)  \
  -I$(CLOCKS_INC)  \
  -I$(TRACER_INC)  \
	-I$(HWC_INC) \
  -I$(WRAPPERS_INC)/API \
   -I$(top_srcdir)/include 

noinst_LTLIBRARIES = libevent_timer.la

libevent_timer_la_SOURCES = $(event_timer_API_SRCS)
libevent_timer_la_CFLAGS  = $(iface_API_INCLUDES) 
