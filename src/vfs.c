/*
 * File:				vfs.c
 * Created:				February 2008
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

#include <glib/gprintf.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <libgnome/gnome-desktop-item.h>

/******************************************************************************/
gboolean
vfs_file_is_executable (const gchar *file_name) {
	gchar *mime_type = gnome_vfs_get_mime_type (file_name);	
	gboolean is_executable = FALSE;
	is_executable =  g_file_test (file_name, G_FILE_TEST_IS_EXECUTABLE) &&
					!g_file_test (file_name, G_FILE_TEST_IS_DIR) &&
					(g_str_has_prefix (mime_type, "application/x-") ||
					 g_str_has_prefix (mime_type, "text/x-"));
	g_free (mime_type);
	return is_executable;
}
/******************************************************************************/
gboolean
vfs_file_is_desktop (const gchar *file_name) {
	return g_str_has_suffix (file_name, ".desktop");
}
/******************************************************************************/
gchar *
vfs_get_mime_application (const gchar *file_name_and_path) {
	GnomeVFSMimeApplication *mime_application = NULL;
	gchar *mime_type = NULL;
	gchar *file_mime_app_exec = NULL;
	
	mime_type		 = gnome_vfs_get_mime_type (file_name_and_path);	
	mime_application = gnome_vfs_mime_get_default_application (mime_type);

	if (mime_application) {
		file_mime_app_exec = g_strdup ((gchar *)mime_application->command);	
	}

	g_free (mime_type);
	g_free (mime_application);

	return file_mime_app_exec;
}
/******************************************************************************/
gboolean
vfs_file_exists (const gchar *file_name) {
	return g_file_test (file_name, G_FILE_TEST_EXISTS);
}
/******************************************************************************/
gchar *
vfs_get_dir_contents (GPtrArray *files,
					  GPtrArray *dirs,
					  gboolean show_hidden,
					  gchar *path) {

	GnomeVFSDirectoryHandle *vfs_dir_handle = NULL;
	GnomeVFSResult	 		vfs_result;
	GnomeVFSFileInfo		*vfs_file_info = NULL;
	gchar					*error = NULL;

	/*make struct for getting file info, open the dir for reading and get the first entry*/
	vfs_file_info = gnome_vfs_file_info_new();
	
	vfs_result = gnome_vfs_directory_open (&vfs_dir_handle,
										   path,
/*										   GNOME_VFS_FILE_INFO_GET_MIME_TYPE |*/
										   GNOME_VFS_FILE_INFO_FOLLOW_LINKS);

	/* make sure the dir was opened OK. This fixes bug #3 */
	if (vfs_result == GNOME_VFS_OK) {
		/* get the first entry */
		vfs_result = gnome_vfs_directory_read_next (vfs_dir_handle,
													vfs_file_info);
		/* if it opened OK and while its not empty, keep reading items */
		while (vfs_result == GNOME_VFS_OK) {	
			/*if it's not a hidden file or were showing hidden files...*/
			if ((g_ascii_strncasecmp (vfs_file_info->name, ".", 1) != 0 || show_hidden) &&
				 g_ascii_strcasecmp (vfs_file_info->name, ".") != 0 &&
				 g_ascii_strcasecmp (vfs_file_info->name, "..") != 0) {

				if (vfs_file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
					/*make an array that holds all the dirs in this dir*/
					g_ptr_array_add (dirs, (gpointer)g_strdup (vfs_file_info->name));
				}
				if (vfs_file_info->type == GNOME_VFS_FILE_TYPE_REGULAR) {
					/*make an array that holds all the files in this dir*/
					g_ptr_array_add (files, (gpointer)g_strdup (vfs_file_info->name));
				}
			}
		/*get the next entry*/
			vfs_result = gnome_vfs_directory_read_next (vfs_dir_handle,
														vfs_file_info);
		}
		/*close the dir*/
		vfs_result = gnome_vfs_directory_close (vfs_dir_handle);
	}
	else {
		error = g_strdup_printf ("(%s)",gnome_vfs_result_to_string (vfs_result));
		if (DEBUG) g_printf ("Error opening directory. GNOME_VFS error: %s\n",
							 error);
	}
	/**************************** Finished reading dir contents ************************/
	
	/*sort the arrays containing the directory and file listings*/
	g_ptr_array_sort (dirs, (GCompareFunc)&utils_sort_alpha);
	g_ptr_array_sort (files, (GCompareFunc)&utils_sort_alpha);

	gnome_vfs_file_info_clear (vfs_file_info);
	g_free (vfs_file_info);
	return error;
}
/******************************************************************************/
gboolean
vfs_launch_desktop_file (const gchar *file_name) {
	if (!vfs_file_is_desktop (file_name)) {
		return FALSE;
	}
	GError *error = NULL;
	GnomeDesktopItem *ditem = gnome_desktop_item_new_from_file (file_name, 0, &error);

	gnome_desktop_item_launch  (ditem,
								NULL,
								GNOME_DESKTOP_ITEM_LAUNCH_ONLY_ONE,
								&error);
	utils_check_gerror (&error);
	gnome_desktop_item_unref (ditem);
	return TRUE;
}
/******************************************************************************/
void
vfs_launch_app (gchar **args, const gchar *working_dir) {
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

	gchar **args = NULL;
		
	args = g_strsplit (terminal_bin, " ", 0);
	
	vfs_launch_app (args, path);
	g_free (args);	

}
/******************************************************************************/
void
vfs_open_file (const gchar *file_name_and_path, gint exec_action) {
	gchar **args = NULL;
	gchar *arg = NULL;
	gchar *file_mime_app_exec = NULL;
	gchar *working_dir = NULL;
	gboolean is_executable;
	int i;

	working_dir = g_path_get_dirname (file_name_and_path);
	is_executable = vfs_file_is_executable (file_name_and_path);

	/* FIXME: sigh!!! "#" makes gnome_vfs_get_mime_type crash */
	if (!g_strrstr (file_name_and_path, "#")) {
		file_mime_app_exec = vfs_get_mime_application (file_name_and_path);
	}
	else {
		utils_show_dialog ("Error: gnome-vfs bug",
						   "Some gnome-vfs functions cannot handle fine names that include the \"#\" character.",
						   GTK_MESSAGE_ERROR);
		return;
	}
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

	return;
}
/******************************************************************************/
