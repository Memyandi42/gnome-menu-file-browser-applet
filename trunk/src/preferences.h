/*
 * File:				preferences.h
 * Created:				April 2006
 * Created by:			Axel von Bertoldi
 * Last Modified:		April 2007
 * Last Modified by:	Axel von Bertoldi
 * (C) 2005,2006,2007	Axel von Bertoldi
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to:
 * The Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1301, USA.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include "menu-browser.h"
#include "utils.h"

#define MFBARC ".mfbarc"
#define DIR_GROUP "Dirs"
#define PREF_GROUP "Preferences"

#define KEY_DIR "dir"
#define KEY_ICON_NAME "icon"
#define KEY_ICON_SHOW "show_icon"
#define KEY_HIDDEN_SHOW "show_hidden"
#define KEY_TERMINAL "terminal"

/* default options */
#define DEFAULT_ICON "/usr/share/pixmaps/menu-file-browser-applet.png"
#define DEFAULT_SHOW_ICON TRUE
#define DEFAULT_LABEL "Home"
#define DEFAULT_PATH g_get_home_dir ()

/****************** "Public" data *********************************************/
typedef struct
{
	GPtrArray *dirs;
	GPtrArray *labels;
	gboolean  show_icon;
	gchar	  *icon;

	BrowserPreferences *browser_prefs;
} AppletPreferences;
/******************************************************************************/

/****************** "Public" functions ****************************************/
AppletPreferences *preferences_get  ();
gboolean		   preferences_save (AppletPreferences *prefs);
void			   preferences_delete (AppletPreferences *prefs);
void			   preferences_make_window (AppletPreferences *prefs);
/******************************************************************************/

#endif
