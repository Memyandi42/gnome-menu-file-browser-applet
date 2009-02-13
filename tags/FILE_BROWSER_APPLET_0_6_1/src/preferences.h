/*
 * File:				preferences.h
 * Created:				April 2006
 * Created by:			Axel von Bertoldi
 * Last Modified:		August 2008
 * Last Modified by:	Axel von Bertoldi
 * (C) 2005-2008		Axel von Bertoldi
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

#include <glib-object.h>
#include <panel-applet-gconf.h>

G_BEGIN_DECLS

/******************************************************************************/
#define TYPE_APPLET_PREFERENCES (applet_preferences_get_type ())
#define APPLET_PREFERENCES(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_APPLET_PREFERENCES, AppletPreferences))
#define APPLET_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_APPLET_PREFERENCES, AppletPreferencesClass))
#define IS_APPLET_PREFERENCES(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_APPLET_PREFERENCES))
#define IS_APPLET_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_APPLET_PREFERENCES))
#define APPLET_PREFERENCES_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_APPLET_PREFERENCES, AppletPreferencesClass))
/******************************************************************************/
#define KEY_DIR				"dirs"
#define KEY_LABELS			"labels"
#define KEY_ICON_NAME		"icon"
#define KEY_ICON_SHOW		"show_icon"
#define KEY_HIDDEN_SHOW		"show_hidden"
#define KEY_TERMINAL		"terminal"
#define KEY_EDITOR			"editor"
#define KEY_HORIZONTAL_TEXT	"horizontal_text"

/* default options */
#define DEFAULT_ICON			"user-home"
#define DEFAULT_SHOW_ICON		TRUE
#define DEFAULT_LABEL			"Home"
#define DEFAULT_PATH			g_get_home_dir ()
#define DEFAULT_TERMINAL		"gnome-terminal"
#define DEFAULT_EDITOR			"gedit"
#define DEFAULT_SHOW_HIDDEN		FALSE
#define DEFAULT_HORIZONTAL_TEXT	TRUE
/******************************************************************************/
typedef struct _BrowserPrefs BrowserPrefs;
typedef struct _MenuBarPrefs MenuBarPrefs;
typedef struct _AppletPreferences AppletPreferences;
typedef struct _AppletPreferencesClass AppletPreferencesClass;
typedef struct _AppletPreferencesPrivate AppletPreferencesPrivate;
typedef struct _PrefsChangedSignalData PrefsChangedSignalData;
/******************************************************************************/
struct _BrowserPrefs {
	gchar		*terminal;
	gchar		*editor;
	gboolean	show_hidden;
};
struct _MenuBarPrefs {
	GSList			*dirs;
	GSList			*labels;
	gboolean 		show_icon;
	gboolean 		horizontal_text;
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
	void (*prefs_changed)(AppletPreferences *a_prefs, PrefsChangedSignalData *signal_data);
};
struct _PrefsChangedSignalData {
	gint	signal_id;
	gint	instance;
	gchar	*label;
	gchar	*path;
};
/******************************************************************************/
enum {
	PREFS_SIGNAL_TERMINAL,
	PREFS_SIGNAL_SHOW_HIDDEN,
	PREFS_SIGNAL_SHOW_ICON,
	PREFS_SIGNAL_ICON_CHANGED,
	PREFS_SIGNAL_DIR_CHANGED,
	PREFS_SIGNAL_DIR_MOVE_UP,
	PREFS_SIGNAL_DIR_MOVE_DOWN,
	PREFS_SIGNAL_DIR_ADD,
	PREFS_SIGNAL_DIR_DEL,
	PREFS_SIGNAL_HORIZONTAL_TEXT
};
/******************************************************************************/
AppletPreferences*	applet_preferences_new (PanelApplet *applet);
void				applet_preferences_make_dialog (AppletPreferences *preferences);
GType				applet_preferences_get_type (void);
/******************************************************************************/

G_END_DECLS

#endif
