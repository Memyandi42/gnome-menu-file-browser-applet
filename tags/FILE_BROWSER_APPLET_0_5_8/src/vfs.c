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
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <glib/gprintf.h>

GtkIconTheme *icon_theme = NULL;

/******************************************************************************/
gboolean
vfs_file_is_executable (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GFileInfo* file_info =  g_file_query_info (g_file_new_for_path (file_name),
											   G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE,
											   0,
											   NULL,
											   NULL);

	return (g_file_info_get_attribute_boolean (file_info,
											  G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE)) &&
				!vfs_file_is_directory (file_name); 
}
/******************************************************************************/
gboolean
vfs_file_is_desktop (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	return !(g_desktop_app_info_new_from_filename (file_name) == NULL);
}
/******************************************************************************/
gboolean
vfs_file_is_directory (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);;

	GFileInfo* file_info =  g_file_query_info (g_file_new_for_path (file_name),
											   "standard::*",
											   0,
											   NULL,
											   NULL);

	return (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY);
}
/******************************************************************************/
/* Gets the executable file name of the application associated  with the passed
 * file*/
gchar *
vfs_get_mime_application (const gchar *file_name_and_path) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GFileInfo* file_info =  g_file_query_info (g_file_new_for_path (file_name_and_path),
											   "standard::*",
											   0,
											   NULL,
											   NULL);

	return (gchar *)g_app_info_get_executable (
					g_app_info_get_default_for_type (
					g_file_info_get_content_type (file_info), FALSE));
}
/******************************************************************************/
gboolean
vfs_file_exists (const gchar *file_name) {
	return g_file_test (file_name, G_FILE_TEST_EXISTS);
}
/******************************************************************************/
/* Gets the contents of a directory. Puts the files and directories in separate
 * arrays, and sorts them both. */
