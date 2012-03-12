/*
    tvout_status_menu_item.c - TV out Status Menu Item Hildon plugin
    Copyright (C) 2011  Pali Rohár <pali.rohar@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include <stdio.h>
#include <string.h>

#include <dlfcn.h>
#include <libintl.h>

#include <hal/libhal.h>

#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <gconf/gconf-client.h>

#include <tvout-ctl.h>
#include "tvout_gconf_keys.h"
#include "tvout_status_menu_item.h"

#define HAL_JACK_UDI "/org/freedesktop/Hal/devices/platform_soc_audio_logicaldev_input"
#define HAL_JACK_KEY_TYPE "input.jack.type"

#define LIBCPTVOUT "/usr/lib/hildon-control-panel/libcptvout.so"

struct _TVoutStatusMenuItemPrivate {
	DBusConnection * dbus_connection;
	LibHalContext * ctx;
	TVoutCtl * tvout_ctl;
	GConfClient * gconf_client;
	GIOChannel * io;
	GtkWidget * button;
	guint watch;
	guint gconf_enable;
	/* guint gconf_tv_std; */
	guint gconf_aspect;
	guint gconf_scale;
	gpointer data;
};

#define TVOUT_STATUS_MENU_ITEM_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE (obj, TVOUT_TYPE_STATUS_MENU_ITEM, TVoutStatusMenuItemPrivate))

HD_DEFINE_PLUGIN_MODULE(TVoutStatusMenuItem, tvout_status_menu_item, HD_TYPE_STATUS_MENU_ITEM);

static void tvout_status_menu_item_update_button(LibHalContext * ctx, const char * udi, const char * key, dbus_bool_t is_removed G_GNUC_UNUSED, dbus_bool_t is_added G_GNUC_UNUSED) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(libhal_ctx_get_user_data(ctx));
	gboolean enable;
	char ** types;

	if ( strcmp(udi, HAL_JACK_UDI) != 0 || strcmp(key, HAL_JACK_KEY_TYPE) != 0 )
		return;

	types = libhal_device_get_property_strlist(plugin->priv->ctx, HAL_JACK_UDI, HAL_JACK_KEY_TYPE, NULL);

	if ( ! types )
		return;

	while ( *types ) {

		if ( strncmp(*types, "video", strlen("video")) == 0 ) {

			enable = gconf_client_get_bool(plugin->priv->gconf_client, TVOUT_GCONF_ENABLE_KEY, NULL);
			hildon_button_set_value(HILDON_BUTTON(plugin->priv->button), enable ? "Output enabled" : "Output disabled"); /* TODO: dgettext */
			gtk_widget_show_all(GTK_WIDGET(plugin));
			return;

		}

		++types;

	}

	gtk_widget_hide(GTK_WIDGET(plugin));

}

static void tvout_status_menu_item_update_tvout(GConfClient * client G_GNUC_UNUSED, guint cnxn_id G_GNUC_UNUSED, GConfEntry * entry, gpointer user_data) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(user_data);
	const char * key = gconf_entry_get_key(entry);
	GConfValue * value = gconf_entry_get_value(entry);

	if ( strcmp(key, TVOUT_GCONF_ENABLE_KEY) == 0 )
		tvout_ctl_set(plugin->priv->tvout_ctl, TVOUT_CTL_ENABLE, gconf_value_get_bool(value));
	/* else if ( strcmp(key, TVOUT_GCONF_TV_STD_KEY) == 0 )
		tvout_ctl_set_tv_std(plugin->priv->tvout_ctl, strcmp(gconf_value_get_string(value), "PAL") == 0 ? 0 : 1); */
	else if ( strcmp(key, TVOUT_GCONF_ASPECT_KEY) == 0 )
		tvout_ctl_set(plugin->priv->tvout_ctl, TVOUT_CTL_ASPECT, strcmp(gconf_value_get_string(value), "NORMAL") == 0 ? 0 : 1);
	else if ( strcmp(key, TVOUT_GCONF_SCALE_KEY) == 0 )
		tvout_ctl_set(plugin->priv->tvout_ctl, TVOUT_CTL_SCALE, gconf_value_get_int(value));

	tvout_status_menu_item_update_button(plugin->priv->ctx, HAL_JACK_UDI, HAL_JACK_KEY_TYPE, FALSE, FALSE);

}

