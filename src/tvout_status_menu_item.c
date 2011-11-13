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
#include <libintl.h>
#include <libosso.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <gconf/gconf-client.h>

#include "tvout-ctl.h"
#include "tvout_gconf_keys.h"
#include "tvout_status_menu_item.h"


/* From kernel-2.6.28/drivers/misc/nokia-av.c */
#define NOKIA_AV_DETECT_FILE "/sys/devices/platform/nokia-av/detect"
enum {
	UNKNOWN,
	HEADPHONES, /* or line input cable or external mic */
	VIDEO_CABLE,
	OPEN_CABLE,
	BASIC_HEADSET,
};


struct _TVoutStatusMenuItemPrivate {
	GtkWidget * button;
	osso_context_t * osso_context;
	TVoutCtl * tvout_ctl;
	GIOChannel * io;
	guint watch;
	gint timer;
	GConfClient * gconf_client;
	guint gconf_enable;
	/* guint gconf_tv_std; */
	guint gconf_aspect;
	guint gconf_scale;
	gpointer data;
};

#define TVOUT_STATUS_MENU_ITEM_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE (obj, TVOUT_TYPE_STATUS_MENU_ITEM, TVoutStatusMenuItemPrivate))

HD_DEFINE_PLUGIN_MODULE(TVoutStatusMenuItem, tvout_status_menu_item, HD_TYPE_STATUS_MENU_ITEM);

static gboolean tvout_status_menu_item_update_plugin(gpointer user_data) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(user_data);
	FILE * detect = fopen(NOKIA_AV_DETECT_FILE, "r");
	int type = UNKNOWN;
	gboolean enable;

	if ( detect ) {

		fscanf(detect, "%d", &type);
		fclose(detect);

	}

	if ( type == VIDEO_CABLE ) {

		enable = gconf_client_get_bool(plugin->priv->gconf_client, TVOUT_GCONF_ENABLE_KEY, NULL);
		hildon_button_set_value(HILDON_BUTTON(plugin->priv->button), enable ? "Output enabled" : "Output disabled"); /* TODO: dgettext */
		gtk_widget_show_all(GTK_WIDGET(plugin));

	} else {

		gtk_widget_hide_all(GTK_WIDGET(plugin));

	}

	return TRUE;

}

static void tvout_status_menu_item_update_tvout(GConfClient * client G_GNUC_UNUSED, guint cnxn_id G_GNUC_UNUSED, GConfEntry * entry, gpointer user_data) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(user_data);
	const char * key = gconf_entry_get_key(entry);
	GConfValue * value = gconf_entry_get_value(entry);

	if ( strcmp(key, TVOUT_GCONF_ENABLE_KEY) == 0 )
		tvout_ctl_set_enable(plugin->priv->tvout_ctl, gconf_value_get_bool(value));
	/* else if ( strcmp(key, TVOUT_GCONF_TV_STD_KEY) == 0 )
		tvout_ctl_set_tv_std(plugin->priv->tvout_ctl, strcmp(gconf_value_get_string(value), "PAL") == 0 ? 0 : 1); */
	else if ( strcmp(key, TVOUT_GCONF_ASPECT_KEY) == 0 )
		tvout_ctl_set_aspect(plugin->priv->tvout_ctl, strcmp(gconf_value_get_string(value), "NORMAL") == 0 ? 0 : 1);
	else if ( strcmp(key, TVOUT_GCONF_SCALE_KEY) == 0 )
		tvout_ctl_set_scale(plugin->priv->tvout_ctl, gconf_value_get_int(value));

	tvout_status_menu_item_update_plugin(user_data);

}

static void tvout_status_menu_item_update_gconf(int attr, int value, void * user_data) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(user_data);

	if ( attr == ATTR_ENABLE )
		gconf_client_set_bool(plugin->priv->gconf_client, TVOUT_GCONF_ENABLE_KEY, value, NULL);
	/* else if ( attr == ATTR_TV_STD )
		gconf_client_set_string(plugin->priv->gconf_client, TVOUT_GCONF_TV_STD_KEY, value == 0 ? "PAL" : "NTSC", NULL); */
	else if ( attr == ATTR_ASPECT )
		gconf_client_set_string(plugin->priv->gconf_client, TVOUT_GCONF_ASPECT_KEY, value == 0 ? "NORMAL" : "WIDE", NULL);
	else if ( attr == ATTR_SCALE )
		gconf_client_set_int(plugin->priv->gconf_client, TVOUT_GCONF_SCALE_KEY, value, NULL);

	tvout_status_menu_item_update_plugin(user_data);

}

static void tvout_status_menu_item_clicked(GObject * button G_GNUC_UNUSED, gpointer user_data) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(user_data);
	osso_cp_plugin_execute(plugin->priv->osso_context, "libcptvout.so", NULL, TRUE);

}

static gboolean tvout_status_menu_item_io_func(GIOChannel * source G_GNUC_UNUSED, GIOCondition condition G_GNUC_UNUSED, gpointer user_data) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(user_data);
	tvout_ctl_fd_ready(plugin->priv->tvout_ctl);
	return TRUE;

}