gchar *
vfs_get_dir_listings (GPtrArray *files,
					  GPtrArray *dirs,
					  gboolean show_hidden,
					  gchar *path) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GError *error = NULL;
	GFile *file = NULL;
	GFileInfo *file_info = NULL;
	GFileEnumerator *enummerator = NULL;

	file = g_file_new_for_path (path);
	enummerator = g_file_enumerate_children (file,
											 "standard::*",
											 0,
											 NULL,
											 &error);
	/* did we read the dir correctly? */
	if (error) {
		gchar *error_msg = g_strdup (error->message);
		g_error_free (error);
		error = NULL;
		return error_msg;
	}

	while ((file_info = g_file_enumerator_next_file (enummerator, NULL, &error)) != NULL) {
		/* skip the file if it's hidden and we aren't showing hidden files */
		if (g_file_info_get_is_hidden (file_info) && !show_hidden) {
			continue;
		}

		/* get eh file's human readable name */
		gchar *display_name = g_strdup (g_file_info_get_display_name (file_info));

		/* add it to the array */
		if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY) {
			g_ptr_array_add (dirs, (gpointer)display_name);
		}
		else {
			g_ptr_array_add (files, (gpointer)display_name);
		}
	}

	/* always check for errors */
	if (error) {
		gchar *error_msg = g_strdup (error->message);
		g_error_free (error);
		error = NULL;
		return error_msg;
	}

	g_ptr_array_sort (dirs, (GCompareFunc)&utils_sort_alpha);
	g_ptr_array_sort (files, (GCompareFunc)&utils_sort_alpha);

	return NULL;
}
/******************************************************************************/
gboolean
vfs_launch_desktop_file (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	if (!vfs_file_is_desktop (file_name)) {
		return FALSE;
	}

	GDesktopAppInfo *info = NULL;
	if ((info = g_desktop_app_info_new_from_filename (file_name)) != NULL) {
		return g_app_info_launch (G_APP_INFO (info),
								  NULL,
								  NULL,
								  NULL);
		}
	return FALSE;
}
/******************************************************************************/
void
vfs_launch_app (gchar **args, const gchar *working_dir) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GError *error = NULL;
	gint child_pid;
	gboolean ret;

	ret = g_spawn_async_with_pipes (working_dir,
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

	gchar **args = NULL;
	gchar *arg = NULL;
	gchar *working_dir = NULL;
	int i;

	working_dir = g_path_get_dirname (file_name_and_path);
	arg = g_strdelimit (editor_bin, " ", '\1');
	arg = g_strconcat (arg, "\1", file_name_and_path, NULL);
	args = g_strsplit (arg, "\1", 0);
	vfs_launch_app (args, working_dir);

	g_free (arg);
	for (i = 0; args[i]; i++) {
		g_free (args[i]);
	}
	g_free (args);
	g_free (working_dir);
}
/******************************************************************************/
void
vfs_launch_terminal (const gchar *path, gchar *terminal_bin) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar **args = NULL;

	args = g_strsplit (terminal_bin, " ", 0);

	vfs_launch_app (args, path);
	g_free (args);

}
/******************************************************************************/
void
vfs_open_file (const gchar *file_name_and_path, gint exec_action) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar **args = NULL;
	gchar *arg = NULL;
	gchar *file_mime_app_exec = NULL;
	gchar *working_dir = NULL;
	gboolean is_executable;
	int i;

	working_dir = g_path_get_dirname (file_name_and_path);
	is_executable = vfs_file_is_executable (file_name_and_path);

	file_mime_app_exec = vfs_get_mime_application (file_name_and_path);

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

	g_free (arg);
	for (i = 0; args[i]; i++) {
		g_free (args[i]);
	}
	g_free (args);
	g_free (file_mime_app_exec);
}
/******************************************************************************/
void
vfs_trash_file (gchar *file_name) {
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
 * files will generally be handles by the G_IS_FILE_ICON section. Theme images
 * are handles by the G_IS_THEMED_ICON section where we first do some shit I
 * don't understand and if that fails, we ask gtk_icon_theme_choose_icon to
 * figure out wat icon to use. */
static GdkPixbuf *
vfs_get_pixbuf_for_icon (GIcon *icon) {
	GdkPixbuf  *pixbuf = NULL;
	gchar *filename;

	if (G_IS_THEMED_ICON (icon)) {
		const gchar * const *names = g_themed_icon_get_names (G_THEMED_ICON (icon));
		GtkIconInfo *icon_info = gtk_icon_theme_choose_icon (icon_theme,
															 (const gchar **)names,
															 ICON_MENU_SIZE,
															 0);
		pixbuf = gtk_icon_info_load_icon (icon_info, NULL);

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
		filename = g_file_get_path (g_file_icon_get_file (G_FILE_ICON (icon)));
		if (filename) {
			pixbuf = gdk_pixbuf_new_from_file_at_size (filename,
													   ICON_MENU_SIZE,
													   ICON_MENU_SIZE,
													   NULL);
		}
		g_free (filename);
	}
	return pixbuf;
}
/******************************************************************************/
/* Returns the image associated with a file. Works on both normal and desktop
 * files. */
GtkWidget *
vfs_get_icon_for_file (const gchar *file_name) {
	if (icon_theme == NULL) {
		icon_theme = gtk_icon_theme_get_default();
	}

	GIcon *icon = NULL;
	GdkPixbuf *icon_pixbuf = NULL; 

	/* try for desktop file */
	if (vfs_file_is_desktop (file_name)) {
		GDesktopAppInfo *info = g_desktop_app_info_new_from_filename (file_name);
		icon = g_app_info_get_icon (G_APP_INFO (info));
	}	/* not a desktop file */
	else {
		GFileInfo* file_info = g_file_query_info (g_file_new_for_path (file_name),
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

	return icon_widget; 
}
/******************************************************************************/
const gchar *
vfs_get_desktop_app_name (const gchar *file_name) {
	GDesktopAppInfo *info = g_desktop_app_info_new_from_filename (file_name);
	return g_app_info_get_name (G_APP_INFO (info));
}
/******************************************************************************/