static void tvout_status_menu_item_update_gconf(void *user_data,
						enum TVoutCtlAttr attr,
						int value) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(user_data);

	if ( attr == TVOUT_CTL_ENABLE )
		gconf_client_set_bool(plugin->priv->gconf_client, TVOUT_GCONF_ENABLE_KEY, value, NULL);
	/* else if ( attr == TVOUT_CTL_TV_STD )
		gconf_client_set_string(plugin->priv->gconf_client, TVOUT_GCONF_TV_STD_KEY, value == 0 ? "PAL" : "NTSC", NULL); */
	else if ( attr == TVOUT_CTL_ASPECT )
		gconf_client_set_string(plugin->priv->gconf_client, TVOUT_GCONF_ASPECT_KEY, value == 0 ? "NORMAL" : "WIDE", NULL);
	else if ( attr == TVOUT_CTL_SCALE )
		gconf_client_set_int(plugin->priv->gconf_client, TVOUT_GCONF_SCALE_KEY, value, NULL);

	tvout_status_menu_item_update_button(plugin->priv->ctx, HAL_JACK_UDI, HAL_JACK_KEY_TYPE, FALSE, FALSE);

}

static void tvout_status_menu_item_clicked(GObject * button G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {

	void * handle;
	int (*execute)(void *, gpointer, gboolean);

	handle = dlopen(LIBCPTVOUT, RTLD_LAZY);

	if ( ! handle )
		return;

	execute = dlsym(handle, "execute");

	if ( ! execute )
		return;

	execute(NULL, NULL, TRUE);
	dlclose(handle);

}

static gboolean tvout_status_menu_item_io_func(GIOChannel * source G_GNUC_UNUSED, GIOCondition condition G_GNUC_UNUSED, gpointer user_data) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(user_data);
	tvout_ctl_fd_ready(plugin->priv->tvout_ctl);
	return TRUE;

}

static gboolean tvout_status_menu_item_create_io_channel(TVoutStatusMenuItem * plugin) {

	GIOStatus s;
	int fd = tvout_ctl_fd(plugin->priv->tvout_ctl);

	if ( fd < 0 )
		return FALSE;

	plugin->priv->io = g_io_channel_unix_new(fd);

	if ( ! plugin->priv->io )
		return FALSE;

	s = g_io_channel_set_encoding(plugin->priv->io, NULL, NULL);

	if ( s != G_IO_STATUS_NORMAL ) {

		g_io_channel_unref(plugin->priv->io);
		plugin->priv->io = NULL;
		return FALSE;

	}

	g_io_channel_set_buffered(plugin->priv->io, FALSE);
	plugin->priv->watch = g_io_add_watch(plugin->priv->io, G_IO_IN | G_IO_PRI, tvout_status_menu_item_io_func, plugin);

	if ( ! plugin->priv->watch ) {

		g_io_channel_unref(plugin->priv->io);
		plugin->priv->io = NULL;
		return FALSE;

	}

	return TRUE;

}

static void tvout_status_menu_item_destroy_io_channel(TVoutStatusMenuItem * plugin) {

	if ( plugin->priv->watch ) {

		g_source_remove(plugin->priv->watch);
		plugin->priv->watch = 0;

	}

	if ( plugin->priv->io ) {

		g_io_channel_unref(plugin->priv->io);
		plugin->priv->io = NULL;

	}

}

