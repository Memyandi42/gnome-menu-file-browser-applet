/*
 * File:				vfs.c
 * Created:				August 2008
 * Created by:			Axel von Bertoldi
 * Last Modified:		March 2008
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

#include "vfs.h"
#include "utils.h"

#include <string.h>
#include <gio/gdesktopappinfo.h>
#include <glib/gprintf.h>

GtkIconTheme *icon_theme = NULL;

/******************************************************************************/
gboolean
vfs_file_is_executable (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GFile*	   file = g_file_new_for_path (file_name);
	GFileInfo* file_info =  g_file_query_info (file,
											   G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE,
											   0,
											   NULL,
											   NULL);

	gboolean ret = g_file_info_get_attribute_boolean (file_info,
													  G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE) &&
											  		  !vfs_file_is_directory (file_name);
	g_object_unref (file_info);
	g_object_unref (file);
	return ret;	
}
/******************************************************************************/
gboolean
vfs_file_is_directory (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GFile*	   file = g_file_new_for_path (file_name);
	GFileInfo* file_info =  g_file_query_info (file,
											   G_FILE_ATTRIBUTE_STANDARD_TYPE,
											   0,
											   NULL,
											   NULL);

	gboolean ret = (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY);
	g_object_unref (file_info);
	g_object_unref (file);
	return ret;
}
/******************************************************************************/
gboolean
vfs_file_is_desktop (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GDesktopAppInfo *app_info = g_desktop_app_info_new_from_filename (file_name);
	gboolean ret = !(app_info == NULL);
	g_object_unref (app_info);
	return ret;
}
/******************************************************************************/
/* Gets the executable file name of the application associated  with the passed
 * file. The caller must free the returned value. */
gchar*
vfs_get_mime_application (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GFile*	   file = g_file_new_for_path (file_name);
	GFileInfo* file_info =  g_file_query_info (file,
											   G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
											   0,
											   NULL,
											   NULL);

	const gchar* content_type = g_file_info_get_content_type (file_info);
	GAppInfo *app_info = g_app_info_get_default_for_type (content_type, FALSE);
	gchar* exec = g_strdup (g_app_info_get_executable (app_info));

	g_object_unref (file_info);
	g_object_unref (file);
	g_object_unref (app_info);

	return exec;
}
/******************************************************************************/
gboolean
vfs_file_exists (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);
	return g_file_test (file_name, G_FILE_TEST_EXISTS);
}
/******************************************************************************/
void
vfs_get_dir_listings_async_cancel (GCancellable *cancellable) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

g_printf ("In %s START	\n", __FUNCTION__);
	if (G_IS_CANCELLABLE (cancellable)) {
		g_cancellable_cancel (cancellable);
		/*g_object_unref (cancellable);*/
	}	
