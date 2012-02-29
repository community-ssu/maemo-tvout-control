noinst_HEADERS = \
	tvout_gconf_keys.h

hildon_desktopdir = \
	$(HILDON_DESKTOP_LIB_DIR)

hildon_desktop_LTLIBRARIES = \
	libtvout_status_menu_item.la \
	tvout-home-plugin.la

libtvout_status_menu_item_la_CFLAGS = \
	$(HILDON_DESKTOP_CFLAGS) \
	$(HAL_CFLAGS) \
	$(TVOUT_CTL_CFLAGS)

libtvout_status_menu_item_la_LIBADD = \
	$(HILDON_DESKTOP_LIBS) \
	$(HAL_LIBS) \
	$(TVOUT_CTL_LIBS) \
	$(DLOPEN_LIBS)

libtvout_status_menu_item_la_LDFLAGS = \
	-avoid-version \
	-module \
	-shared

libtvout_status_menu_item_la_SOURCES = \
	tvout_status_menu_item.c \
	tvout_status_menu_item.h

tvout_home_plugin_la_CFLAGS = \
	$(HILDON_DESKTOP_CFLAGS)

tvout_home_plugin_la_LIBADD = \
	$(HILDON_DESKTOP_LIBS)

tvout_home_plugin_la_LDFLAGS = \
	-avoid-version \
	-module \
	-shared

tvout_home_plugin_la_SOURCES = \
	tvout-home-plugin.c \
	tvout-home-plugin.h

hildon_controlpaneldir = \
	$(HILDON_CONTROLPANEL_LIB_DIR)

hildon_controlpanel_LTLIBRARIES = \
	libcptvout.la

libcptvout_la_CFLAGS = \
	$(HILDON_CONTROLPANEL_CFLAGS) \
	$(GCONF_CFLAGS)

libcptvout_la_LIBADD = \
	$(HILDON_CONTROLPANEL_LIBS) \
	$(GCONF_LIBS)

libcptvout_la_LDFLAGS = \
	-avoid-version \
	-module \
	-shared

libcptvout_la_SOURCES = \
	libcptvout.c
