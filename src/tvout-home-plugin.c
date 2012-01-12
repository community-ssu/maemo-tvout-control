/*
 * Maemo TV out control home plugin
 * Copyright (C) 2010-2012  Ville Syrjälä <syrjala@sci.fi>
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

#include <hildon/hildon.h>

#include <tvout-ctl.h>

#include "tvout-home-plugin.h"

HD_DEFINE_PLUGIN_MODULE (TVoutHomePlugin, tvout_home_plugin, HD_TYPE_HOME_PLUGIN_ITEM)

#define TVOUT_HOME_PLUGIN_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TVOUT_TYPE_HOME_PLUGIN, TVoutHomePluginPrivate))

struct _TVoutHomePluginPrivate
{
  TVoutCtl *tvout_ctl;

  GtkWidget *enable_button;
  GtkWidget *tv_std_button;
  GtkWidget *aspect_button;
  GtkWidget *scale_dec_button;
  GtkWidget *scale_inc_button;
  GtkWidget *scale_label;

  GIOChannel *io;
  guint watch;
};

static void tvout_ui_set_enable (TVoutHomePluginPrivate *priv, gint value)
{
  static const gchar *labels[] = {
    "OFF",
    "ON",
  };

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->enable_button), labels[value]);
}

static void tvout_ui_set_tv_std (TVoutHomePluginPrivate *priv, gint value)
{
  static const gchar *labels[] = {
    "PAL",
    "NTSC",
  };

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->tv_std_button), labels[value]);
}

static void tvout_ui_set_aspect (TVoutHomePluginPrivate *priv, gint value)
{
  static const gchar *labels[] = {
    "4:3",
    "16:9",
  };

  if (value < 0 || value > 1)
    return;

  gtk_button_set_label (GTK_BUTTON (priv->aspect_button), labels[value]);
}

static void tvout_ui_set_scale (TVoutHomePluginPrivate *priv, gint value)
{
  gchar text[4];

  if (value < 1 || value > 100)
    return;

  g_snprintf(text, sizeof text, "%d", value);
  gtk_label_set_text (GTK_LABEL (priv->scale_label), text);

  gtk_widget_set_sensitive (priv->scale_dec_button, value > 1);
  gtk_widget_set_sensitive (priv->scale_inc_button, value < 100);
}

static void tvout_ui_notify (void *data, enum TVoutCtlAttr attr, int value)
{
  TVoutHomePluginPrivate *priv = data;

  switch (attr) {
  case TVOUT_CTL_ENABLE:
    tvout_ui_set_enable (priv, value);
    break;
  case TVOUT_CTL_TV_STD:
    tvout_ui_set_tv_std (priv, value);
    break;
  case TVOUT_CTL_ASPECT:
    tvout_ui_set_aspect (priv, value);
    break;
  case TVOUT_CTL_SCALE:
    tvout_ui_set_scale (priv, value);
    break;
  default:
    break;
  }
}

static void enable_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;

  tvout_ctl_set (priv->tvout_ctl, TVOUT_CTL_ENABLE,
		 !tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_ENABLE));
}

static void tv_std_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;

  tvout_ctl_set (priv->tvout_ctl, TVOUT_CTL_TV_STD,
		 !tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_TV_STD));
}

static void aspect_button_clicked (GtkButton *widget,
                                   gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;

  tvout_ctl_set (priv->tvout_ctl, TVOUT_CTL_ASPECT,
		 !tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_ASPECT));
}

static void scale_dec_button_clicked (GtkButton *widget,
                                      gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  int value;

  value = tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_SCALE) - 1;
  if (value < 1 || value > 100)
    return;

  tvout_ctl_set (priv->tvout_ctl, TVOUT_CTL_SCALE, value);
}

static void scale_inc_button_clicked (GtkButton *widget,
                                      gpointer user_data)
{
  TVoutHomePluginPrivate *priv = user_data;
  int value;

  value = tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_SCALE) + 1;
  if (value < 1 || value > 100)
    return;

  tvout_ctl_set (priv->tvout_ctl, TVOUT_CTL_SCALE, value);
}

static GtkWidget *create_ui_enable (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("TV out");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->enable_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "OFF");
  gtk_button_set_label (GTK_BUTTON (button), "ON");

  tvout_ui_set_enable (priv, tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_ENABLE));

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (enable_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_tv_std (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Video format");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->tv_std_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "PAL");
  gtk_button_set_label (GTK_BUTTON (button), "NTSC");

  tvout_ui_set_tv_std (priv, tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_TV_STD));

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (tv_std_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_aspect (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Aspect ratio");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->aspect_button = button = gtk_button_new ();

  gtk_button_set_label (GTK_BUTTON (button), "4:3");
  gtk_button_set_label (GTK_BUTTON (button), "16:9");

  tvout_ui_set_aspect (priv, tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_ASPECT));

  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (aspect_button_clicked), priv);

  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  return hbox;
}

static GtkWidget *create_ui_scale (TVoutHomePluginPrivate *priv)
{
  GtkWidget *hbox, *label, *button;

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new ("Scale");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), label);

  priv->scale_dec_button = button = gtk_button_new_with_label ("<");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (scale_dec_button_clicked), priv);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

  priv->scale_label = label = gtk_label_new ("0");
  tvout_ui_set_scale (priv, tvout_ctl_get (priv->tvout_ctl, TVOUT_CTL_SCALE));
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

static gboolean tvout_io_func (GIOChannel *source,
			       GIOCondition condition,
			       gpointer data)
{
  TVoutHomePluginPrivate *priv = data;

  tvout_ctl_fd_ready (priv->tvout_ctl);

  return TRUE;
}

static void create_io_channel (TVoutHomePluginPrivate *priv)
{
  GIOStatus s;
  int fd = tvout_ctl_fd (priv->tvout_ctl);

  if (fd < 0)
    return;

  priv->io = g_io_channel_unix_new (fd);
  if (!priv->io)
    return;

  s = g_io_channel_set_encoding (priv->io, NULL, NULL);
  if (s != G_IO_STATUS_NORMAL) {
    g_io_channel_unref (priv->io);
    priv->io = NULL;
    return;
  }

  g_io_channel_set_buffered (priv->io, FALSE);

  priv->watch = g_io_add_watch (priv->io, G_IO_IN | G_IO_PRI, tvout_io_func, priv);
  if (!priv->watch) {
    g_io_channel_unref (priv->io);
    priv->io = NULL;
    return;
  }
}

static void destroy_io_channel (TVoutHomePluginPrivate *priv)
{
  if (priv->watch) {
    g_source_remove (priv->watch);
    priv->watch = 0;
  }
  if (priv->io) {
    g_io_channel_unref (priv->io);
    priv->io = NULL;
  }
}

static void
tvout_home_plugin_init (TVoutHomePlugin *self)
{
  TVoutHomePluginPrivate *priv;
  GtkWidget *contents;

  self->priv = priv = TVOUT_HOME_PLUGIN_GET_PRIVATE (self);

  priv->tvout_ctl = tvout_ctl_init (tvout_ui_notify, priv);

  create_io_channel (priv);

  contents = create_ui (self);

  if (!priv->tvout_ctl)
    gtk_widget_set_sensitive (contents, FALSE);

  gtk_widget_show_all (contents);

  gtk_container_add (GTK_CONTAINER (self), contents);
}

static void tvout_home_plugin_finalize (GObject *self)
{
  TVoutHomePluginPrivate *priv = TVOUT_HOME_PLUGIN (self)->priv;

  destroy_io_channel (priv);

  tvout_ctl_exit (priv->tvout_ctl);

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
