/*
 * File:                vfs.c
 * Created:             August 2008
 * Created by:          Axel von Bertoldi
 * Last Modified:       November 2009
 * Last Modified by:    Axel von Bertoldi
 * (C) 2005-2009        Axel von Bertoldi
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
#include <glib/gstdio.h>
#include <gio/gio.h>

#include "vfs.h"
#include "utils.h"
#include "config.h"

/******************************************************************************/
const gchar *size_units[] = {"bytes","KB","MB","GB","TB","HUGE",NULL};
/******************************************************************************/
/* Caller must free return value. */
static inline gchar * vfs_human_file_size (guint64 size) {
    int order_of_magnitude = 0;
    double _size = size;
    while (_size > 1024){
        _size = _size / 1024;
        order_of_magnitude++;
    }
    gchar *human_size = g_strdup_printf ("%.1f %s", _size,
                                         (order_of_magnitude <= 5 ?
                                          size_units[order_of_magnitude] :
                                          size_units[5]));
    return human_size;
}
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
    GFile*     file = g_file_new_for_path (file_name);
    GFileInfo* file_info =  g_file_query_info (file,
                                               /*G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE ,*/
                                               "standard::content-type,"
                                               "access::can-execute",
                                               0,
                                               NULL,
                                               NULL);

    gboolean ret = g_file_info_get_attribute_boolean (file_info,
                                                      G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE) &&
                                                      g_content_type_can_be_executable (g_file_info_get_content_type (file_info));

    g_object_unref (file_info);
    g_object_unref (file);
    return ret; 
}
/******************************************************************************/
gboolean
vfs_file_is_directory (const gchar *file_name) {
    GFile*     file = g_file_new_for_path (file_name);
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
    GFile*     file = g_file_new_for_path (file_name);
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
vfs_file_trash (const gchar *file_name) {
    GError *error = NULL;
    GFile *file = g_file_new_for_path (file_name);

    /* Try moving it to the trash */
    g_file_trash (file, NULL, &error);

    /* Let the user know if we failed. */
    utils_gerror_ok (&error, TRUE);
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
    GFile*     file = g_file_new_for_path (file_name);
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
/* Takes a null-terminated array of strings at least 2 items long. The first
 * item is always the application to execute, including if it is a script or
 * .desktop file. The second is the files to open. Either ARG_FILE or ARG_APP
 * can be NULL, but not both. If ARG_FILE is NULL, just launch ARG_APP. If
 * ARG_APP is NULL, open ARG_FILE with the default mime handler. */
gboolean
vfs_launch_application (const gchar *const *args) {

    /* Can't have both arg[ARG_APP] and args[ARG_FILE} NULL */
    g_return_val_if_fail ((args[ARG_APP] != NULL || args[ARG_FILE] != NULL), FALSE);

    guint    i;
    gboolean ret       = FALSE;
    GError   *error    = NULL;
    GList    *files    = NULL;
    gchar    *command_line = NULL;

    /* Put the file(s) in a list (only of there is a file) for g_app_info_launch() */
    for (i = ARG_FILE; args[i]; i++) {
        files = g_list_append (files, (gpointer)g_file_new_for_path (args[i]));
    }

    gchar *uri = args[ARG_FILE] ? g_filename_to_uri(args[ARG_FILE], NULL, NULL) : g_filename_to_uri(args[ARG_APP], NULL, NULL);
    GtkRecentManager *recent = gtk_recent_manager_new ();
    gtk_recent_manager_add_item (recent, uri);
    g_object_unref (G_OBJECT(recent));
    g_free (uri);

    /* Set the current working dir. Do we use ARG_APP or ARG_FILE to set
     * cwd?  If ARG_FILE == NULL, use ARG_APP, otherwise use ARG_FILE */
    const gchar *cwd_root = args[ARG_FILE] == NULL ? args[ARG_APP] : args[ARG_FILE];
    gchar *working_dir = vfs_file_is_directory (cwd_root) ? g_strdup (cwd_root) : g_path_get_dirname (cwd_root);
    g_chdir (working_dir);
    g_free (working_dir);

    /* No agrs[ARG_APP] was NULL, open file with default mime handler */
    if (args[ARG_APP] == NULL) {
        GFile *file = g_file_new_for_path (args[ARG_FILE]);
        gchar *uri = g_file_get_uri (file);
        ret = g_app_info_launch_default_for_uri (uri, NULL, &error);
        utils_gerror_ok (&error, TRUE);
        g_free (uri);
        g_object_unref (file);
    }
    else {
        GAppInfo *app_info = NULL;
        if (vfs_file_is_desktop (args[ARG_APP])) {
            app_info = G_APP_INFO (g_desktop_app_info_new_from_filename (args[ARG_APP]));
        }
        else {
            command_line = vfs_file_is_executable (args[ARG_APP]) ?
                utils_escape_str (args[ARG_APP], " ", "\\ ") : g_strdup (args[ARG_APP]);
            app_info = g_app_info_create_from_commandline (command_line, NULL, 0, &error);
            utils_gerror_ok (&error, TRUE);
        }

        if (app_info) {
            ret = g_app_info_launch (app_info, files, NULL, &error);
            utils_gerror_ok (&error, TRUE);
            g_object_unref (app_info);
        }
        else {
            gchar *msg = g_strdup_printf (_("Could not open \"%s\".\n"), args[ARG_FILE]);
            utils_show_dialog (_("Error"), msg, GTK_MESSAGE_ERROR);
            g_free (msg);
        }
    }
    g_list_foreach (files, (GFunc)g_object_unref, NULL);
    g_list_free (files);
    g_free (command_line);

    g_chdir (g_get_home_dir());

    return ret;
}
/******************************************************************************/
gboolean
vfs_file_do_default_action (const gchar *file_name) {
    gchar **args = g_strv_new (ARGS_SIZE);
    gchar *_file_name = g_strdup (file_name);

    if (vfs_file_is_desktop (_file_name) ||
        vfs_file_is_executable (_file_name)) {
        args[ARG_APP] = _file_name;
    }
    /* open it with the default mime app */
    else {
        args[ARG_FILE] = _file_name;
    }
    gboolean ret = vfs_launch_application ((const gchar*const*)args);
    g_strfreev (args);
    return ret;
}
/******************************************************************************/
/* Gets the contents of a directory. Puts the files and directories in separate
 * arrays, and sorts them both. The caller must free the return value. */
gchar*
vfs_get_dir_listings (GPtrArray *files,
                      GPtrArray *dirs,
                      gboolean show_hidden,
                      gboolean show_thumbnail,
                      gboolean hide_files,
                      const gchar *path) {
    GError *error = NULL;
    const gchar *_attributes = "standard::type,standard::is-hidden,standard::name,standard::display-name,access::can-execute,standard::size";
    gchar *attributes = show_thumbnail ? g_strdup_printf ("%s,thumbnail::path", _attributes) : g_strdup(_attributes);

    GFile *file = g_file_new_for_path (path);
    /* get ALL the info about the files */
    GFileEnumerator *enumerator = g_file_enumerate_children (file,
                                                             attributes,
                                                             0,
                                                             NULL,
                                                             &error);
    g_free (attributes);
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
        if (g_file_info_get_file_type (file_info) != G_FILE_TYPE_DIRECTORY && hide_files) {
            g_object_unref (file_info);
            continue;
        }

        VfsFileInfo *vfs_file_info = g_new0 (VfsFileInfo ,1);
        vfs_file_info->icon = NULL;

        vfs_file_info->file_name = g_strdup_printf ("%s/%s", path, g_file_info_get_name (file_info));

        /* get the file's human readable name, including if it's a desktop file */

        
        vfs_file_info->display_name = (vfs_file_is_desktop (vfs_file_info->file_name)) ?
                                       vfs_get_desktop_app_name (vfs_file_info->file_name) :
                                       g_strdup (g_file_info_get_display_name (file_info));

        vfs_file_info->is_executable = g_file_info_get_attribute_boolean (file_info,
                                                                          G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE) &&
                                            (g_file_info_get_file_type (file_info) != G_FILE_TYPE_DIRECTORY);

        /* get the icon or thumbnail */
        /* FIXME: How expensive is this call? Should put it around an if (show_hidden) if it is expensive. */
        const gchar *thumbnail = g_file_info_get_attribute_byte_string (file_info, G_FILE_ATTRIBUTE_THUMBNAIL_PATH);
        if (thumbnail) {
            vfs_file_info->icon = gtk_image_new_from_file (thumbnail);
        }
        else {
            vfs_file_info->icon = vfs_get_icon_for_file (vfs_file_info->file_name); 
        }

        vfs_file_info->size = vfs_human_file_size (g_file_info_get_size (file_info));

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
