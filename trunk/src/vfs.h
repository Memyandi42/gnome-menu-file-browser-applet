/*
 * File:                vfs.h
 * Created:             February 2008
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

#ifndef __VFS_H__
#define __VFS_H__

#include <gtk/gtk.h>

/******************************************************************************/
enum {
    ARG_APP,
    ARG_FILE,
    ARGS_SIZE,
};
/******************************************************************************/
typedef struct _VfsFileInfo VfsFileInfo;
/******************************************************************************/
struct _VfsFileInfo {
    gchar       *display_name;  /* DO need to free this */
    gchar       *file_name;     /* DO need to free this */
    GtkWidget   *icon;          /* Don't need to free this */
    gboolean    is_executable;
    gchar       *size;          /* DO need to free this */
};
/******************************************************************************/
gboolean    vfs_file_is_executable   (const gchar *file_name);
gboolean    vfs_file_is_desktop      (const gchar *file_name);
gboolean    vfs_file_is_directory    (const gchar *file_name);
gboolean    vfs_file_exists          (const gchar *file_name);

gchar*      vfs_get_dir_listings        (GPtrArray *files, GPtrArray *dirs, gboolean show_hidden, gboolean show_thumbnail, gboolean hide_files, const gchar *path);
GList*      vfs_get_all_mime_applications (const gchar *file_name);
GtkWidget*  vfs_get_icon_for_file       (const gchar *file_name);

gboolean    vfs_launch_application      (const gchar *const*args);
gboolean    vfs_file_do_default_action  (const gchar *file_name);
void        vfs_file_trash              (const gchar *file_name);
/******************************************************************************/

#endif
