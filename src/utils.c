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
#include "config.h"

/******************************************************************************/
void
garbage_init (Garbage *garabage) {
	GPtrArray **_garbage = (GPtrArray**)garabage;
	*_garbage = g_ptr_array_new();
}
/******************************************************************************/
void
garbage_empty (Garbage *garabage, gboolean reuse) {
	GPtrArray **_garbage = (GPtrArray**)garabage;

	g_ptr_array_foreach (*_garbage, (GFunc)g_free, NULL);
	g_ptr_array_free (*_garbage, TRUE);
	*_garbage = reuse ? g_ptr_array_new() : NULL;
}
/******************************************************************************/
void
garbage_add_item (Garbage garabage, gpointer item) {
	GPtrArray *_garbage = (GPtrArray*)garabage;
	g_ptr_array_add (_garbage, item);
}
/******************************************************************************/
gboolean
utils_gerror_ok (GError **error, gboolean show_error) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	if (*error) {
		if (DEBUG) g_printf ("error: %s\n", (*error)->message);

		if (show_error)
			utils_show_dialog (_("Error"), (*error)->message, GTK_MESSAGE_ERROR);

		g_error_free (*error);
		*error = NULL;
		return FALSE;
	}
	return TRUE;
}
/******************************************************************************/
void
utils_show_dialog (const gchar *title, const gchar *message, GtkMessageType type) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GtkWidget *dialog = gtk_message_dialog_new (NULL,
												0,
												type,
												GTK_BUTTONS_CLOSE,
												"%s",
												message);
	gtk_window_set_title (GTK_WINDOW (dialog), title);

	g_signal_connect (G_OBJECT (dialog), "destroy", G_CALLBACK (gtk_widget_destroy), dialog);
    g_signal_connect (G_OBJECT (dialog), "delete_event", G_CALLBACK (gtk_widget_destroy), dialog);
	g_signal_connect_swapped (G_OBJECT (dialog), "response", G_CALLBACK (gtk_widget_destroy), dialog);
	gtk_widget_show_all (dialog);

	return;
}
/******************************************************************************/
GtkWidget*
utils_get_scaled_image_from_file (const gchar *file_name, int size) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GdkPixbuf *pixbuf   = gdk_pixbuf_new_from_file_at_size (file_name,
															size,
															size,
															NULL);
		if (pixbuf == NULL) return NULL;

		GtkWidget *icon = gtk_image_new_from_pixbuf (pixbuf);
		g_object_unref (pixbuf);
		return icon;
}
/******************************************************************************/
GSList *
g_slist_swap_data (GSList *list, guint index) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gpointer tmp   = NULL;
	GSList *first  = g_slist_nth (list, index);
	GSList *second = first->next;

	tmp = first->data;
	first->data  = second->data;
	second->data = tmp;
	return list;
}
/******************************************************************************/
