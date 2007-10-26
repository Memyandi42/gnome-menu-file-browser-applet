/*
 * File:				menu-browser.h
 * Created:				September 2005
 * Created by:			Axel von Bertoldi
 * Last Modified:		August 2007
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

#ifndef MENU_BROWSER_H
#define MENU_BROWSER_H

#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>

#include "utils.h"
#include "preferences.h"

#define MAX_FILE_NAME_LENGTH 30 /* What's a good value here??? */


/******************************************************************************/
typedef struct _MenuFileBrowser			MenuFileBrowser;
typedef struct _MenuFileBrowserPrivate	MenuFileBrowserPrivate;
/******************************************************************************/

/****************** "Public" data *********************************************/
struct _MenuFileBrowser {
	GtkWidget				*menu_item;	
	MenuFileBrowserPrivate	*priv;
	BrowserPrefs			*prefs;
};

enum {
	EXEC_OPEN,
	EXEC_RUN
};
/******************************************************************************/

/****************** "Public" functions ****************************************/
MenuFileBrowser		*menu_browser_new (const gchar *label, const gchar *root_path, BrowserPrefs *cgf);
gint				 menu_browser_open_file (const gchar *file_name_and_path, gint exec_action);
void				 menu_browser_delete (MenuFileBrowser *browser);
/******************************************************************************/


#endif /*MENU_BROWSER_H*/