g_printf ("In %s END\n", __FUNCTION__);
}
/******************************************************************************/
inline gint
vfs_sort_alpha (const VfsFileInfo **i1, const VfsFileInfo **i2) {
	return g_utf8_collate ((gchar *)(*i1)->display_name, (gchar *)(*i2)->display_name);
}
/******************************************************************************/
static void
vfs_enumerate_children_callback (GObject		*source_object,
								 GAsyncResult	*result,
								 gpointer		data) {
 	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

g_printf ("In %s START\n", __FUNCTION__);

	EnumerateData	*enum_data = (EnumerateData *)data;
	GError			*error = NULL;
	GFileInfo		*file_info = NULL;
	GPtrArray		*files_array = g_ptr_array_new();
	GPtrArray		*dirs_array = g_ptr_array_new();
	gchar			*error_msg = NULL;

	if (g_cancellable_is_cancelled (enum_data->cancellable)) {
		g_printf ("operation cancelled. bailing!!!!!!!!!!!!!!!!\n");
		g_object_unref (enum_data->cancellable);
	 	return;
	}

	GFileEnumerator	*enumerator = g_file_enumerate_children_finish (G_FILE (source_object),
																	result,
																	&error);

	if (enumerator != NULL) {
		while ((file_info = g_file_enumerator_next_file (enumerator, NULL, &error)) != NULL) {

			/* bail if the operation was cancelled */
			if (g_cancellable_is_cancelled (enum_data->cancellable)) {
				g_printf ("operation cancelled. bailing 2 !!!!!!!!!!!!!!!!\n");
				break;
			}	

			/* skip the file if it's hidden and we aren't showing hidden files */
			if (g_file_info_get_is_hidden (file_info) && !enum_data->show_hidden) {
				continue;
			}
			
			VfsFileInfo *vfs_file_info = g_new0 (VfsFileInfo ,1);

			vfs_file_info->file_name = g_strdup_printf ("%s/%s", enum_data->path, g_file_info_get_name (file_info));
			vfs_file_info->is_desktop = vfs_file_is_desktop (vfs_file_info->file_name);

			/* get the file's human readable name, including if it's a desktop file */
			if (vfs_file_info->is_desktop) {
				vfs_file_info->display_name = vfs_get_desktop_app_name (vfs_file_info->file_name);
			}
			else {
				vfs_file_info->display_name = g_strdup (g_file_info_get_display_name (file_info));
			}
			vfs_file_info->is_executable = g_file_info_get_attribute_boolean (file_info,
																			  G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE) &&
												(g_file_info_get_file_type (file_info) != G_FILE_TYPE_DIRECTORY);
			/* get the icon */
			vfs_file_info->icon = vfs_get_icon_for_file (vfs_file_info->file_name); 

			/* add it to the array */
			if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY) {
				g_ptr_array_add (dirs_array, (gpointer)vfs_file_info);
			}
			else {
				g_ptr_array_add (files_array, (gpointer)vfs_file_info);
			}
		}
	}

	/* always check for errors */
	if (error) {
		error_msg = g_strdup (error->message);
		g_error_free (error);
		error = NULL;
	}
	
	g_ptr_array_sort (dirs_array, (GCompareFunc)&vfs_sort_alpha);
	g_ptr_array_sort (files_array, (GCompareFunc)&vfs_sort_alpha);

	menu_browser_populate_menu_async_callback (enum_data->menu_browser,
											   enum_data->path,
											   enum_data->menu,
											   dirs_array,
											   files_array,
											   error_msg);

	g_object_unref (enumerator);
	g_object_unref (enum_data->cancellable);
	g_object_unref (source_object);
	g_free (enum_data);
