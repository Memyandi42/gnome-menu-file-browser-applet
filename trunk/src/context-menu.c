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

#include <glade/glade-xml.h>
#include <panel-applet.h>
#include <glib/gprintf.h>
#include <glib.h>

#include "context-menu.h"
#include "vfs.h"

Garbage garbage = NULL;

/******************************************************************************/
static char *archive_mime_types[] = {
	"application/x-ar",
	"application/x-arj",
	"application/x-bzip",
	"application/x-bzip-compressed-tar",
	"application/x-compress",
	"application/x-compressed-tar",
	"application/x-deb",
	"application/x-gtar",
	"application/x-gzip",
	"application/x-lha",
	"application/x-lhz",
	"application/x-rar",
	"application/x-rar-compressed",
	"application/x-tar",
	"application/x-zip",
	"application/x-zip-compressed",
	"application/zip",
	"multipart/x-zip",
	"application/x-rpm",
	"application/x-jar",
	"application/x-java-archive",
	"application/x-lzop",
	"application/x-zoo",
	"application/x-cd-image",
	"application/x-7z-compressed",
	"application/x-gzpostscript",
	"application/x-ms-dos-executable",
	NULL
};
/******************************************************************************/
/*
static void
context_menu_add_burn_callback (const gchar *file_name) {
	GFile *source = g_file_new_for_path (file_name);
	GFile *destination = g_file_new_for_uri ("burn:///");

	GError *error = NULL;

	g_file_copy (source,
				 destination,
				 G_FILE_COPY_OVERWRITE,
				 NULL,
				 NULL,
				 NULL,
				 &error);

	utils_check_gerror (&error);
}
*/
/******************************************************************************/
static void
context_menu_add_new_dir_callback (const gchar *file_name) {
	GError *error = NULL;

	GladeXML* xml = glade_xml_new (GLADEUI_PATH, "new_dir_dialog", NULL);
	g_return_if_fail (xml != NULL);

	GtkWidget *new_dir_dialog = glade_xml_get_widget (xml, "new_dir_dialog");
	GtkWidget *new_dir_entry  = glade_xml_get_widget (xml, "new_dir_entry");

	if (gtk_dialog_run (GTK_DIALOG (new_dir_dialog)) == GTK_RESPONSE_ACCEPT) {
		const gchar *entry_text = gtk_entry_get_text (GTK_ENTRY (new_dir_entry));
		gchar *new_dir = g_strdup_printf ("%s/%s", file_name, entry_text);

		GFile *file = g_file_new_for_path (new_dir);
		g_file_make_directory (file, NULL, &error);
		g_object_unref (file);

		/* open the dir if we succeeded in creating it */
		if (!utils_check_gerror (&error)) {
			vfs_file_do_default_action (new_dir);
		}
	}
	gtk_widget_destroy (new_dir_dialog);
}
/******************************************************************************/
static void
context_menu_add_new_dir (const gchar *file_name, GtkWidget *menu) {

	if (!vfs_file_is_directory (file_name)) return;

	GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic ("_New Folder Here");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   gtk_image_new_from_stock (GTK_STOCK_NEW,
								   							 GTK_ICON_SIZE_MENU));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	g_signal_connect_swapped (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (context_menu_add_new_dir_callback),
							  (gpointer) file_name);	
}
/******************************************************************************/
static void
context_menu_add_burn (const gchar *file_name, GtkWidget *menu) {
	GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic ("_Create CD/DVD");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   gtk_image_new_from_icon_name ("nautilus-cd-burner",
								   								 GTK_ICON_SIZE_MENU));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	LaunchInfo *launch_info = g_new0 (LaunchInfo, 1);
	launch_info->command = g_strdup ("brasero");
	launch_info->file = g_strdup (file_name);

	g_signal_connect_swapped (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (vfs_launch_application),
							  (gpointer) launch_info);

	garbage_add_item (garbage, launch_info);
	garbage_add_item (garbage, launch_info->command);
	garbage_add_item (garbage, launch_info->file);
}
/******************************************************************************/
static void
context_menu_add_compile_tex (const gchar *file_name, GtkWidget *menu) {

	if (!g_str_has_suffix (file_name, "tex")) return;

	GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic ("_Build Latex Document");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   gtk_image_new_from_icon_name ("build",
								   								 GTK_ICON_SIZE_MENU));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	
	LaunchInfo *launch_info = g_new0 (LaunchInfo, 1);
	launch_info->command = g_strdup ("rubber -f --inplace -d");
	launch_info->file = g_strdup (file_name);

	g_signal_connect_swapped (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (vfs_launch_application),
							  (gpointer) launch_info);

	garbage_add_item (garbage, launch_info);
	garbage_add_item (garbage, launch_info->command);
	garbage_add_item (garbage, launch_info->file);
}
/******************************************************************************/
static gboolean
is_archive (const gchar *file_name) {
	gboolean ret = FALSE;
	GFile*	   file = g_file_new_for_path (file_name);
	GFileInfo* file_info =  g_file_query_info (file,
											   G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
											   0,
											   NULL,											   
											   NULL);

	const gchar* content_type = g_file_info_get_content_type (file_info);

	int i;
	for (i = 0; archive_mime_types[i] != NULL; i++) {
		if (g_strcmp0 (content_type, archive_mime_types[i]) == 0) {
			ret = TRUE;
			break;
		}
	}
	g_object_unref (file_info);
	g_object_unref (file);

	return ret;
}
/******************************************************************************/
static void
context_menu_add_archive_action (const gchar *file_name, GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar *archive_label = NULL;
	gchar *archive_action = NULL;

	if (is_archive (file_name)) {
		archive_label = "_Extract Here";
		archive_action = "file-roller -h";
	}
	else {
		archive_label = "Create _Archive";
		archive_action = "file-roller -d";
	}

	GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (archive_label);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   gtk_image_new_from_icon_name ("package",
								   								 GTK_ICON_SIZE_MENU));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	
	LaunchInfo *launch_info = g_new0 (LaunchInfo, 1);
	launch_info->command = g_strdup (archive_action);
	launch_info->file = g_strdup (file_name);

	g_signal_connect_swapped (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (vfs_launch_application),
							  (gpointer) launch_info);

	garbage_add_item (garbage, launch_info);
	garbage_add_item (garbage, launch_info->command);
	garbage_add_item (garbage, launch_info->file);
}
/******************************************************************************/
static void
context_menu_add_open_with_item (const gchar *file_name, GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GList *root = vfs_get_all_mime_applications (file_name);
	GList *apps = root;
	
	if (root == NULL) return;

	GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic ("_Open With");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   gtk_image_new_from_stock (GTK_STOCK_OPEN,
								   							 GTK_ICON_SIZE_MENU));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	GtkWidget *sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
							   sub_menu);

	while (apps != NULL) {
		menu_item = gtk_image_menu_item_new_with_label (g_app_info_get_name (apps->data));
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
									   vfs_get_icon_for_app_info (apps->data));
