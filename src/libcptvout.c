/*
    libcptvout.c - TV out Hildon Control Panel plugin
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

#include <string.h>
#include <unistd.h>
#include <libintl.h>

#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <hildon-cp-plugin/hildon-cp-plugin-interface.h>
#include <gconf/gconf-client.h>

#include "tvout_gconf_keys.h"

static GConfClient * gconf_client;
static GtkWidget * button_enable;
static GtkWidget * button_tv_std;
static GtkWidget * button_aspect;
static GtkWidget * button_scale;
static GtkWidget * selector_tv_std;
static GtkWidget * selector_aspect;
static GtkWidget * selector_scale;

static void update(GObject * object, gpointer user_data G_GNUC_UNUSED) {

	GtkWidget * button = GTK_WIDGET(object);

	if ( button == button_enable )
		gconf_client_set_bool(gconf_client, TVOUT_GCONF_ENABLE_KEY, hildon_check_button_get_active(HILDON_CHECK_BUTTON(button_enable)), NULL);
	else if ( button == button_tv_std )
		gconf_client_set_string(gconf_client, TVOUT_GCONF_TV_STD_KEY, hildon_touch_selector_get_active(HILDON_TOUCH_SELECTOR(selector_tv_std), 0) == 0 ? "PAL" : "NTSC", NULL);
	else if ( button == button_aspect )
		gconf_client_set_string(gconf_client, TVOUT_GCONF_ASPECT_KEY, hildon_touch_selector_get_active(HILDON_TOUCH_SELECTOR(selector_aspect), 0) == 0 ? "NORMAL" : "WIDE", NULL);
	else if ( button == button_scale )
		gconf_client_set_int(gconf_client, TVOUT_GCONF_SCALE_KEY, hildon_touch_selector_get_active(HILDON_TOUCH_SELECTOR(selector_scale), 0) + 1, NULL);

}

osso_return_t execute(osso_context_t * osso G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED, gboolean user_activated G_GNUC_UNUSED) {

	GdkGeometry geometry;
	GtkWidget * dialog;
	GtkWidget * box;
	GtkWidget * gbox;
	GtkWidget * pan;
	char * str_tv_std;
	char * str_aspect;
	int enable;
	int tv_std;
	int aspect;
	int scale;
	int responce;
	int i;

	gconf_client = gconf_client_get_default();
	dialog = gtk_dialog_new_with_buttons(dgettext("osso-tv-out", "tvou_ap_cpa"), GTK_WINDOW(user_data), GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR, dgettext("hildon-libs", "wdgt_bd_save"), GTK_RESPONSE_ACCEPT, NULL);
	box = gtk_vbox_new(TRUE, 0);

	if ( ! gconf_client || ! dialog || ! box )
		return OSSO_ERROR;

	button_enable = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	button_tv_std = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
	button_aspect = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);
	button_scale = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL);

	selector_tv_std = hildon_touch_selector_new_text();
	selector_aspect = hildon_touch_selector_new_text();
	selector_scale = hildon_touch_selector_new_text();

	enable = gconf_client_get_bool(gconf_client, TVOUT_GCONF_ENABLE_KEY, NULL);
	scale = gconf_client_get_int(gconf_client, TVOUT_GCONF_SCALE_KEY, NULL);

	str_tv_std = gconf_client_get_string(gconf_client, TVOUT_GCONF_TV_STD_KEY, NULL);
	str_aspect = gconf_client_get_string(gconf_client, TVOUT_GCONF_ASPECT_KEY, NULL);

	if ( scale < 1 )
		scale = 0;
	else if ( scale > 100 )
		scale = 99;
	else
		--scale;

	if ( str_tv_std )
		tv_std = strcmp(str_tv_std, "PAL") == 0 ? 0 : 1;
	else
		tv_std = 0;

	if ( str_aspect )
		aspect = strcmp(str_aspect, "NORMAL") == 0 ? 0 : 1;
	else
		aspect = 0;

	g_free(str_aspect);
	g_free(str_tv_std);

	gtk_button_set_label(GTK_BUTTON(button_enable), "Enable"); /* TODO: dgettext */
	gtk_button_set_alignment(GTK_BUTTON(button_enable), 0, 0.5);
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(button_enable), enable);
	g_signal_connect(button_enable, "toggled", G_CALLBACK(update), NULL);

	hildon_button_set_title(HILDON_BUTTON(button_tv_std), dgettext("osso-tv-out", "tvou_fi_tv_out"));
	gtk_button_set_alignment(GTK_BUTTON(button_tv_std), 0, 0.5);
	hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(button_tv_std), HILDON_TOUCH_SELECTOR(selector_tv_std));
	hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(selector_tv_std), dgettext("osso-tv-out", "tvou_va_tv_out_pal"));
	hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(selector_tv_std), dgettext("osso-tv-out", "tvou_va_tv_out_ntsc"));
	hildon_touch_selector_set_active(HILDON_TOUCH_SELECTOR(selector_tv_std), 0, tv_std);
	g_signal_connect(button_tv_std, "value_changed", G_CALLBACK(update), NULL);

	hildon_button_set_title(HILDON_BUTTON(button_aspect), "Aspect"); /* TODO: dgettext */
	gtk_button_set_alignment(GTK_BUTTON(button_aspect), 0, 0.5);
	hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(button_aspect), HILDON_TOUCH_SELECTOR(selector_aspect));
	hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(selector_aspect), "Normal (4:3)"); /* TODO: dgettext */
	hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(selector_aspect), "Wide (16:9)"); /* TODO: dgettext */
	hildon_touch_selector_set_active(HILDON_TOUCH_SELECTOR(selector_aspect), 0, aspect);
	g_signal_connect(button_aspect, "value_changed", G_CALLBACK(update), NULL);

	hildon_button_set_title(HILDON_BUTTON(button_scale), "Scale"); /* TODO: dgettext */
	gtk_button_set_alignment(GTK_BUTTON(button_scale), 0, 0.5);
	hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(button_scale), HILDON_TOUCH_SELECTOR(selector_scale));

	for ( i = 1; i <= 100; ++i )
		hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(selector_scale), g_strdup_printf("%d %%", i));

	hildon_touch_selector_set_active(HILDON_TOUCH_SELECTOR(selector_scale), 0, scale);
	g_signal_connect(button_scale, "value_changed", G_CALLBACK(update), NULL);

	gtk_container_add(GTK_CONTAINER(box), button_enable);
	gtk_container_add(GTK_CONTAINER(box), button_tv_std);
	gtk_container_add(GTK_CONTAINER(box), button_aspect);
	gtk_container_add(GTK_CONTAINER(box), button_scale);

	geometry.min_width = 480;

	if ( gdk_screen_get_width(gdk_display_get_default_screen(gdk_display_get_default())) < 800 )
		geometry.min_height = 500;
	else
		geometry.min_height = 360;

	gtk_window_set_geometry_hints(GTK_WINDOW(dialog), dialog, &geometry, GDK_HINT_MIN_SIZE);

	gbox = gtk_vbox_new(FALSE, 0);

	if ( ! gbox )
		return OSSO_ERROR;

	gtk_box_pack_start(GTK_BOX(gbox), box, TRUE, FALSE, 0);

	pan = hildon_pannable_area_new();

	if ( ! pan )
		return OSSO_ERROR;

	hildon_pannable_area_add_with_viewport(HILDON_PANNABLE_AREA(pan), gbox);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), pan, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);
	responce = gtk_dialog_run(GTK_DIALOG(dialog));

	if ( responce != GTK_RESPONSE_ACCEPT ) {

		gconf_client_set_string(gconf_client, TVOUT_GCONF_TV_STD_KEY, tv_std == 0 ? "PAL" : "NTSC", NULL);
		gconf_client_set_string(gconf_client, TVOUT_GCONF_ASPECT_KEY, aspect == 0 ? "NORMAL" : "WIDE", NULL);
		gconf_client_set_int(gconf_client, TVOUT_GCONF_SCALE_KEY, scale + 1, NULL);

		/* this key must be last, make sure that status applet change all other x11 properties */
		sleep(1);
		gconf_client_set_bool(gconf_client, TVOUT_GCONF_ENABLE_KEY, enable, NULL);

	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
	g_object_unref(gconf_client);

	return OSSO_OK;

}

osso_return_t save_state(osso_context_t * osso G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {

	return OSSO_OK;

}
