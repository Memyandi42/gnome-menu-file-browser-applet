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
vfs_open_with_default_handler (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	/*Try launching file as desktop file first*/
	if (vfs_launch_desktop_file (file_name)) return TRUE;

	if (vfs_file_is_executable (file_name)) {
		return vfs_launch_application (file_name, NULL);
	}

	GError	*error = NULL;
	GFile	*file = g_file_new_for_path (file_name);
	gchar	*uri = g_file_get_uri (file);

	gboolean ret = g_app_info_launch_default_for_uri (uri, NULL, &error);

	if (utils_check_gerror (&error)) {
		gchar *msg = g_strdup_printf ("Error: Failed to launch \"%s\".",
									  file_name);
									  /*g_app_info_get_executable (G_APP_INFO (app_info)));*/
		utils_show_dialog ("Error: Failed to launch application",
						   msg,
						   GTK_MESSAGE_ERROR);
		g_free (msg);
	}
	g_free (uri);
	g_object_unref (file);
	return ret;
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
	g_object_unref (app_info);
	return ret;
}
/******************************************************************************/
gboolean
vfs_file_exists (const gchar *file_name) {
	return g_file_test (file_name, G_FILE_TEST_EXISTS);
}
/******************************************************************************/
static inline gint
vfs_sort_alpha (const VfsFileInfo **i1, const VfsFileInfo **i2) {
	return g_utf8_collate ((gchar *)(*i1)->display_name, (gchar *)(*i2)->display_name);
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
/* Gets the contents of a directory. Puts the files and directories in separate
 * arrays, and sorts them both. The caller must free the return value. */
gchar*
vfs_get_dir_listings (GPtrArray *files,
					  GPtrArray *dirs,
					  gboolean show_hidden,
					  const gchar *path) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GError *error = NULL;
	GFile *file = NULL;
	GFileInfo *file_info = NULL;
	GFileEnumerator *enumerator = NULL;

	file = g_file_new_for_path (path);
	/* get ALL the info about the files */
	enumerator = g_file_enumerate_children (file,
											 "*",
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

	while ((file_info = g_file_enumerator_next_file (enumerator, NULL, &error)) != NULL) {
		/* skip the file if it's hidden and we aren't showing hidden files */
		if (g_file_info_get_is_hidden (file_info) && !show_hidden) {
			continue;
		}
		VfsFileInfo *vfs_file_info = g_new0 (VfsFileInfo ,1);

		vfs_file_info->file_name = g_strdup_printf ("%s/%s", path, g_file_info_get_name (file_info));

		/* get the file's human readable name, including if it's a desktop file */
		if (vfs_file_is_desktop (vfs_file_info->file_name)) {
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
			g_ptr_array_add (dirs, (gpointer)vfs_file_info);
		}
		else {
			g_ptr_array_add (files, (gpointer)vfs_file_info);
		}
	}
	g_object_unref (enumerator);

	/* always check for errors */
	if (error) {
		gchar *error_msg = g_strdup (error->message);
		g_error_free (error);
		error = NULL;
		return error_msg;
	}

	g_ptr_array_sort (dirs, (GCompareFunc)&vfs_sort_alpha);
	g_ptr_array_sort (files, (GCompareFunc)&vfs_sort_alpha);

	return NULL;
}
/******************************************************************************/
gboolean
vfs_launch_desktop_file (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GError	*error = NULL;
	GDesktopAppInfo *app_info = g_desktop_app_info_new_from_filename (file_name);

	if (app_info == NULL) return FALSE;

	g_app_info_launch (G_APP_INFO (app_info),
					   NULL,
					   NULL,
					   &error);

	if (utils_check_gerror (&error)) {
		gchar *msg = g_strdup_printf ("Error: Failed to launch \"%s\".",
									  g_app_info_get_executable (G_APP_INFO (app_info)));
		utils_show_dialog ("Error: Failed to launch application",
						   msg,
						   GTK_MESSAGE_ERROR);
		g_free (msg);
	}
	g_object_unref (app_info);

	return TRUE;
}
/******************************************************************************/
static gboolean
_vfs_launch_application (const GAppInfo *app_info, GList *list) {
	gboolean ret = TRUE;
	GError *error = NULL;

	g_app_info_launch (G_APP_INFO (app_info),
					   list,
					   NULL,
					   &error);

	if (utils_check_gerror (&error)) {
		gchar *msg = g_strdup_printf ("Error: Failed to launch \"%s\".",
									  g_app_info_get_executable (G_APP_INFO (app_info)));
		utils_show_dialog ("Error: Failed to launch application",
						   msg,
						   GTK_MESSAGE_ERROR);
		g_free (msg);
		ret = FALSE;
	}

	return ret;
}
/******************************************************************************/
gboolean
vfs_launch_application (const gchar *cmd, const gchar *file_name) {

	gboolean ret = TRUE;
	GList *list = NULL;
	GFile *file = NULL;

	GAppInfo *app_info = g_app_info_create_from_commandline (cmd,
															 NULL,
															 0,
															 NULL);
	if (file_name != NULL) {
		file = g_file_new_for_path (file_name);
		list = g_list_append (list, (gpointer)file);
	}

	ret = _vfs_launch_application (app_info, list);

	g_object_unref (file);
	g_object_unref (app_info);
	g_list_free (list);

	return ret;
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
GtkWidget *
vfs_get_icon_for_file (const gchar *file_name) {

	GIcon *icon = NULL; 
	GtkWidget *icon_widget = NULL;
	GDesktopAppInfo *app_info = g_desktop_app_info_new_from_filename (file_name);

	if (app_info != NULL) {
		icon = g_app_info_get_icon (G_APP_INFO (app_info));
		icon_widget = gtk_image_new_from_gicon (icon,
												GTK_ICON_SIZE_MENU);
		g_object_unref (app_info);
	}
	else {
		GFile *file = g_file_new_for_path (file_name);
		GFileInfo *file_info = g_file_query_info (file,
											  G_FILE_ATTRIBUTE_STANDARD_ICON,
											  0,
											  NULL,
											  NULL);	

		icon = g_file_info_get_icon (file_info);
		icon_widget = gtk_image_new_from_gicon (icon,
												GTK_ICON_SIZE_MENU);
		g_object_unref (file);
		g_object_unref (file_info);
	}
	/*must wait for GTK 2.14*/
	return icon_widget;
}
/******************************************************************************/
GList*
vfs_get_all_mime_applications (const gchar *file_name) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	GFile*	   file = g_file_new_for_path (file_name);
	GFileInfo* file_info =  g_file_query_info (file,
											   "standard::*",
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
GtkWidget*
vfs_get_icon_for_app_info  (GAppInfo *app_info) {

	GIcon *icon = g_app_info_get_icon (app_info);
	GtkWidget *icon_widget = gtk_image_new_from_gicon (icon,
													   GTK_ICON_SIZE_MENU);
	return icon_widget;
}
/******************************************************************************/
void
vfs_run_action (Action *action) {
	vfs_launch_application (action->command, action->file_name);
	g_free (action->command);
	g_free (action->file_name);
	g_free (action);
}
/******************************************************************************/
