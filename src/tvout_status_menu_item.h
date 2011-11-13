/*
    tvout_status_menu_item.h - TV out Status Menu Item Hildon plugin
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

#ifndef __TVOUT_STATUS_MENU_ITEM_H__
#define __TVOUT_STATUS_MENU_ITEM_H__

#include <glib.h>
#include <glib-object.h>
#include <libhildondesktop/libhildondesktop.h>

G_BEGIN_DECLS

#define TVOUT_TYPE_STATUS_MENU_ITEM		(tvout_status_menu_item_get_type())
#define TVOUT_STATUS_MENU_ITEM(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), TVOUT_TYPE_STATUS_MENU_ITEM, TVoutStatusMenuItem))
#define TVOUT_IS_STATUS_MENU_ITEM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TVOUT_TYPE_STATUS_MENU_ITEM))
#define TVOUT_STATUS_MENU_ITEM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), TVOUT_TYPE_STATUS_MENU_ITEM, TVoutStatusMenuItemClass))
#define TVOUT_IS_STATUS_MENU_ITEM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), TVOUT_TYPE_STATUS_MENU_ITEM))
#define TVOUT_STATUS_MENU_ITEM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), TVOUT_TYPE_STATUS_MENU_ITEM, TVoutStatusMenuItemClass))

typedef struct _TVoutStatusMenuItem		TVoutStatusMenuItem;
typedef struct _TVoutStatusMenuItemClass	TVoutStatusMenuItemClass;
typedef struct _TVoutStatusMenuItemPrivate	TVoutStatusMenuItemPrivate;

struct _TVoutStatusMenuItem {
	HDStatusMenuItem parent_instance;
	TVoutStatusMenuItemPrivate * priv;
};

struct _TVoutStatusMenuItemClass {
	HDStatusMenuItemClass parent_class;
};

GType tvout_status_menu_item_get_type(void);

G_END_DECLS

#endif /* __TVOUT_STATUS_MENU_ITEM_H__ */
