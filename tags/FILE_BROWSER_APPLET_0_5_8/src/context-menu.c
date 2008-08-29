/*
 * File:				context_menu.c
 * Created:				April 2008
 * Created by:			Axel von Bertoldi
 * Last Modified:		April 2008
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
#include <glib.h>

#include "context-menu.h"
#include "vfs.h"

/******************************************************************************/
static void
context_menu_add_delete_item (const gchar *file_name,
							  GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);
							  
	GtkWidget *menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_DELETE, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	
	GtkWidget *item_label = GTK_BIN (menu_item)->child;
	gtk_label_set_text (GTK_LABEL (item_label), "Move to Trash");
	
	g_signal_connect_swapped (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (vfs_trash_file),
							  (gpointer) file_name);
}
/******************************************************************************/
static void
context_menu_add_fake_items (const gchar *file_name,
							 GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GtkWidget *menu_item;
	
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ADD, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_APPLY, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_BOLD, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CDROM, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
}
/******************************************************************************/

static void
context_menu_populate (const gchar *file_name,
					   GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	context_menu_add_delete_item (file_name, menu);
	context_menu_add_fake_items (file_name, menu);
}
/******************************************************************************/
static void
context_menu_clean_up (GtkMenuShell *menu,
					   GtkWidget *browser) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (browser));
	gtk_menu_shell_deactivate (GTK_MENU_SHELL (parent));
	
/*	gtk_grab_remove (GTK_WIDGET (menu));*/
/*	gdk_pointer_ungrab (GDK_CURRENT_TIME);*/
	
	gtk_widget_destroy (GTK_WIDGET (menu));
}
/******************************************************************************/
gboolean
context_menu_display (const gchar *file_name,
					  GtkWidget *menu_item) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	int event_button;
	int event_time;

	GdkEventButton *event = g_object_get_data (G_OBJECT (menu_item),
											   "button_event");
	if (event) {
		event_button = event->button;
		event_time = event->time;
	}
	else {
		event_button = 3;
		event_time = gtk_get_current_event_time();
	}

	GtkWidget *browser = g_object_get_data (G_OBJECT (menu_item),
											  "menu_browser");

/*
GtkWidget *panel_menu_bar = gtk_widget_get_parent (GTK_WIDGET (browser));
GtkWidget *parent_menu = gtk_widget_get_parent (GTK_WIDGET (menu_item));
GtkMenuShell *panel_menu_bar_shell = GTK_MENU_SHELL (panel_menu_bar);
GtkWidget *applet = gtk_widget_get_parent (GTK_WIDGET (panel_menu_bar));
GtkWidget *panel = gtk_widget_get_parent (GTK_WIDGET (applet));
*/

	GtkWidget *menu = gtk_menu_new ();

	g_signal_connect (GTK_MENU_SHELL (menu),
  					  "selection_done",
					  G_CALLBACK (context_menu_clean_up),
					  browser);

	context_menu_populate (file_name, menu);

	gtk_widget_show_all (menu);

	gtk_menu_popup (GTK_MENU (menu),
					NULL,
					NULL,
					NULL,
					NULL,
					event_button,
					event_time);

/*gtk_grab_add (GTK_WIDGET (menu));*/
/*gdk_pointer_grab (GTK_WIDGET (menu)->window,
				  TRUE,
				  0,
				  NULL,
				  NULL,
				  GDK_CURRENT_TIME);*/

	return TRUE;
}
/******************************************************************************/