g_printf ("In %s END\n", __FUNCTION__);
}
/******************************************************************************/
void
vfs_get_dir_listings_async (gchar *path,
							GtkWidget *menu,
							gboolean show_hidden,
							MenuBrowser *menu_browser,
							GCancellable *cancellable) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (menu_browser));

	EnumerateData *enum_data = g_new0 (EnumerateData, 1);
	enum_data->path = path;
	enum_data->menu = menu;
	enum_data->show_hidden = show_hidden;
	enum_data->cancellable = cancellable;
	enum_data->menu_browser = menu_browser;

	GFile *file = g_file_new_for_path (path);

	g_file_enumerate_children_async (file,
									 "*",
									 0,
									 G_PRIORITY_DEFAULT,
									 cancellable,
									 vfs_enumerate_children_callback,
									 (gpointer)enum_data);
	g_object_unref (file);
}
/******************************************************************************/
void
vfs_launch_desktop_file (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GDesktopAppInfo *app_info = g_desktop_app_info_new_from_filename (file_name);

	if (app_info == NULL) return;

	g_app_info_launch (G_APP_INFO (app_info),
					   NULL,
					   NULL,
					   NULL);

	g_object_unref (app_info);
}
/******************************************************************************/
void
vfs_launch_app (char **args, const gchar *working_dir) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GError *error = NULL;
	gint child_pid;

	g_spawn_async_with_pipes (working_dir,
							  args,
							  NULL,
							  G_SPAWN_SEARCH_PATH,
							  NULL,
							  NULL,
							  &child_pid,
							  NULL,
							  NULL,
							  NULL,
							  &error);

	if (utils_check_gerror (&error)) {
		gchar *tmp = g_strdup_printf ("Error: Failed to launch \"%s\".", args[0]);
		utils_show_dialog ("Error: Failed to launch application",
						   tmp,
						   GTK_MESSAGE_ERROR);
		g_free (tmp);
	}
	return;
}
/******************************************************************************/
void
vfs_edit_file (const gchar *file_name_and_path, gchar *editor_bin) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);


	gchar* working_dir = g_path_get_dirname (file_name_and_path);
	gchar* arg = g_strdelimit (editor_bin, " ", '\1');
	arg = g_strconcat (arg, "\1", file_name_and_path, NULL);
	gchar** args = g_strsplit (arg, "\1", 0);
	vfs_launch_app (args, working_dir);

	int i;
	for (i = 0; args[i]; i++) {
		g_free (args[i]);
	}
	g_free (arg);
	g_free (args);
	g_free (working_dir);
}
/******************************************************************************/
void
vfs_launch_terminal (const gchar *path, const gchar *terminal_bin) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar **args = g_strsplit (terminal_bin, " ", 0);
	vfs_launch_app (args, path);
	
	int i;
	for (i = 0; args[i]; i++) {
		g_free (args[i]);
	}
	g_free (args);
}
/******************************************************************************/
void
vfs_open_file (const gchar *file_name_and_path, gint exec_action) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar **args = NULL;
	gchar *arg = NULL;

	gchar* working_dir = g_path_get_dirname (file_name_and_path);
	gboolean is_executable = vfs_file_is_executable (file_name_and_path);

	gchar* file_mime_app_exec = vfs_get_mime_application (file_name_and_path);

	/* if it's a binary file run it*/
	if (is_executable) {
		arg = g_strdup_printf ("%s", file_name_and_path);
		args = g_strsplit (arg, "\1", 0);
	}
	else {
		if (file_mime_app_exec) {
			arg = g_strdelimit (file_mime_app_exec, " ", '\1');
			arg = g_strconcat (arg, "\1", file_name_and_path, NULL);
			args = g_strsplit (arg, "\1", 0);
			if (DEBUG) g_printf ("%s ", file_mime_app_exec);
		}
		else {
			if (DEBUG) g_printf ("Error: failed to get mime application for %s\n", file_name_and_path);
			gchar *message = g_strdup_printf ("Cannot open %s:\n"
											  "No application is known for this kind of file.",
											  file_name_and_path);

			utils_show_dialog ("Error: no application found",
										   message,
										   GTK_MESSAGE_ERROR);
			g_free (message);
			return;
		}
	}
	if (DEBUG) g_printf ("%s\n", file_name_and_path);
	vfs_launch_app (args, working_dir);

	int i;
	for (i = 0; args[i]; i++) {
		g_free (args[i]);
	}
	g_free (arg);
	g_free (args);
	g_free (working_dir);
	g_free (file_mime_app_exec);
}
/******************************************************************************/
void
vfs_trash_file (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GError *error = NULL;
	GFile *file = g_file_new_for_path (file_name);

	/* Try moving it to the trash */
	g_file_trash (file,
				  NULL,
				  &error);

	/* Let the user know if we failed. */
	if (utils_check_gerror (&error)) {
		gchar *message = g_strdup_printf ("Error: Failed to move \"%s\" to Trash.",
										  file_name);
		utils_show_dialog ("Error: Failed to move tile to Trash",
						   message,
						   GTK_MESSAGE_ERROR);
		g_free (message);
	}
}
/******************************************************************************/
/******************************************************************************/
/* Blatantly stolen (and modified) from nautilus-mime-application-chooser.c.
 * Returns a pixmap of the image associated with the passed file. .desktop
 * files will generally be handled by the G_IS_FILE_ICON section. Theme images
 * are handles by the G_IS_THEMED_ICON section where we first do some shit I
 * don't understand and if that fails, we ask gtk_icon_theme_choose_icon to
 * figure out what icon to use.
 * The caller must free the returned value. */
