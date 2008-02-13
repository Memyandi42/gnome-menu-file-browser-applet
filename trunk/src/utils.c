/*
 * File:				utils.c
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

#include <glib/gprintf.h>

#include "utils.h"

/******************************************************************************/
gboolean
utils_check_gerror (GError **error)
{
	if (*error)
	{		
		if (DEBUG) g_printf ("error: %s\n", (*error)->message);
		utils_show_dialog ("Application Error",
						   (*error)->message,
						   GTK_MESSAGE_ERROR);
		g_error_free (*error);
		*error = NULL;
		return TRUE;
	}
	return FALSE;
}
/******************************************************************************/
void
utils_show_dialog (gchar *title, gchar *message, GtkMessageType type)
{
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
												0,
												type,
												GTK_BUTTONS_CLOSE,
												message);
	gtk_window_set_title (GTK_WINDOW (dialog), title);

	g_signal_connect (G_OBJECT (dialog), "destroy",
					  G_CALLBACK (gtk_widget_destroy), dialog);
    g_signal_connect (G_OBJECT (dialog), "delete_event",
                      G_CALLBACK (gtk_widget_destroy), dialog);
	g_signal_connect_swapped (G_OBJECT (dialog),
							  "response", 
							  G_CALLBACK (gtk_widget_destroy),
							  dialog);
	gtk_widget_show_all (dialog);

	return;
}
/******************************************************************************/
GtkWidget*
utils_get_scaled_image_from_file (gchar *file_name, int size) {
		GdkPixbuf *orig   = gdk_pixbuf_new_from_file (file_name ,NULL);

		GdkPixbuf *scaled = gdk_pixbuf_scale_simple (orig,
                                          			 size,
										  			 size,
										  			 GDK_INTERP_HYPER);
		GtkWidget *icon = gtk_image_new_from_pixbuf (scaled);
		g_object_unref (orig);
		g_object_unref (scaled);
		return icon;
}
/******************************************************************************/
GSList *
g_slist_swap_data (GSList *list, guint index) {
	gpointer tmp   = NULL;
	GSList *first  = g_slist_nth (list, index);
	GSList *second = first->next;

	tmp = first->data;
	first->data  = second->data;
	second->data = tmp;
	return list;
}
/******************************************************************************/
