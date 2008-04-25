/*
 * File:				vfs.h
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

#ifndef VFS_H
#define VFS_H

#include <glib.h>
#include <gtk/gtk.h>

#include "config.h"

#define DEBUG 1

gboolean vfs_file_is_executable (const gchar *file_name);
gboolean vfs_file_is_desktop (const gchar *file_name);
gboolean vfs_file_exists (const gchar *file_name);
gchar*	 vfs_get_mime_application (const gchar *file_name_and_path);
gchar*	 vfs_get_dir_contents (GPtrArray *files, GPtrArray *dirs, gboolean show_hidden, gchar *path);

#endif
