/*
 * File:                menu-browser.h
 * Created:             September 2005
 * Created by:          Axel von Bertoldi
 * Last Modified:       August 2008
 * Last Modified by:    Axel von Bertoldi
 * (C) 2005-2008        Axel von Bertoldi
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

#ifndef __MENU_BROWSER_H__
#define __MENU_BROWSER_H__

#include <gtk/gtk.h>

#include "preferences.h"

G_BEGIN_DECLS

/******************************************************************************/
#define TYPE_MENU_BROWSER (menu_browser_get_type ())
#define MENU_BROWSER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MENU_BROWSER, MenuBrowser))
#define MENU_BROWSER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MENU_BROWSER, MenuBrowserClass))
#define IS_MENU_BROWSER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MENU_BROWSER))
#define IS_MENU_BROWSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MENU_BROWSER))
#define MENU_BROWSER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MENU_BROWSER, MenuBrowserClass))
/******************************************************************************/
typedef struct _MenuBrowser MenuBrowser;
typedef struct _MenuBrowserClass MenuBrowserClass;
typedef struct _MenuBrowserPrivate MenuBrowserPrivate;
/******************************************************************************/
struct _MenuBrowser {
    GtkImageMenuItem    parent;
    MenuBrowserPrivate  *priv;
    BrowserPrefs        *prefs;
};
struct _MenuBrowserClass {
    GtkImageMenuItemClass parent;
};
enum {
    EXEC_OPEN,
    EXEC_RUN
};
/******************************************************************************/
GtkWidget* menu_browser_new (const gchar *path, const gchar *label, BrowserPrefs *prefs);
void       menu_browser_update (MenuBrowser *self, const gchar *path, const gchar *label);
GType      menu_browser_get_type (void);
/******************************************************************************/

G_END_DECLS

#endif
