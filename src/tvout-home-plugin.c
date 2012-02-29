/*
 * Maemo TV out control home plugin
 * Copyright (C) 2010-2012  Ville Syrjälä <syrjala@sci.fi>
 * Copyright (C) 2011       Pali Rohár <pali.rohar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <string.h>

#include <hildon/hildon.h>
#include <gconf/gconf-client.h>

#include <tvout-ctl.h>

#include "tvout-home-plugin.h"
#include "tvout_gconf_keys.h"

HD_DEFINE_PLUGIN_MODULE (TVoutHomePlugin, tvout_home_plugin, HD_TYPE_HOME_PLUGIN_ITEM)

#define TVOUT_HOME_PLUGIN_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TVOUT_TYPE_HOME_PLUGIN, TVoutHomePluginPrivate))

struct _TVoutHomePluginPrivate
{
  GConfClient *gconf_client;
  guint gconf_enable;
  guint gconf_tv_std;
  guint gconf_aspect;
  guint gconf_scale;

  GtkWidget *enable_button;
  GtkWidget *tv_std_button;
  GtkWidget *aspect_button;
  GtkWidget *scale_dec_button;
  GtkWidget *scale_inc_button;
  GtkWidget *scale_label;
};

static void tvout_ui_set_enable (TVoutHomePluginPrivate *priv, gboolean value)
{
  static const gchar *labels[] = {
    "OFF",
    "ON",
  };

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->enable_button), labels[value]);
}

static gint tv_std_to_int (const gchar *value)
{
  if (strcmp (value, "PAL") == 0)
    return 0;
  else if (strcmp (value, "NTSC") == 0)
    return 1;
  else
    return -1;
}

static void tvout_ui_set_tv_std (TVoutHomePluginPrivate *priv,
                                 const gchar *str)
{
  static const gchar *labels[] = {
    "PAL",
    "NTSC",
  };
  gint value = tv_std_to_int (str);

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->tv_std_button), labels[value]);
}

static gint aspect_to_int (const gchar *value)
{
  if (strcmp (value, "NORMAL") == 0)
    return 0;
  else if (strcmp (value, "WIDE") == 0)
    return 1;
  else
    return -1;
}

static void tvout_ui_set_aspect (TVoutHomePluginPrivate *priv,
                                 const gchar *str)
{
  static const gchar *labels[] = {
    "4:3",
    "16:9",
  };
  gint value = aspect_to_int (str);

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->aspect_button), labels[value]);
}

static void tvout_ui_set_scale (TVoutHomePluginPrivate *priv, gint value)
{
  gchar text[4];

  if (value < 1 || value > 100)
    return;

  g_snprintf (text, sizeof text, "%d", value);
  gtk_label_set_text (GTK_LABEL (priv->scale_label), text);

  gtk_widget_set_sensitive (priv->scale_dec_button, value > 1);
  gtk_widget_set_sensitive (priv->scale_inc_button, value < 100);
}

static void tvout_ui_update (GConfClient *client G_GNUC_UNUSED,
                             guint cnxn_id G_GNUC_UNUSED, GConfEntry *entry,
                             gpointer data)
{
  TVoutHomePluginPrivate *priv = data;
  const gchar *key = gconf_entry_get_key (entry);
  GConfValue *value = gconf_entry_get_value (entry);

  if (strcmp (key, TVOUT_GCONF_ENABLE_KEY) == 0)
    tvout_ui_set_enable (priv, gconf_value_get_bool (value));
  else if (strcmp (key, TVOUT_GCONF_TV_STD_KEY) == 0)
    tvout_ui_set_tv_std (priv, gconf_value_get_string (value));
  else if (strcmp (key, TVOUT_GCONF_ASPECT_KEY) == 0)
    tvout_ui_set_aspect (priv, gconf_value_get_string (value));
  else if (strcmp (key, TVOUT_GCONF_SCALE_KEY) == 0)
    tvout_ui_set_scale (priv, gconf_value_get_int (value));
}

static void enable_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  gboolean value = !gconf_client_get_bool (priv->gconf_client,
                                           TVOUT_GCONF_ENABLE_KEY, NULL);

  gconf_client_set_bool (priv->gconf_client,
                         TVOUT_GCONF_ENABLE_KEY, value, NULL);
}

static void tv_std_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  gchar *old = gconf_client_get_string (priv->gconf_client,
                                        TVOUT_GCONF_TV_STD_KEY, NULL);
  const gchar *value = NULL;

  if (strcmp (old, "PAL") == 0)
    value = "NTSC";
  if (strcmp (old, "NTSC") == 0)
    value = "PAL";

  g_free (old);

  if (value)
    gconf_client_set_string (priv->gconf_client,
                             TVOUT_GCONF_TV_STD_KEY, value, NULL);
}

static void aspect_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  gchar *old = gconf_client_get_string (priv->gconf_client,
                                        TVOUT_GCONF_ASPECT_KEY, NULL);
  const gchar *value = NULL;

  if (strcmp (old, "NORMAL") == 0)
    value = "WIDE";
  if (strcmp (old, "WIDE") == 0)
    value = "NORMAL";

  g_free (old);

  if (value)
    gconf_client_set_string (priv->gconf_client,
                             TVOUT_GCONF_ASPECT_KEY, value, NULL);
}

static void scale_dec_button_clicked (GtkButton *widget,
                                      gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  gint value =  gconf_client_get_int (priv->gconf_client,
                                      TVOUT_GCONF_SCALE_KEY, NULL) - 1;

  if (value < 1 || value > 100)
    return;

  gconf_client_set_int (priv->gconf_client,
                        TVOUT_GCONF_SCALE_KEY, value, NULL);
}

static void scale_inc_button_clicked (GtkButton *widget,
                                      gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  gint value = gconf_client_get_int (priv->gconf_client,
                                     TVOUT_GCONF_SCALE_KEY, NULL) + 1;

  if (value < 1 || value > 100)
    return;

  gconf_client_set_int (priv->gconf_client,
                        TVOUT_GCONF_SCALE_KEY, value, NULL);
}

static GtkWidget *create_ui_enable (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;
  gboolean value;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("TV out");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->enable_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "OFF");
  gtk_button_set_label (GTK_BUTTON (button), "ON");

  value = gconf_client_get_bool (priv->gconf_client,
                                 TVOUT_GCONF_ENABLE_KEY, NULL);
  tvout_ui_set_enable (priv, value);

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (enable_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_tv_std (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;
  gchar *value;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Video format");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->tv_std_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "PAL");
  gtk_button_set_label (GTK_BUTTON (button), "NTSC");

  value = gconf_client_get_string (priv->gconf_client,
                                   TVOUT_GCONF_TV_STD_KEY, NULL);
  tvout_ui_set_tv_std (priv, value);
  g_free (value);

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (tv_std_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_aspect (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;
  gchar *value;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Aspect ratio");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->aspect_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "4:3");
  gtk_button_set_label (GTK_BUTTON (button), "16:9");

  value = gconf_client_get_string (priv->gconf_client,
                                   TVOUT_GCONF_ASPECT_KEY, NULL);
  tvout_ui_set_aspect (priv, value);
  g_free (value);

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (aspect_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_scale (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;
  gint value;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Scale");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->scale_dec_button = button = gtk_button_new_with_label ("<");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (scale_dec_button_clicked), priv);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  priv->scale_label = label = gtk_label_new ("0");
  value = gconf_client_get_int (priv->gconf_client,
                                TVOUT_GCONF_SCALE_KEY, NULL);
  tvout_ui_set_scale (priv, value);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->scale_inc_button = button = gtk_button_new_with_label (">");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (scale_inc_button_clicked), priv);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui (TVoutHomePlugin *self)
{
  TVoutHomePluginPrivate *priv = self->priv;
  GtkWidget *vbox, *widget;

  vbox = gtk_vbox_new (0, FALSE);

  widget = create_ui_enable (priv);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

  widget = create_ui_tv_std (priv);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

  widget = create_ui_aspect (priv);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

  widget = create_ui_scale (priv);
  gtk_box_pack_start_defaults (GTK_BOX (vbox), widget);

  return vbox;
}

static void
tvout_home_plugin_init (TVoutHomePlugin *self)
{
  TVoutHomePluginPrivate *priv;
  GtkWidget *contents;

  self->priv = priv = TVOUT_HOME_PLUGIN_GET_PRIVATE (self);

  priv->gconf_client = gconf_client_get_default ();
  if (!priv->gconf_client)
    return;

  gconf_client_add_dir (priv->gconf_client, TVOUT_GCONF_PATH,
                        GCONF_CLIENT_PRELOAD_NONE, NULL);

  priv->gconf_enable = gconf_client_notify_add (priv->gconf_client,
                                                TVOUT_GCONF_ENABLE_KEY,
                                                tvout_ui_update, priv,
                                                NULL, NULL);
  priv->gconf_tv_std = gconf_client_notify_add (priv->gconf_client,
                                                TVOUT_GCONF_TV_STD_KEY,
                                                tvout_ui_update, priv,
                                                NULL, NULL);
  priv->gconf_aspect = gconf_client_notify_add (priv->gconf_client,
                                                TVOUT_GCONF_ASPECT_KEY,
                                                tvout_ui_update, priv,
                                                NULL, NULL);
  priv->gconf_scale = gconf_client_notify_add (priv->gconf_client,
                                               TVOUT_GCONF_SCALE_KEY,
                                               tvout_ui_update, priv,
                                               NULL, NULL);

  contents = create_ui (self);

  gtk_widget_show_all (contents);

  gtk_container_add (GTK_CONTAINER (self), contents);
}

static void tvout_home_plugin_finalize (GObject *self)
{
  TVoutHomePluginPrivate *priv = TVOUT_HOME_PLUGIN (self)->priv;

  if (priv->gconf_client) {
    if (priv->gconf_enable)
      gconf_client_notify_remove (priv->gconf_client, priv->gconf_enable);

    if (priv->gconf_tv_std)
      gconf_client_notify_remove (priv->gconf_client, priv->gconf_tv_std);

    if (priv->gconf_aspect)
      gconf_client_notify_remove (priv->gconf_client, priv->gconf_aspect);

    if (priv->gconf_scale)
      gconf_client_notify_remove (priv->gconf_client, priv->gconf_scale);

    gconf_client_remove_dir (priv->gconf_client, TVOUT_GCONF_PATH, NULL);
    gconf_client_clear_cache (priv->gconf_client);
    g_object_unref (priv->gconf_client);
  }

  G_OBJECT_CLASS (tvout_home_plugin_parent_class)->finalize (self);
}

static void
tvout_home_plugin_class_finalize (TVoutHomePluginClass *klass)
{
}

static void
tvout_home_plugin_class_init (TVoutHomePluginClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = tvout_home_plugin_finalize;

  g_type_class_add_private (klass, sizeof (TVoutHomePluginPrivate));
}
