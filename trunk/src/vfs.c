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

#include <gio/gdesktopappinfo.h>
#include <glib/gprintf.h>
#include <gio/gio.h>

#include "vfs.h"
#include "utils.h"
#include "config.h"

/******************************************************************************/
/* sort the structures based on the file's display_name */
static inline gint
vfs_sort_array (const VfsFileInfo **i1, const VfsFileInfo **i2) {
	return g_utf8_collate ((gchar *)(*i1)->display_name, (gchar *)(*i2)->display_name);
}
/******************************************************************************/
gboolean
vfs_file_exists (const gchar *file_name) {
	return g_file_test (file_name, G_FILE_TEST_EXISTS);
}
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
	app_info ? g_object_unref (app_info) : NULL;
	return ret;
}
/******************************************************************************/
/* Gets the executable file name of the application associated  with the passed
 * file. The caller must free the returned value. */
gchar*
vfs_get_default_mime_application (const gchar *file_name) {
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
 /* The caller  must free the return value. */
static gchar *
vfs_get_desktop_app_name (const gchar *file_name) {
	GDesktopAppInfo *info = g_desktop_app_info_new_from_filename (file_name);
	gchar* ret = g_strdup (g_app_info_get_name (G_APP_INFO (info)));
	g_object_unref (info);
	return ret;
}
/******************************************************************************/
void
vfs_file_trash (gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GError *error = NULL;
	GFile *file = g_file_new_for_path (file_name);

	/* Try moving it to the trash */
	g_file_trash (file, NULL, &error);

	/* Let the user know if we failed. */
	utils_gerror_ok (&error, TRUE);
	g_free (file_name);
}
/******************************************************************************/
GtkWidget *
vfs_get_icon_for_file (const gchar *file_name) {

	GtkWidget *icon_widget = NULL;
	GDesktopAppInfo *app_info = g_desktop_app_info_new_from_filename (file_name);

	if (app_info != NULL) {
		GIcon *icon = g_app_info_get_icon (G_APP_INFO (app_info));
		icon_widget = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_MENU);
		g_object_unref (app_info);
	}
	else {
		GFile *file = g_file_new_for_path (file_name);
		GFileInfo *file_info = g_file_query_info (file,
											  G_FILE_ATTRIBUTE_STANDARD_ICON,
											  0,
											  NULL,
											  NULL);	

		GIcon *icon = g_file_info_get_icon (file_info);
		icon_widget = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_MENU);
		g_object_unref (file);
		g_object_unref (file_info);
	}
	return icon_widget;
}
/******************************************************************************/
GList*
vfs_get_all_mime_applications (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GFile*	   file = g_file_new_for_path (file_name);
	GFileInfo* file_info =  g_file_query_info (file,
											   G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
											   0,
											   NULL,
											   NULL);

	const gchar* content_type = g_file_info_get_content_type (file_info);
	GList *app_infos = g_app_info_get_all_for_type (content_type);
	
	g_object_unref (file_info);
	g_object_unref (file);

	return app_infos;
}
/******************************************************************************/
gboolean
vfs_launch_application (LaunchInfo *launch_info) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gboolean ret = TRUE;
	GError *error = NULL;
	gint child_pid;
	gchar *working_dir = NULL;

	if (launch_info->file) {
		working_dir = (vfs_file_is_directory (launch_info->file)) ? 
					  g_strdup (launch_info->file) :
					  g_path_get_dirname (launch_info->file);
	}
	else {
		working_dir = g_strdup (g_get_home_dir ());
	}

	g_strdelimit (launch_info->command, " ", '\1');

	/* only add the file name as an argument if one was specified */
	gchar *arg = (launch_info->file) ? 
				 g_strconcat (launch_info->command, "\1", launch_info->file, NULL) :
				 g_strdup (launch_info->command);

	gchar** args = g_strsplit (arg, "\1", 0);

	/* need to do this to convert the '\2's back into spaces. */
	g_strdelimit (args[0], "\2", ' ');

	/* remove trailing spaces from all args */
	int i = 0;
	while (args[i]) {
		g_strchomp (args[i]);
		i++;
	}

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

	ret = utils_gerror_ok (&error, TRUE) ? ret : FALSE;
	
	g_free (arg);
	g_strfreev (args);
	g_free (working_dir);

	return ret;
}
/******************************************************************************/
gboolean
vfs_file_do_default_action (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	LaunchInfo *launch_info = g_new0 (LaunchInfo, 1);

	/*Try launching file as desktop file first*/
	if (vfs_file_is_desktop (file_name)) {
		GDesktopAppInfo *app_info = g_desktop_app_info_new_from_filename (file_name);
		launch_info->command = g_strdup (g_app_info_get_executable (G_APP_INFO (app_info)));
		launch_info->file = NULL;
		g_object_unref (app_info);
	}
	/* run it if its an executable */
	else if (vfs_file_is_executable (file_name)) {
		launch_info->command = g_strdup (file_name);
		/* need to do this in case the path or filename has spaces in it since
		 * the spaces are converted to '\1' in vfs_launch_application.
		 * vfs_launch_application will convert the '\2' back to a space. */
		g_strdelimit (launch_info->command, " ", '\2');
		launch_info->file = NULL;
	}
	/* open it with the default mime app */
	else {
		launch_info->command = vfs_get_default_mime_application (file_name);
		launch_info->file =  g_strdup (file_name);
	}

	gboolean ret = FALSE;
	if (launch_info->command) {
		ret = vfs_launch_application (launch_info);
	}
	else {
		gchar *msg = g_strdup_printf (_("Could not display \"%s\".\n"
									  "There is no application installed for this file type."), file_name);
		utils_show_dialog (_("Error"), msg, GTK_MESSAGE_ERROR);
		g_free (msg);
	}

	g_free (launch_info->command);
	g_free (launch_info->file);
	g_free (launch_info);

	return ret;
}
/******************************************************************************/
/* Gets the contents of a directory. Puts the files and directories in separate
 * arrays, and sorts them both. The caller must free the return value. */
