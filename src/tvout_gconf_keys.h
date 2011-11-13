/*
    tvout_gconf_keys.h - TV out GConf keys handled by Status Item Hildon plugin
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

#ifndef TVOUT_GCONF_KEYS
#define TVOUT_GCONF_KEYS

/* GConf keys monitored and handled by this status menu item plugin */
#define TVOUT_GCONF_PATH		"/system"
#define TVOUT_GCONF_ENABLE_KEY		TVOUT_GCONF_PATH "/tvout_enable"	/* handled only by this tvout status item plugin */
#define TVOUT_GCONF_TV_STD_KEY		TVOUT_GCONF_PATH "/tvout"		/* handled by default on maemo5 (ohm and dres) */
#define TVOUT_GCONF_ASPECT_KEY		TVOUT_GCONF_PATH "/aspectratio"		/* handled by default on meego (ohm and dres) but not on maemo5 */
#define TVOUT_GCONF_SCALE_KEY		TVOUT_GCONF_PATH "/tvout_scale"		/* handled only by this tvout status item plugin */

#endif
