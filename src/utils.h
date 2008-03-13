/*
 * File:				utils.h
 * Created:				August 2007
 * Created by:			Axel von Bertoldi
 * Last Modified:		January 2008
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

#ifndef UTILS_H
#define UTILS_H

#include <glib.h>
#include <gtk/gtk.h>

#include "config.h"

#define DEBUG 1

gboolean	utils_check_gerror (GError **error);
void		utils_show_dialog (gchar *title, gchar *message, GtkMessageType type);
GtkWidget*	utils_get_scaled_image_from_file (gchar *file_name, int size);
GSList*		g_slist_swap_data (GSList *list, guint index);
gint		utils_sort_alpha (const gchar **s1, const gchar **s2);
gchar*		utils_clamp_file_name (const gchar *file_name, int length, gboolean *clamped);

#endif