gchar*
vfs_get_dir_listings (GPtrArray *files,
					  GPtrArray *dirs,
					  gboolean show_hidden,
					  const gchar *path) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GError *error = NULL;

	GFile *file = g_file_new_for_path (path);
	/* get ALL the info about the files */
	GFileEnumerator *enumerator = g_file_enumerate_children (file,
															 /*"*",*/
															 "standard::type,"
															 "standard::is-hidden,"
															 "standard::name,"
															 "standard::display-name,"
															 "access::can-execute",
															 0,
															 NULL,
															 &error);
	/* did we read the dir correctly? */
	if (error) {
		gchar *error_msg = g_strdup (error->message);
		utils_gerror_ok (&error, FALSE);
		return error_msg;
	}

	GFileInfo *file_info = NULL;
	while ((file_info = g_file_enumerator_next_file (enumerator, NULL, &error)) != NULL) {
		/* skip the file if it's hidden and we aren't showing hidden files */
		if (g_file_info_get_is_hidden (file_info) && !show_hidden) {
			g_object_unref (file_info);
			continue;
		}
		VfsFileInfo *vfs_file_info = g_new0 (VfsFileInfo ,1);

		vfs_file_info->file_name = g_strdup_printf ("%s/%s", path, g_file_info_get_name (file_info));

		/* get the file's human readable name, including if it's a desktop file */

		
		vfs_file_info->display_name = (vfs_file_is_desktop (vfs_file_info->file_name)) ?
									   vfs_get_desktop_app_name (vfs_file_info->file_name) :
									   g_strdup (g_file_info_get_display_name (file_info));

		vfs_file_info->is_executable = g_file_info_get_attribute_boolean (file_info,
																		  G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE) &&
											(g_file_info_get_file_type (file_info) != G_FILE_TYPE_DIRECTORY);
		/* get the icon */
		vfs_file_info->icon = vfs_get_icon_for_file (vfs_file_info->file_name); 

		/* add it to the array */
		if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY) {
			g_ptr_array_add (dirs, (gpointer)vfs_file_info);
		}
		else {
			g_ptr_array_add (files, (gpointer)vfs_file_info);
		}
		g_object_unref (file_info);
	}
	g_object_unref (enumerator);
	g_object_unref (file);

	/* always check for errors */
	if (error) {
		gchar *error_msg = g_strdup (error->message);
		utils_gerror_ok (&error, FALSE);
		return error_msg;
	}

	g_ptr_array_sort (dirs, (GCompareFunc)&vfs_sort_array);
	g_ptr_array_sort (files, (GCompareFunc)&vfs_sort_array);

	return NULL;
}
/******************************************************************************/