/* or
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
									   gtk_image_new_from_gicon (g_app_info_get_icon (apps->data),
									   							 GTK_ICON_SIZE_MENU));
*/

		LaunchInfo *launch_info = g_new0 (LaunchInfo, 1);
		launch_info->command = g_strdup (g_app_info_get_executable (apps->data));
		launch_info->file = g_strdup (file_name);

		g_signal_connect_swapped (GTK_MENU_ITEM (menu_item),
								  "activate",
								  G_CALLBACK (vfs_launch_application),
								  (gpointer) launch_info);

		gtk_menu_shell_append (GTK_MENU_SHELL (sub_menu), menu_item);

		garbage_add_item (garbage, launch_info);
		garbage_add_item (garbage, launch_info->command);
		garbage_add_item (garbage, launch_info->file);

		g_object_unref (apps->data);
		apps = apps->next;
	}
	g_list_free (root);
}
/******************************************************************************/
static void
context_menu_add_trash_item (const gchar *file_name, GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);
						  
	GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic ("_Move to Trash");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
														gtk_image_new_from_stock (GTK_STOCK_DELETE,
																				  GTK_ICON_SIZE_MENU));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	
	g_signal_connect_swapped (G_OBJECT (menu_item),
							  "activate",
							  G_CALLBACK (vfs_trash_file),
							  (gpointer) file_name);
}
/******************************************************************************/
static void
context_menu_add_close_item (GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
						   gtk_separator_menu_item_new());

	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
						   gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, NULL));
}
/******************************************************************************/

static void
context_menu_populate (const gchar *file_name, GtkWidget *menu) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	context_menu_add_open_with_item	(file_name, menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());
	context_menu_add_new_dir		(file_name, menu);
	context_menu_add_trash_item		(file_name, menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());
	context_menu_add_archive_action	(file_name, menu);
	context_menu_add_compile_tex	(file_name, menu);
	context_menu_add_burn			(file_name, menu);
	context_menu_add_close_item		(menu);
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
	garbage_empty (&garbage, FALSE);
}
/******************************************************************************/
gboolean
context_menu_display (const gchar *file_name, GtkWidget *menu_item) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	int event_button;
	int event_time;

	GdkEventButton *event = g_object_get_data (G_OBJECT (menu_item), "button_event");
	if (event) {
		event_button = event->button;
		event_time = event->time;
	}
	else {
		event_button = 3;
		event_time = gtk_get_current_event_time();
	}
	garbage_init (&garbage);


	GtkWidget *menu = gtk_menu_new ();

	GtkWidget *browser = g_object_get_data (G_OBJECT (menu_item), "menu_browser");
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

/*
	GtkWidget *current = gtk_widget_get_parent (GTK_WIDGET (menu_item));
	while (current && !PANEL_IS_APPLET (current)) {
		gtk_widget_set_sensitive (current, FALSE);
		current = gtk_widget_get_parent (GTK_WIDGET (current));
	}
*/

/*
	GtkWidget *browser = g_object_get_data (G_OBJECT (menu_item), "menu_browser");
GtkWidget *panel_menu_bar = gtk_widget_get_parent (GTK_WIDGET (browser));
GtkWidget *parent_menu = gtk_widget_get_parent (GTK_WIDGET (menu_item));
GtkMenuShell *panel_menu_bar_shell = GTK_MENU_SHELL (panel_menu_bar);
GtkWidget *applet = gtk_widget_get_parent (GTK_WIDGET (panel_menu_bar));
GtkWidget *panel = gtk_widget_get_parent (GTK_WIDGET (applet));
*/

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