static GdkPixbuf *
vfs_get_pixbuf_for_icon (const GIcon *icon) {
	GdkPixbuf  *pixbuf = NULL;

	if (G_IS_THEMED_ICON (icon)) {
		const gchar * const *names = g_themed_icon_get_names (G_THEMED_ICON (icon));
		GtkIconInfo *icon_info = gtk_icon_theme_choose_icon (icon_theme,
															 (const gchar **)names,
															 ICON_MENU_SIZE,
															 0);
		pixbuf = gtk_icon_info_load_icon (icon_info, NULL);
		/* the line below causes a crash */
		/*g_object_unref (icon_info);*/

		if (pixbuf == NULL) {
			if (names != NULL && names[0] != NULL) {
				gchar *icon_no_extension = g_strdup (names[0]);
				gchar *p = strrchr (icon_no_extension, '.');
				if (p &&
					(strcmp (p, ".png") == 0 ||
					 strcmp (p, ".xpm") == 0 ||
					 strcmp (p, ".svg") == 0)) {
					*p = 0;
				}
				pixbuf = gtk_icon_theme_load_icon (icon_theme,
												   icon_no_extension,
												   ICON_MENU_SIZE,
												   0,
												   NULL);
				g_free (icon_no_extension);
			}
		}
	}
	else if (G_IS_FILE_ICON (icon)) {
		GFile *file = g_file_icon_get_file (G_FILE_ICON (icon));
		gchar *file_name = g_file_get_path (file);
		if (file_name) {
			pixbuf = gdk_pixbuf_new_from_file_at_size (file_name,
													   ICON_MENU_SIZE,
													   ICON_MENU_SIZE,
													   NULL);
		}
		g_free (file_name);
		g_object_unref (file);
	}
	return pixbuf;
}
/******************************************************************************/
/* Returns the image associated with a file. Works on both normal and desktop
 * files. The caller  must free the return value. */
GtkWidget *
vfs_get_icon_for_file (const gchar *file_name) {
	if (icon_theme == NULL) {
		icon_theme = gtk_icon_theme_get_default();
	}

	GIcon *icon = NULL;
	GdkPixbuf *icon_pixbuf = NULL; 
	GFile* file = NULL;
	GFileInfo* file_info = NULL;

	/* try for desktop file */
	if (vfs_file_is_desktop (file_name)) {
		GDesktopAppInfo *info = g_desktop_app_info_new_from_filename (file_name);
		icon = g_app_info_get_icon (G_APP_INFO (info));
	}	/* not a desktop file */
	else {
		file = g_file_new_for_path (file_name),
		file_info = g_file_query_info (file,
									   G_FILE_ATTRIBUTE_STANDARD_ICON,
									   0,
									   NULL,
									   NULL);	
		icon = g_file_info_get_icon (file_info);
	}

	if (icon != NULL) {
		icon_pixbuf = vfs_get_pixbuf_for_icon (icon);
	}

	GtkWidget *icon_widget = gtk_image_new_from_pixbuf (icon_pixbuf);
	g_object_unref (icon_pixbuf);
	g_object_unref (icon);
	g_object_unref (file_info);

	g_object_unref (file);
	return icon_widget; 
}
/******************************************************************************/
 /* The caller  must free the return value. */
gchar *
vfs_get_desktop_app_name (const gchar *file_name) {
	GDesktopAppInfo *info = g_desktop_app_info_new_from_filename (file_name);
	gchar* ret = g_strdup (g_app_info_get_name (G_APP_INFO (info)));
	g_object_unref (info);
	return ret;
}
/******************************************************************************/
