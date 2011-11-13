hildondir = $(HILDON_LIB_DIR)

hildon_LTLIBRARIES = libtvout_status_menu_item.la tvout-home-plugin.la

libtvout_status_menu_item_la_CFLAGS = $(HILDON_CFLAGS)

libtvout_status_menu_item_la_LIBADD = $(HILDON_LIBS)

libtvout_status_menu_item_la_LDFLAGS = \
	-avoid-version \
	-module \
	-shared

libtvout_status_menu_item_la_SOURCES = \
	tvout-ctl.c \
	tvout-ctl.h \
	tvout_gconf_keys.h \
	tvout_status_menu_item.c \
	tvout_status_menu_item.h

tvout_home_plugin_la_CFLAGS = $(HILDON_CFLAGS)

tvout_home_plugin_la_LIBADD = $(HILDON_LIBS)

tvout_home_plugin_la_LDFLAGS = \
	-avoid-version \
	-module \
	-shared

tvout_home_plugin_la_SOURCES = \
	tvout-home-plugin.c \
	tvout-home-plugin.h

controlpaneldir = $(CONTROLPANEL_LIB_DIR)

controlpanel_LTLIBRARIES = libcptvout.la

libcptvout_la_CFLAGS = $(HILDON_CFLAGS) $(CONTROLPANEL_CFLAGS)

libcptvout_la_LIBADD = $(HILDON_LIBS) $(CONTROLPANEL_LIBS)

libcptvout_la_LDFLAGS = \
	-avoid-version \
	-module \
	-shared

libcptvout_la_SOURCES = libcptvout.c