static void tvout_status_menu_item_init(TVoutStatusMenuItem * plugin) {

	DBusError error;
	char *str;

	plugin->priv = TVOUT_STATUS_MENU_ITEM_GET_PRIVATE(plugin);

	dbus_error_init(&error);

	plugin->priv->dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

	if ( ! plugin->priv->dbus_connection || dbus_error_is_set(&error) ) {

		dbus_error_free(&error);
		return;

	}

	dbus_error_free(&error);

	plugin->priv->ctx = libhal_ctx_new();

	if ( ! plugin->priv->ctx )
		return;

	libhal_ctx_set_dbus_connection(plugin->priv->ctx, plugin->priv->dbus_connection);
	libhal_ctx_set_user_data(plugin->priv->ctx, plugin);
	libhal_ctx_set_device_property_modified(plugin->priv->ctx, tvout_status_menu_item_update_button);

	if ( ! libhal_ctx_init(plugin->priv->ctx, &error) )
		return;

	if ( ! libhal_device_add_property_watch(plugin->priv->ctx, HAL_JACK_UDI, &error) )
		return;

	plugin->priv->gconf_client = gconf_client_get_default();

	if ( ! plugin->priv->gconf_client )
		return;

	plugin->priv->tvout_ctl = tvout_ctl_init(tvout_status_menu_item_update_gconf, plugin);

	if ( ! plugin->priv->tvout_ctl )
		return;

	if ( ! tvout_status_menu_item_create_io_channel(plugin) )
		return;

	plugin->priv->button = hildon_button_new(HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH, HILDON_BUTTON_ARRANGEMENT_VERTICAL);

	if ( ! plugin->priv->button )
		return;

	hildon_button_set_style(HILDON_BUTTON(plugin->priv->button), HILDON_BUTTON_STYLE_PICKER);
	hildon_button_set_image(HILDON_BUTTON(plugin->priv->button), gtk_image_new_from_icon_name("control_tv_out", GTK_ICON_SIZE_DIALOG));
	hildon_button_set_title(HILDON_BUTTON(plugin->priv->button), dgettext("osso-tv-out", "tvou_ap_cpa"));
	gtk_button_set_alignment(GTK_BUTTON(plugin->priv->button), 0, 0);
	g_signal_connect_after(G_OBJECT(plugin->priv->button), "clicked", G_CALLBACK(tvout_status_menu_item_clicked), plugin);

	gtk_container_add(GTK_CONTAINER(plugin), plugin->priv->button);

	gconf_client_add_dir(plugin->priv->gconf_client, TVOUT_GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
	plugin->priv->gconf_enable = gconf_client_notify_add(plugin->priv->gconf_client, TVOUT_GCONF_ENABLE_KEY, tvout_status_menu_item_update_tvout, plugin, NULL, NULL);
	/* plugin->priv->gconf_tv_std = gconf_client_notify_add(plugin->priv->gconf_client, TVOUT_GCONF_TV_STD_KEY, tvout_status_menu_item_update_tvout, plugin, NULL, NULL); */
	plugin->priv->gconf_aspect = gconf_client_notify_add(plugin->priv->gconf_client, TVOUT_GCONF_ASPECT_KEY, tvout_status_menu_item_update_tvout, plugin, NULL, NULL);
	plugin->priv->gconf_scale = gconf_client_notify_add(plugin->priv->gconf_client, TVOUT_GCONF_SCALE_KEY, tvout_status_menu_item_update_tvout, plugin, NULL, NULL);

	tvout_ctl_set(plugin->priv->tvout_ctl, TVOUT_CTL_ENABLE, gconf_client_get_bool(plugin->priv->gconf_client, TVOUT_GCONF_ENABLE_KEY, NULL));
#if 0
	str = gconf_client_get_string(plugin->priv->gconf_client, TVOUT_GCONF_TV_STD_KEY, NULL);
	tvout_ctl_set_tv_std(plugin->priv->tvout_ctl, strcmp(str, "PAL") == 0 ? 0 : 1);
	g_free(str);
#endif
	str = gconf_client_get_string(plugin->priv->gconf_client, TVOUT_GCONF_ASPECT_KEY, NULL);
	tvout_ctl_set(plugin->priv->tvout_ctl, TVOUT_CTL_ASPECT, strcmp(str, "NORMAL") == 0 ? 0 : 1);
	g_free(str);
	tvout_ctl_set(plugin->priv->tvout_ctl, TVOUT_CTL_SCALE, gconf_client_get_int(plugin->priv->gconf_client, TVOUT_GCONF_SCALE_KEY, NULL));

	tvout_status_menu_item_update_button(plugin->priv->ctx, HAL_JACK_UDI, HAL_JACK_KEY_TYPE, FALSE, FALSE);

}

static void tvout_status_menu_item_finalize(GObject * object) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(object);

	if ( plugin->priv->gconf_client ) {

		if ( plugin->priv->gconf_enable )
			gconf_client_notify_remove(plugin->priv->gconf_client, plugin->priv->gconf_enable);

/*		if ( plugin->priv->gconf_tv_std )
			gconf_client_notify_remove(plugin->priv->gconf_client, plugin->priv->gconf_tv_std); */

		if ( plugin->priv->gconf_aspect )
			gconf_client_notify_remove(plugin->priv->gconf_client, plugin->priv->gconf_aspect);

		if ( plugin->priv->gconf_scale )
			gconf_client_notify_remove(plugin->priv->gconf_client, plugin->priv->gconf_scale);

		gconf_client_remove_dir(plugin->priv->gconf_client, TVOUT_GCONF_PATH, NULL);
		gconf_client_clear_cache(plugin->priv->gconf_client);
		g_object_unref(plugin->priv->gconf_client);

		plugin->priv->gconf_client = NULL;

	}

	tvout_status_menu_item_destroy_io_channel(plugin);

	if ( plugin->priv->tvout_ctl ) {

		tvout_ctl_exit(plugin->priv->tvout_ctl);
		plugin->priv->tvout_ctl = NULL;

	}

	if ( plugin->priv->ctx ) {

		libhal_ctx_shutdown(plugin->priv->ctx, NULL);
		libhal_ctx_free(plugin->priv->ctx);
		plugin->priv->ctx = NULL;

	}

	if ( plugin->priv->dbus_connection ) {

		dbus_connection_unref(plugin->priv->dbus_connection);
		plugin->priv->dbus_connection = NULL;

	}

	G_OBJECT_CLASS(tvout_status_menu_item_parent_class)->finalize(object);

}

static void tvout_status_menu_item_class_init(TVoutStatusMenuItemClass * klass) {

	GObjectClass * object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = (GObjectFinalizeFunc)tvout_status_menu_item_finalize;
	g_type_class_add_private(klass, sizeof(TVoutStatusMenuItemPrivate));

}

static void tvout_status_menu_item_class_finalize(TVoutStatusMenuItemClass * klass G_GNUC_UNUSED) {

}
