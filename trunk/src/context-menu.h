/*
 * File:                context_menu.h
 * Created:             April 2008
 * Created by:          Axel von Bertoldi
 * Last Modified:       November 2009
 * Last Modified by:    Axel von Bertoldi
 * (C) 2005-2009        Axel von Bertoldi
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

#ifndef CONTEXT_MENU_H
#define CONTEXT_MENU_H

#include <gtk/gtk.h>

/******************************************************************************/
typedef struct _ContextMenuPrefs ContextMenuPrefs;
/******************************************************************************/
struct _ContextMenuPrefs {
    gchar       *terminal;
    gchar       *editor;
};
/******************************************************************************/
gboolean context_menu_display (const gchar *file_name, GtkWidget *menu_item, ContextMenuPrefs prefs);

#endif
