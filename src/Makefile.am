homeplugindir = @homeplugindir@

homeplugin_LTLIBRARIES = \
	tvout-home-plugin.la

tvout_home_plugin_la_CFLAGS = \
	@HILDON_CFLAGS@ \
	@TVOUT_CTL_CFLAGS@

tvout_home_plugin_la_LIBADD = \
	@HILDON_LIBS@ \
	@TVOUT_CTL_LIBS@

tvout_home_plugin_la_LDFLAGS = \
	-avoid-version \
	-module \
	-shared

tvout_home_plugin_la_SOURCES = \
	tvout-home-plugin.c \
	tvout-home-plugin.h
