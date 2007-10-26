/*
 * File:				preferences.h
 * Created:				April 2006
 * Created by:			Axel von Bertoldi
 * Last Modified:		October 2007
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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <glib.h>
#include <glib-object.h>
#include <panel-applet-gconf.h>

#include "utils.h"

G_BEGIN_DECLS

/******************************************************************************/
#define TYPE_APPLET_PREFERENCES (applet_preferences_get_type ())
#define APPLET_PREFERENCES(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_APPLET_PREFERENCES, AppletPreferences))
#define APPLET_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_APPLET_PREFERENCES, AppletPreferencesClass))
#define IS_APPLET_PREFERENCES(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_APPLET_PREFERENCES))
#define IS_APPLET_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_APPLET_PREFERENCES))
#define APPLET_PREFERENCES_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_APPLET_PREFERENCES, AppletPreferencesClass))
/******************************************************************************/
#define KEY_DIR			"dirs"
#define KEY_LABELS		"labels"
#define KEY_ICON_NAME	"icon"
#define KEY_ICON_SHOW	"show_icon"
#define KEY_HIDDEN_SHOW	"show_hidden"
#define KEY_TERMINAL	"terminal"

/* default options */
#define DEFAULT_ICON		"/usr/share/pixmaps/menu-file-browser-applet.png"
#define DEFAULT_SHOW_ICON	TRUE
#define DEFAULT_LABEL		"Home"
#define DEFAULT_PATH		g_get_home_dir ()
#define DEFAULT_TERMINAL	"gnome-terminal"
#define DEFAULT_SHOW_HIDDEN	FALSE
/******************************************************************************/
typedef struct _BrowserPrefs BrowserPrefs;
typedef struct _MenuBarPrefs MenuBarPrefs;
typedef struct _AppletPreferences AppletPreferences;
typedef struct _AppletPreferencesClass AppletPreferencesClass;
typedef struct _AppletPreferencesPrivate AppletPreferencesPrivate;
/******************************************************************************/
struct _BrowserPrefs {
	gchar		*terminal;
	gboolean	show_hidden;
};
struct _MenuBarPrefs {
	GPtrArray		*dirs;
	GPtrArray		*labels;
	gboolean 		show_icon;
	gchar 			*icon;
	BrowserPrefs	*browser_prefs;
};
struct _AppletPreferences {
	GObject						parent;
	MenuBarPrefs 				*menu_bar_prefs;
	AppletPreferencesPrivate	*priv;
};
struct _AppletPreferencesClass {
	GObjectClass parent;
};
/******************************************************************************/
AppletPreferences* applet_preferences_new (PanelApplet *applet);
void applet_preferences_make_window (AppletPreferences *preferences);
GType applet_preferences_get_type (void);
/******************************************************************************/

G_END_DECLS

#endif
