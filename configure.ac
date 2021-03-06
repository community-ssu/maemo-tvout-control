#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([maemo-tvout-control], [6], [])
AM_INIT_AUTOMAKE([])
AC_CONFIG_SRCDIR([src/tvout-home-plugin.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_INSTALL
AM_PROG_LIBTOOL

saved_LIBS=$LIBS
LIBS=
AC_SEARCH_LIBS([dlopen], [dl], [], AC_MSG_ERROR([dlopen not found]))
DLOPEN_LIBS=$LIBS
LIBS=$saved_LIBS
AC_SUBST(DLOPEN_LIBS)

PKG_CHECK_MODULES(HILDON_CONTROLPANEL, hildon-control-panel)
AC_SUBST(HILDON_CONTROLPANEL_CFLAGS)
AC_SUBST(HILDON_CONTROLPANEL_LIBS)

PKG_CHECK_MODULES(OSSO, libosso)
AC_SUBST(OSSO_CFLAGS)
AC_SUBST(OSSO_LIBS)

PKG_CHECK_MODULES(GCONF, gconf-2.0)
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

AM_GCONF_SOURCE_2

PKG_CHECK_MODULES(HILDON_DESKTOP, libhildondesktop-1)
AC_SUBST(HILDON_DESKTOP_CFLAGS)
AC_SUBST(HILDON_DESKTOP_LIBS)

PKG_CHECK_MODULES(HAL, hal)
AC_SUBST(HAL_CFLAGS)
AC_SUBST(HAL_LIBS)

PKG_CHECK_MODULES(TVOUT_CTL, tvout-ctl)
AC_SUBST(TVOUT_CTL_CFLAGS)
AC_SUBST(TVOUT_CTL_LIBS)

HILDON_DESKTOP_LIB_DIR=`$PKG_CONFIG --variable=hildondesktoplibdir libhildondesktop-1`
AC_SUBST(HILDON_DESKTOP_LIB_DIR)

HILDON_DESKTOP_ENTRY_DIR=`$PKG_CONFIG --variable=hildonhomedesktopentrydir libhildondesktop-1`
AC_SUBST(HILDON_DESKTOP_ENTRY_DIR)

HILDON_MENU_ENTRY_DIR=`$PKG_CONFIG --variable=hildonstatusmenudesktopentrydir libhildondesktop-1`
AC_SUBST(HILDON_MENU_ENTRY_DIR)

HILDON_CONTROLPANEL_LIB_DIR=`$PKG_CONFIG --variable=pluginlibdir hildon-control-panel`
AC_SUBST(HILDON_CONTROLPANEL_LIB_DIR)

HILDON_CONTROLPANEL_ENTRY_DIR=`$PKG_CONFIG --variable=plugindesktopentrydir hildon-control-panel`
AC_SUBST(HILDON_CONTROLPANEL_ENTRY_DIR)

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile])
AC_OUTPUT
