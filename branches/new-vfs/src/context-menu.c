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
context_menu_add_create_archive (const gchar *file_name, GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GtkWidget *menu_item = gtk_image_menu_item_new_with_label ("Create Archive");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   gtk_image_new_from_icon_name ("package",
								   								 GTK_ICON_SIZE_MENU));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	
	LaunchAppInfo *app_info = g_new0 (LaunchAppInfo, 1);
	
	app_info->exec = g_strdup ("file-roller");
	app_info->args = g_strdup ("-d");
	app_info->file = g_strdup (file_name);
	app_info->wd = NULL;

	g_signal_connect_swapped (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (vfs_launch_appication),
							  (gpointer) app_info);
}
/******************************************************************************/
static void
context_menu_add_open_with_item (const gchar *file_name, GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GList *root = vfs_get_all_mime_applications (file_name);
	GList *apps = root;
	
	if (root == NULL) return;

	GtkWidget *menu_item = gtk_image_menu_item_new_with_label ("Open With");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	GtkWidget *sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
							   sub_menu);

	while (apps != NULL) {
		menu_item = gtk_image_menu_item_new_with_label (vfs_get_app_name_for_app_info (apps->data));
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
									   vfs_get_icon_for_app_info (apps->data));

		LaunchAppInfo *app_info = g_new0 (LaunchAppInfo, 1);
	
		app_info->exec = g_strdup (vfs_get_exec_for_app_info (apps->data));
		app_info->args = NULL;
		app_info->file = g_strdup (file_name);
		app_info->wd = g_path_get_dirname (file_name);

		g_signal_connect_swapped (GTK_MENU_ITEM (menu_item),
								  "activate",
								  G_CALLBACK (vfs_launch_appication),
								  (gpointer) app_info);

		gtk_menu_shell_append (GTK_MENU_SHELL (sub_menu), menu_item);

		g_object_unref (apps->data);
		apps = apps->next;
	}
	g_list_free (apps);
}
/******************************************************************************/
static void
context_menu_add_delete_item (const gchar *file_name, GtkWidget *menu) {
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
context_menu_add_fake_items (const gchar *file_name, GtkWidget *menu) {
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
context_menu_populate (const gchar *file_name, GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	context_menu_add_open_with_item	(file_name, menu);
	context_menu_add_delete_item	(file_name, menu);
	context_menu_add_create_archive	(file_name, menu);
	/*context_menu_add_fake_items (file_name, menu);*/
}
/******************************************************************************/
static void
context_menu_clean_up (GtkMenuShell *menu, GtkWidget *browser) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (browser));
	gtk_menu_shell_deactivate (GTK_MENU_SHELL (parent));
	
/*	gtk_grab_remove (GTK_WIDGET (menu));*/
/*	gdk_pointer_ungrab (GDK_CURRENT_TIME);*/
	
	gtk_widget_destroy (GTK_WIDGET (menu));
}
/******************************************************************************/
gboolean
context_menu_display (const gchar *file_name, GtkWidget *menu_item) {
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