static void tvout_status_menu_item_create_io_channel(TVoutStatusMenuItem * plugin) {

	GIOStatus s;
	int fd = tvout_ctl_fd(plugin->priv->tvout_ctl);

	if ( fd < 0 )
		return;

	plugin->priv->io = g_io_channel_unix_new(fd);

	if ( ! plugin->priv->io )
		return;

	s = g_io_channel_set_encoding(plugin->priv->io, NULL, NULL);

	if ( s != G_IO_STATUS_NORMAL ) {

		g_io_channel_unref(plugin->priv->io);
		plugin->priv->io = NULL;
		return;

	}

	g_io_channel_set_buffered(plugin->priv->io, FALSE);
	plugin->priv->watch = g_io_add_watch(plugin->priv->io, G_IO_IN | G_IO_PRI, tvout_status_menu_item_io_func, plugin);

	if ( ! plugin->priv->watch ) {

		g_io_channel_unref(plugin->priv->io);
		plugin->priv->io = NULL;
		return;

	}

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

	plugin->priv = TVOUT_STATUS_MENU_ITEM_GET_PRIVATE(plugin);

	plugin->priv->osso_context = osso_initialize("tvout_status_menu_item", "1.0", TRUE, NULL);
	plugin->priv->gconf_client = gconf_client_get_default();
	plugin->priv->button = hildon_button_new(HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH, HILDON_BUTTON_ARRANGEMENT_VERTICAL);
	plugin->priv->tvout_ctl = tvout_ctl_init(plugin, tvout_status_menu_item_update_gconf);

	if ( ! plugin->priv->osso_context || ! plugin->priv->gconf_client || ! plugin->priv->button || ! plugin->priv->tvout_ctl )
		return;

	tvout_status_menu_item_create_io_channel(plugin);

	hildon_button_set_style(HILDON_BUTTON(plugin->priv->button), HILDON_BUTTON_STYLE_PICKER);
	hildon_button_set_image(HILDON_BUTTON(plugin->priv->button), gtk_image_new_from_icon_name("control_tv_out", GTK_ICON_SIZE_DIALOG));
	hildon_button_set_title(HILDON_BUTTON(plugin->priv->button), dgettext("osso-tv-out", "tvou_ap_cpa"));
	gtk_button_set_alignment(GTK_BUTTON(plugin->priv->button), 0, 0);
	g_signal_connect_after(G_OBJECT(plugin->priv->button), "clicked", G_CALLBACK(tvout_status_menu_item_clicked), plugin);

	gtk_container_add(GTK_CONTAINER(plugin), plugin->priv->button);

	tvout_ctl_set_enable(plugin->priv->tvout_ctl, gconf_client_get_bool(plugin->priv->gconf_client, TVOUT_GCONF_ENABLE_KEY, NULL));
	/* tvout_ctl_set_tv_std(plugin->priv->tvout_ctl, strcmp(gconf_client_get_string(plugin->priv->gconf_client, TVOUT_GCONF_TV_STD_KEY, NULL), "PAL") == 0 ? 0 : 1); */
	tvout_ctl_set_aspect(plugin->priv->tvout_ctl, strcmp(gconf_client_get_string(plugin->priv->gconf_client, TVOUT_GCONF_ASPECT_KEY, NULL), "NORMAL") == 0 ? 0 : 1);
	tvout_ctl_set_scale(plugin->priv->tvout_ctl, gconf_client_get_int(plugin->priv->gconf_client, TVOUT_GCONF_SCALE_KEY, NULL));

	gconf_client_add_dir(plugin->priv->gconf_client, TVOUT_GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
	plugin->priv->gconf_enable = gconf_client_notify_add(plugin->priv->gconf_client, TVOUT_GCONF_ENABLE_KEY, (GConfClientNotifyFunc)tvout_status_menu_item_update_tvout, plugin, NULL, NULL);
	/* plugin->priv->gconf_tv_std = gconf_client_notify_add(plugin->priv->gconf_client, TVOUT_GCONF_TV_STD_KEY, (GConfClientNotifyFunc)tvout_status_menu_item_update_tvout, plugin, NULL, NULL); */
	plugin->priv->gconf_aspect = gconf_client_notify_add(plugin->priv->gconf_client, TVOUT_GCONF_ASPECT_KEY, (GConfClientNotifyFunc)tvout_status_menu_item_update_tvout, plugin, NULL, NULL);
	plugin->priv->gconf_scale = gconf_client_notify_add(plugin->priv->gconf_client, TVOUT_GCONF_SCALE_KEY, (GConfClientNotifyFunc)tvout_status_menu_item_update_tvout, plugin, NULL, NULL);

	tvout_status_menu_item_update_plugin(plugin);
	plugin->priv->timer = g_timeout_add(5000, tvout_status_menu_item_update_plugin, plugin);

}

static void tvout_status_menu_item_finalize(GObject * object) {

	TVoutStatusMenuItem * plugin = TVOUT_STATUS_MENU_ITEM(object);

	if ( plugin->priv->timer )
		g_source_remove(plugin->priv->timer);

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

	}

	tvout_status_menu_item_destroy_io_channel(plugin);

	if ( plugin->priv->tvout_ctl )
		tvout_ctl_exit(plugin->priv->tvout_ctl);

	if ( plugin->priv->osso_context )
		osso_deinitialize(plugin->priv->osso_context);

	G_OBJECT_CLASS(tvout_status_menu_item_parent_class)->finalize(object);

}

static void tvout_status_menu_item_class_init(TVoutStatusMenuItemClass * klass) {

	GObjectClass * object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = (GObjectFinalizeFunc)tvout_status_menu_item_finalize;
	g_type_class_add_private(klass, sizeof(TVoutStatusMenuItemPrivate));

}

static void tvout_status_menu_item_class_finalize(TVoutStatusMenuItemClass * klass G_GNUC_UNUSED) {

}
