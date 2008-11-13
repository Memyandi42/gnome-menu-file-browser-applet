/*
 * File:				vfs.h
 * Created:				February 2008
 * Created by:			Axel von Bertoldi
 * Last Modified:		August 2008
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

#ifndef __VFS_H__
#define __VFS_H__

#include <gtk/gtk.h>

/******************************************************************************/
typedef struct _VfsFileInfo VfsFileInfo;
typedef struct _LaunchInfo LaunchInfo;
/******************************************************************************/
struct _VfsFileInfo {
	gchar		*display_name; /* need to free this */
	gchar		*file_name; /* need to free this */
	GtkWidget	*icon; /* don't need to free this */
	gboolean	is_executable;
};
struct _LaunchInfo {
	gchar *command;	/* the app to launch. can include args in here too, even file names. */
	gchar *file;	/* the file to open if any. Should be NULL if not used. */
};
/******************************************************************************/
gboolean	vfs_file_is_executable		(const gchar *file_name);
gboolean	vfs_file_is_desktop			(const gchar *file_name);
gboolean	vfs_file_is_directory		(const gchar *file_name);
gboolean	vfs_file_exists				(const gchar *file_name);

gchar*		vfs_get_dir_listings		(GPtrArray *files, GPtrArray *dirs, gboolean show_hidden, const gchar *path);
GList*		vfs_get_all_mime_applications (const gchar *file_name);
GtkWidget*	vfs_get_icon_for_file		(const gchar *file_name);

gboolean	vfs_launch_application		(LaunchInfo *launch_info);
gboolean	vfs_file_do_default_action	(const gchar *file_name);
void	 	vfs_file_trash				(gchar *file_name);
/******************************************************************************/

#endif
