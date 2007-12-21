/*
 * File:				menu-browser.c
 * Created:				September 2005
 * Created by:			Axel von Bertoldi
 * Last Modified:		August 2007
 * Last Modified by:	Axel von Bertoldi
 * (C) 2005,2006,2007	Axel von Bertoldi
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

#include <glib.h>
#include <glib/gprintf.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>

#include "menu-browser.h"
#include "utils.h"

/******************************************************************************/
#define G_OBJECT_DATA_NAME "item_path"
/******************************************************************************/
struct _MenuBrowserPrivate {
	gchar			*dir_mime_icon_name;
	GtkWidget		*menu;
	GtkWidget		*menu_item_label;
	GtkIconTheme	*icon_theme;
	GPtrArray		*tmp_array;
};
#define MENU_BROWSER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_MENU_BROWSER, MenuBrowserPrivate))
enum  {
	MENU_BROWSER_DUMMY_PROPERTY
};
/******************************************************************************/
static gpointer menu_browser_parent_class = NULL;
static void menu_browser_dispose (GObject * obj);
/******************************************************************************/
static gchar *
clamp_file_name (const gchar *file_name, gboolean *clamped) {
/* *clamped is true if the string is actually clamped */
  gchar *tmp, *ret;

	if (strlen (file_name) > MAX_FILE_NAME_LENGTH) {
		tmp = g_strndup (file_name, MAX_FILE_NAME_LENGTH - 3);
		ret = g_strdup_printf ("%s...", tmp);
		g_free (tmp);
        if (clamped != NULL) *clamped = TRUE;
		return ret;
	}
	else {
        if (clamped != NULL) *clamped = FALSE;
		return g_strdup (file_name);
	}
}
/******************************************************************************/
static gint
menu_browser_clean_up (MenuBrowser *self) {
	g_printf ("freeing temporary data...\n");
	/* free the structure pointed to by each element */
	g_ptr_array_foreach (self->priv->tmp_array,
						 (GFunc)g_free,
						 NULL);
	/* delete and recreate the array */
    g_ptr_array_free (self->priv->tmp_array, TRUE);
	self->priv->tmp_array = g_ptr_array_new();
	return 0;
}
/******************************************************************************/
static gint
menu_browser_sort_alpha (const gchar **s1,
						 const gchar **s2) {
	return g_ascii_strcasecmp ((gchar *)*s1, (gchar *)*s2);
	/* return strcmp ((gchar *)*s1, (gchar *)*s2); */
}
/******************************************************************************/
static gint
menu_browser_launch_app (gchar **args, const gchar *working_dir) {
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

	if (utils_check_gerror (&error)) return -1;

	return 0;
}
/******************************************************************************/
static gchar *
menu_browser_get_mime_application (const gchar *file_name_and_path) {
	GnomeVFSMimeApplication *mime_application = NULL;
	gchar *mime_type = NULL;
	gchar *file_mime_app_exec = NULL;
	
	mime_type        = gnome_vfs_get_mime_type (file_name_and_path);	
	mime_application = gnome_vfs_mime_get_default_application (mime_type);

	if (mime_application) {
		file_mime_app_exec = g_strdup ((gchar *)mime_application->command);	
	}
    
    g_free (mime_type);
    g_free (mime_application);

    return file_mime_app_exec;
}
/******************************************************************************/
static gint
menu_browser_launch_terminal (MenuBrowser *self, gchar *path) {

	gchar **args = NULL;
		
    args = g_strsplit (self->prefs->terminal,
					   "\1", 0);
	
	menu_browser_launch_app (args, path);
	
	g_free (args);	

	return 0;
}
/******************************************************************************/
static gint
menu_browser_open_file (const gchar *file_name_and_path, gint exec_action) {
	gchar **args = NULL;
	gchar *arg = NULL;
	gchar *file_mime_app_exec = NULL;
	gchar *working_dir = NULL;
	gboolean is_executable;
	int i;

	working_dir = g_path_get_dirname (file_name_and_path);

    is_executable = g_file_test (file_name_and_path, G_FILE_TEST_IS_EXECUTABLE) &&
				   !g_file_test (file_name_and_path, G_FILE_TEST_IS_DIR);

	file_mime_app_exec = menu_browser_get_mime_application	(file_name_and_path);
	/* if it's a binary file (i.e. it has no associated mime type) run it it as if it were a prog*/
    if (is_executable && file_mime_app_exec == NULL) {
		arg = g_strdup_printf ("%s", file_name_and_path);    
    	args = g_strsplit (arg, "\1", 0);
	}
	else {
		if (file_mime_app_exec) {
			/* if it's a script or something, look at the exec_action option*/
			if (is_executable && exec_action == EXEC_RUN) {
				/* run it */
				arg = g_strdup_printf ("%s", file_name_and_path);    
		    	args = g_strsplit (arg, "\1", 0);
			}
			else {
				/* open it for editing/viewing */
				arg = g_strdelimit (file_mime_app_exec, " ", '\1');
				arg = g_strconcat (arg, "\1", file_name_and_path, NULL);    
				args = g_strsplit (arg, "\1", 0);
				g_printf ("%s ", file_mime_app_exec);
			}
		}
		else {
			g_printf ("Error: failed to get mime application for %s\n", file_name_and_path);
			gchar *message = g_strdup_printf ("Cannot open %s:\n"
											  "No application is known for this kind of file.",
											  file_name_and_path);

			utils_show_dialog ("Error: no application found",
										   message,
										   GTK_MESSAGE_ERROR);
			g_free (message);
			return -1;
		}
	}
    g_printf ("%s\n", file_name_and_path);
	menu_browser_launch_app (args, working_dir);
	
	g_free (arg);
	for (i = 0; args[i]; i++) {
		g_free (args[i]);
	}
	g_free (args);
	g_free (file_mime_app_exec);

	return 0;
}
/******************************************************************************/
static gint
menu_browser_on_dir_left_click (const gchar *file_name_and_path) {
	return menu_browser_open_file (file_name_and_path, EXEC_OPEN);
}
/******************************************************************************/
static gint
menu_browser_on_dir_middle_click (MenuBrowser *self, gchar *path) {
    menu_browser_launch_terminal (self, path);

	return 0;
}
/******************************************************************************/
static gint
menu_browser_on_dir_right_click (const gchar *file_name_and_path) {
	g_printf ("Right click on directory action not implemented\n");
	return 0;
}
/******************************************************************************/
static gint
menu_browser_on_file_left_click (const gchar *file_name_and_path) {
	return menu_browser_open_file (file_name_and_path, EXEC_OPEN);
}
/******************************************************************************/
static gint
menu_browser_on_file_middle_click (const gchar *file_name_and_path) {
	return menu_browser_open_file (file_name_and_path, EXEC_RUN);
}
/******************************************************************************/
static gint
menu_browser_on_file_right_click (const gchar *file_name_and_path) {
	g_printf ("Right click on file action not implemented\n");
	return 0;
}
/******************************************************************************/
static gint
menu_browser_on_directory_item_activate (GtkWidget *menu_item,
										 GdkEventButton *event,
										 MenuBrowser *self) {
	gchar *path = (gchar*)g_object_get_data (G_OBJECT (menu_item), G_OBJECT_DATA_NAME);

    if (g_file_test (path, G_FILE_TEST_EXISTS)) {
    	if (event->button == 1) {
		    menu_browser_on_dir_left_click (path);
    	}	
	    else if (event->button == 2) {
	    	menu_browser_on_dir_middle_click (self, path);
    	}	
	    else if (event->button == 3) {
	    	menu_browser_on_dir_right_click (path);
    	}
    }
    else {
        g_printf ("Error: directory %s does not exist\n", path);
    }
	/*menu_browser_clean_up (self);*/
    return 0;
}
/******************************************************************************/
static gint
menu_browser_on_file_item_activate (GtkWidget *menu_item,
									GdkEventButton *event,
									MenuBrowser *self) {

	gchar *path = (gchar*)g_object_get_data (G_OBJECT (menu_item), G_OBJECT_DATA_NAME);

    if (g_file_test (path, G_FILE_TEST_EXISTS)) {
		if (event->button == 1) {
			menu_browser_on_file_left_click (path);
		}	
		else if (event->button == 2) {
			menu_browser_on_file_middle_click (path);
		}	
		else if (event->button == 3) {
			menu_browser_on_file_right_click (path);
		}	  
    }
    else {
        g_printf ("Error: file %s does not exist\n", path);
    }
	/*menu_browser_clean_up (self);*/
    return 0;
}
/******************************************************************************/
static void
menu_browser_clear_menu (GtkWidget *menu_item) {
    GtkWidget *menu = NULL;
	if (1) {
		menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu_item));
		gtk_container_foreach (GTK_CONTAINER (menu),
                           (GtkCallback) gtk_widget_destroy,
                           NULL);
	}
	else {
		GtkWidget *old_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu_item));
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), NULL);
		menu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
								   menu);
		gtk_widget_destroy (GTK_WIDGET (old_menu));
	}
    return;
}
/******************************************************************************/
static gchar *
menu_browser_get_dir_contents (GPtrArray *files,
							   GPtrArray *dirs,
							   MenuBrowser *self,
							   gchar *path) {

	GnomeVFSDirectoryHandle *vfs_dir_handle = NULL;
	GnomeVFSResult    		vfs_result;
	GnomeVFSFileInfo		*vfs_file_info = NULL;
	gchar					*error = NULL;
	gboolean show_hidden = self->prefs->show_hidden;

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
		g_printf ("Error opening directory. GNOME_VFS error: %s\n",
				  error);
	}
    /**************************** Finished reading dir contents ************************/
	
	/*sort the arrays containing the directory and file listings*/
	g_ptr_array_sort (dirs, (GCompareFunc)&menu_browser_sort_alpha);
    g_ptr_array_sort (files, (GCompareFunc)&menu_browser_sort_alpha);

	gnome_vfs_file_info_clear (vfs_file_info);
	g_free (vfs_file_info);
	return error;
}
/******************************************************************************/
static gint
menu_browser_add_menu_header (GtkWidget *current_menu,
							  MenuBrowser *self,
							  gchar *path) {
    GtkWidget *menu_item = NULL;
    GtkWidget *separator = NULL;
	GtkWidget *menu_item_icon = NULL;    

	gchar *dir = g_path_get_basename (path);
	gchar *tmp;
    gboolean clamped = TRUE;
    
    tmp = clamp_file_name(dir, &clamped);
	menu_item = gtk_image_menu_item_new_with_label (tmp);
    if (clamped == TRUE) {
        gtk_widget_set_tooltip_text (menu_item, dir);
	}
	g_free(dir);
	g_free(tmp);
    
	menu_item_icon = gtk_image_new_from_icon_name (self->priv->dir_mime_icon_name,
            									   GTK_ICON_SIZE_MENU);
    
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                    menu_item_icon);

    gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                           menu_item);
    /* FIXME: path should be deleted ? */
	g_object_set_data (G_OBJECT (menu_item),
					   G_OBJECT_DATA_NAME,
					   path);

    g_signal_connect (G_OBJECT (menu_item),
                      "button_release_event",
					  G_CALLBACK (menu_browser_on_directory_item_activate),
					  (gpointer)self);
	
    separator = gtk_separator_menu_item_new();
    
    gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                           separator);

    return 0;
}
/******************************************************************************/
static gint
menu_browser_populate_menu (GtkWidget	*parent_menu_item,
							MenuBrowser *self) {
    gchar 					*file_name_and_path = NULL;
    gchar 					*current_path = NULL;
	gchar					*icon_name = NULL;
	gchar					*error = NULL;
    GtkWidget 				*menu_item = NULL;
    GtkWidget 				*child_menu = NULL;
    GtkWidget 				*current_menu = NULL;
	GtkWidget 				*menu_item_icon = NULL;    
	GPtrArray 				*files = g_ptr_array_new();
	GPtrArray 				*dirs = g_ptr_array_new();
    guint		            i;
	gchar					*tmp = NULL;
    gboolean                clamped = TRUE;


	current_path = (gchar*)g_object_get_data (G_OBJECT (parent_menu_item), G_OBJECT_DATA_NAME);

    /* get the menu widget to pack all the menu items for this dir into */
    current_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (parent_menu_item));

	/* add the dir name and events */
	menu_browser_add_menu_header (current_menu,
								  self,
								  current_path);

	/* read the contents of the dir. */
	error = menu_browser_get_dir_contents (files,
										   dirs,
										   self,
										   current_path);

    /************** for each dir in this dir, add the menu item and events *****************/
    for (i=0; i<dirs->len; i++) { 
        file_name_and_path = g_strdup_printf ("%s/%s",
                                              current_path,
											  (gchar *)g_ptr_array_index (dirs, i)),

        /*make a menu item for this dir, limit the length of the text in the menu*/
        tmp = clamp_file_name ((gchar*)g_ptr_array_index (dirs, i), &clamped);
        menu_item = gtk_image_menu_item_new_with_label (tmp);
        if (clamped == TRUE) {
            gtk_widget_set_tooltip_text (menu_item, (gchar*)g_ptr_array_index (dirs, i));
		}
		g_free(tmp);
		
        /*get the icon widget based on the returned icon name (always the same icon, can speed up here)*/
        menu_item_icon = gtk_image_new_from_icon_name (self->priv->dir_mime_icon_name,
            										   GTK_ICON_SIZE_MENU);

        /*stick the icon in the menu item, the menu item in the menu and show it all*/
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
										menu_item_icon);

       	gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
							   menu_item);		
        
        /*make the sub menu to show all the files in this dir*/
        child_menu = gtk_menu_new ();
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
								   child_menu);

    	/* FIXME: path should be deleted ? */
		g_object_set_data (G_OBJECT (menu_item),
						   G_OBJECT_DATA_NAME,
						   file_name_and_path);
        
        g_signal_connect (GTK_MENU_ITEM (menu_item),
                		  "activate",
						  G_CALLBACK (menu_browser_populate_menu),
						  (gpointer) self);

        g_signal_connect (GTK_MENU_ITEM (menu_item),
						  "deselect",
						  G_CALLBACK (menu_browser_clear_menu),
						  NULL);
    }	
	/*********************Finished adding the dirs to the menu******************/
    
    if ((dirs->len > 0) & (files->len > 0)) {
	    /*add a separator between dirs and files*/
	    menu_item = gtk_separator_menu_item_new();        
        gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                               menu_item);
    }
	
    /********************for each file in this dir, add the menu item and events***************************/
    for (i=0; i<files->len; i++) {  
        file_name_and_path = g_strdup_printf ("%s/%s",
											  current_path,
											  (gchar *)g_ptr_array_index (files, i)),

        
        /*make a menu item for this dir*/
        tmp = clamp_file_name ((gchar*)g_ptr_array_index (files, i), &clamped);
	    menu_item = gtk_image_menu_item_new_with_label (tmp);
        if (clamped == TRUE) {
            gtk_widget_set_tooltip_text (menu_item, (gchar*)g_ptr_array_index (files, i));
		}

		/* bold executable files */
		if (g_file_test (file_name_and_path, G_FILE_TEST_IS_EXECUTABLE) &&
			!g_file_test (file_name_and_path, G_FILE_TEST_IS_DIR)) {
			GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
			gchar *markup = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>", tmp);
			gtk_label_set_markup (GTK_LABEL (label), markup);
			g_free (markup);
		}
		g_free (tmp);

        /*lookup the mime icon name for this file type */
		icon_name = gnome_icon_lookup_sync (self->priv->icon_theme,
									    	NULL,
                                            file_name_and_path,
                                            NULL,
                                            0,
                                            NULL);		

        /*get the icon widget based on the returned icon name*/
        menu_item_icon = gtk_image_new_from_icon_name (icon_name,
                                                	   GTK_ICON_SIZE_MENU);
        
        /*stick the icon in the menu item, the menu item in the menu and show it all*/
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
										menu_item_icon);
        
       	gtk_menu_shell_append (GTK_MENU_SHELL(current_menu),
							   menu_item);
		
    	/* FIXME: path should be deleted ? */
		g_object_set_data (G_OBJECT (menu_item),
						   G_OBJECT_DATA_NAME,
						   file_name_and_path);
        g_signal_connect (GTK_WIDGET (menu_item),
                          "button_release_event",
						  G_CALLBACK (menu_browser_on_file_item_activate),
						  (gpointer) self);
        g_free (icon_name);
    }
	/*********************Finished adding the files to the menu******************/
	if (error != NULL) {	   
	    menu_item = gtk_menu_item_new_with_label (error);
		g_free (error);
        gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                               menu_item);
		gtk_widget_set_sensitive (GTK_WIDGET (menu_item),
								  FALSE);
	}
	else if ((dirs->len == 0) & (files->len == 0)) {
	  
	    menu_item = gtk_menu_item_new_with_label ("(Empty)");        
        gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                               menu_item);
		gtk_widget_set_sensitive (GTK_WIDGET (menu_item),
								  FALSE);
    }
	/****************************************************************************/
    
    gtk_widget_show_all (current_menu);

	/*clean up*/
	g_ptr_array_foreach (dirs, (GFunc)g_free, NULL);
	g_ptr_array_foreach (files, (GFunc)g_free, NULL);
    g_ptr_array_free (dirs, TRUE);
    g_ptr_array_free (files, TRUE);

    return 0;		
}
/******************************************************************************/
void
menu_browser_update (MenuBrowser *self,
					 gchar* path,
					 gchar* label) {
	g_return_if_fail (IS_MENU_BROWSER (self));

	GtkWidget *item_label = GTK_BIN (self)->child;
	
	gtk_label_set_text (GTK_LABEL (item_label), label);
    /* FIXME: path should be deleted ? */
	g_object_set_data (G_OBJECT (self),
					   G_OBJECT_DATA_NAME,
					   g_strdup(path));
	return;
}
/******************************************************************************/
static void
menu_browser_class_init (MenuBrowserClass * klass) {
	menu_browser_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (MenuBrowserPrivate));
	G_OBJECT_CLASS (klass)->dispose = menu_browser_dispose;
}
/******************************************************************************/
static void
menu_browser_init (MenuBrowser * self) {
	self->priv = MENU_BROWSER_GET_PRIVATE (self);
}
/******************************************************************************/
static void
menu_browser_dispose (GObject * obj) {
	MenuBrowser *self;
	MenuBrowserClass *klass;
	self = MENU_BROWSER (obj);
	klass = MENU_BROWSER_CLASS (g_type_class_peek (TYPE_MENU_BROWSER));

	(self->priv->menu == NULL ? NULL : (self->priv->menu = (gtk_widget_destroy (self->priv->menu), NULL)));
	(self->priv->menu_item_label == NULL ? NULL : (self->priv->menu_item_label = (gtk_widget_destroy (self->priv->menu_item_label), NULL)));
	(self->priv->dir_mime_icon_name == NULL ? NULL : (self->priv->dir_mime_icon_name = (g_free (self->priv->dir_mime_icon_name), NULL)));
	(self->priv->tmp_array == NULL ? NULL : (self->priv->tmp_array = (g_ptr_array_free (self->priv->tmp_array, TRUE), NULL)));

	/* do NOT enable this. Ever! */
	/*(self->priv->icon_theme == NULL ? NULL : (self->priv->icon_theme = (g_object_unref (self->priv->icon_theme), NULL)));*/

	G_OBJECT_CLASS (menu_browser_parent_class)->dispose (obj);
	return;
}
/******************************************************************************/
GType
menu_browser_get_type (void) {
	static GType menu_browser_type_id = 0;
	if (G_UNLIKELY (menu_browser_type_id == 0)) {
		static const GTypeInfo g_define_type_info = {sizeof (MenuBrowserClass),
													(GBaseInitFunc) NULL,
													(GBaseFinalizeFunc) NULL,
													(GClassInitFunc) menu_browser_class_init,
													(GClassFinalizeFunc) NULL,
													NULL,
													sizeof (MenuBrowser),
													0,
													(GInstanceInitFunc) menu_browser_init };

		menu_browser_type_id = g_type_register_static (GTK_TYPE_IMAGE_MENU_ITEM,
													   "MenuBrowser",
													   &g_define_type_info, 0);
	}
	return menu_browser_type_id;
}
/******************************************************************************/
static void
menu_browser_reposition_menu (GtkWidget *widget) {
	GtkWidget *menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));
	gtk_menu_reposition (GTK_MENU (menu));
	return;
}
/******************************************************************************/
GtkWidget*
menu_browser_new (const gchar* path,
				  const gchar* label,
				  BrowserPrefs *prefs) {
	MenuBrowser *self;
	self = g_object_newv (TYPE_MENU_BROWSER, 0, NULL);

	if (prefs == NULL) {
		/* get the default config if none passed. */
	/*	prefs = preferences_browser_get_default ();*/
	}
	self->prefs = prefs;	

	self->priv->icon_theme = gtk_icon_theme_get_default();	
	self->priv->tmp_array = g_ptr_array_new();

	/* get the mime icon name for directories */
	self->priv->dir_mime_icon_name = gnome_icon_lookup_sync (self->priv->icon_theme,
    																	   NULL,
																		   path,
																		   NULL,
																		   0,
																		   NULL);
  	GtkWidget *item_label = gtk_label_new (label);
	gtk_misc_set_alignment (GTK_MISC (item_label), 0.0, 0.5);
	gtk_container_add (GTK_CONTAINER (self), item_label);
  	self->priv->menu_item_label = item_label;

	/*make the main menu*/	
	self->priv->menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (self),
							   self->priv->menu);

    /* FIXME: path should be deleted ? */
	g_object_set_data (G_OBJECT (self), G_OBJECT_DATA_NAME, g_strdup (path));

	g_signal_connect (GTK_MENU_ITEM (self),
					  "activate",
					  G_CALLBACK (menu_browser_populate_menu),
					  (gpointer) self);

    g_signal_connect (GTK_MENU_ITEM (self),
                	  "deselect",
					  G_CALLBACK (menu_browser_clear_menu),
					  NULL);

    g_signal_connect_swapped (GTK_MENU_ITEM (self),
                	  		  "deselect",
							  G_CALLBACK (menu_browser_clean_up),
							  self);

	g_signal_connect (G_OBJECT (self),
					  "activate",
					  G_CALLBACK (menu_browser_reposition_menu),
					  self);

	return GTK_WIDGET (self);
}
/******************************************************************************/
