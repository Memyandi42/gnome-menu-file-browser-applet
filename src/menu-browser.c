/*
 * File:				menu-browser.c
 * Created:				September 2005
 * Created by:			Axel von Bertoldi
 * Last Modified:		March 2007
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

#include "menu-browser.h"

/****************** "Private" data ********************************************/
struct _MenuFileBrowserPrivate
{
	gchar			*label;						/* the label of the main menu item */
	gchar			*root_path;					/* root path of the menu */
	gchar			*directory_mime_icon_name;
	GtkWidget		*menu;						/* main menu */
	GtkIconTheme	*icon_theme;
	GPtrArray 		*tmp_handle;				/* ptr to array that holds all strings.
												   used in menu_file_browser_free_structure
												   to delete all string when the menu hides */
};
/******************************************************************************/
typedef struct
{
	MenuFileBrowser	*file_browser;
	gchar			*path;
} Pair;
/****************** "Private" functions ***************************************/
gint	menu_file_browser_populate_menu			(GtkWidget *parent_menu_item, Pair *pair);
gint	menu_file_browser_add_menu_header		(GtkWidget *current_menu, Pair *pair);
gchar*	menu_file_browser_get_dir_contents		(GPtrArray *files, GPtrArray *dirs, Pair *pair);
gint	menu_file_browser_on_file_item_activate (Pair *pair, GdkEventButton *event);
gint	menu_file_browser_on_directory_item_activate (Pair *pair, GdkEventButton *event);
gint	menu_file_browser_on_file_left_click	(const gchar *file_name_and_path);
gint	menu_file_browser_on_file_middle_click	(const gchar *file_name_and_path);
gint 	menu_file_browser_on_file_right_click	(const gchar *file_name_and_path);
gint 	menu_file_browser_on_dir_left_click		(const gchar *file_name_and_path);
gint 	menu_file_browser_on_dir_middle_click	(const gchar *file_name_and_path);
gint 	menu_file_browser_on_dir_right_click	(const gchar *file_name_and_path);
gint	menu_file_browser_open_file				(const gchar *file_name_and_path);
gchar*	menu_file_browser_get_mime_application	(const gchar *file_name_and_path);
gint	menu_file_browser_launch_app			(gchar **args, const gchar *working_dir);
gint	menu_file_browser_launch_terminal		(const gchar *file_name_and_path);
gint	menu_file_browser_sort_alpha			(const gchar **s1, const gchar **s2);
void	menu_file_browser_clear_menu			(GtkWidget *menu_item);
gint	menu_file_browser_clean_up				(MenuFileBrowser *file_browser);
void	menu_file_browser_free_structure		(Pair *pair);
void	menu_file_browser_show_dialog			(gchar * title, gchar *message, GtkMessageType type);
/******************************************************************************/
MenuFileBrowser *
menu_file_browser_new (const gchar *label,
					   const gchar *root_path)
{
	MenuFileBrowser *file_browser;
	file_browser = g_new0 (MenuFileBrowser, 1); 
	file_browser->priv = g_new0 (MenuFileBrowserPrivate, 1); 

	file_browser->priv->icon_theme = gtk_icon_theme_get_default();	
	file_browser->priv->tmp_handle = g_ptr_array_new();

	/* initialize structure */
	file_browser->priv->label = g_strdup (label);
	file_browser->priv->root_path = g_strdup (root_path);

	/*make the menu header, main menu item and icon*/
    file_browser->menu_item = gtk_image_menu_item_new_with_label (label);

	/* get the mime icon name for directories */
	file_browser->priv->directory_mime_icon_name = gnome_icon_lookup_sync (file_browser->priv->icon_theme,
    																	   NULL,
																		   root_path,
																		   NULL,
																		   0,
																		   NULL);

	/*make the main menu*/	
	file_browser->priv->menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_browser->menu_item),
											  file_browser->priv->menu);

	Pair *pair;
	pair = g_new0 (Pair, 1);
	pair->file_browser = file_browser;
	pair->path = file_browser->priv->root_path;

	g_signal_connect (GTK_MENU_ITEM (file_browser->menu_item),
					  "activate",
					  G_CALLBACK (menu_file_browser_populate_menu),
					  (gpointer) pair);

    g_signal_connect (GTK_MENU_ITEM (file_browser->menu_item),
                	  "deselect",
					  G_CALLBACK (menu_file_browser_clear_menu),
					  NULL);

    g_signal_connect_swapped (GTK_MENU_ITEM (file_browser->menu_item),
                	  "deselect",
					  G_CALLBACK (menu_file_browser_clean_up),
					  file_browser);

	return file_browser;	
}
/******************************************************************************/
gint
menu_file_browser_populate_menu (GtkWidget	*parent_menu_item,
								 Pair		*pair)
{
    gchar 					*file_name_and_path = NULL;
	gchar					*icon_name = NULL;
	gchar					*error = NULL;
    GtkWidget 				*menu_item = NULL;
    GtkWidget 				*child_menu = NULL;
    GtkWidget 				*current_menu = NULL;
	GtkWidget 				*menu_item_icon = NULL;    
	GPtrArray 				*files = g_ptr_array_new();
	GPtrArray 				*dirs = g_ptr_array_new();
    guint		            i;
	Pair					*this_pair = NULL;
	
    /* get the menu widget to pack all the menu items for this dir into */
    current_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (parent_menu_item));
    
	/* add the dir name and events */
	menu_file_browser_add_menu_header (current_menu,
									   pair);

	/* read the contents of the dir. */
	error = menu_file_browser_get_dir_contents (files,
												dirs,
												pair);

    /************** for each dir in this dir, add the menu item and events *****************/
    for (i=0; i<dirs->len; i++)
    { 
        file_name_and_path = g_strdup_printf ("%s/%s",
                                              pair->path,
											  (gchar *)g_ptr_array_index (dirs, i)),
        /*make a menu item for this dir*/
	    menu_item = gtk_image_menu_item_new_with_label ((gchar*)g_ptr_array_index (dirs, i));
		
        /*get the icon widget based on the returned icon name (always the same icon, can speed up here)*/
        menu_item_icon = gtk_image_new_from_icon_name (pair->file_browser->priv->directory_mime_icon_name,
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

		this_pair = g_new0 (Pair, 1);
		this_pair->file_browser = pair->file_browser;
		this_pair->path = file_name_and_path;
		g_ptr_array_add (pair->file_browser->priv->tmp_handle, this_pair);

        g_signal_connect (GTK_MENU_ITEM (menu_item),
                		  "activate",
						  G_CALLBACK (menu_file_browser_populate_menu),
						  (gpointer) this_pair);

        g_signal_connect (GTK_MENU_ITEM (menu_item),
						  "deselect",
						  G_CALLBACK (menu_file_browser_clear_menu),
						  NULL);
    }	
	/*********************Finished adding the dirs to the menu******************/
    
    if ((dirs->len > 0) & (files->len > 0))
    {
	    /*add a separator between dirs and files*/
	    menu_item = gtk_separator_menu_item_new();        
        gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                               menu_item);
    }
	
    /********************for each file in this dir, add the menu item and events***************************/
    for (i=0; i<files->len; i++)
    {  
        file_name_and_path = g_strdup_printf ("%s/%s",
											  pair->path,
											  (gchar *)g_ptr_array_index (files, i)),
        /*make a menu item for this dir*/
	    menu_item = gtk_image_menu_item_new_with_label ((gchar *)g_ptr_array_index (files, i));
        
        /*lookup the mime icon name for this file type */
		icon_name = gnome_icon_lookup_sync (pair->file_browser->priv->icon_theme,
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
		
		this_pair = g_new0 (Pair, 1);
		this_pair->file_browser = pair->file_browser;
		this_pair->path = file_name_and_path;
		g_ptr_array_add (pair->file_browser->priv->tmp_handle, this_pair);
		
        g_signal_connect_swapped (GTK_WIDGET (menu_item),
                            	  "button_press_event",
								  G_CALLBACK (menu_file_browser_on_file_item_activate),
								  (gpointer) this_pair);
        g_free (icon_name);
    }
	/*********************Finished adding the files to the menu******************/
	if (error != NULL)
	{	   
	    menu_item = gtk_menu_item_new_with_label (error);
		g_free (error);
        gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                               menu_item);
		gtk_widget_set_sensitive (GTK_WIDGET (menu_item),
								  FALSE);
	}
	else if ((dirs->len == 0) & (files->len == 0))
    {
	  
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
gint
menu_file_browser_add_menu_header (GtkWidget *current_menu,
								   Pair *pair)
{
    GtkWidget *menu_item = NULL;
    GtkWidget *separator = NULL;
	GtkWidget *menu_item_icon = NULL;    

	const gchar *file_name_and_path = pair->path;

	menu_item = gtk_image_menu_item_new_with_label (file_name_and_path);
    
    menu_item_icon = gtk_image_new_from_stock ("gtk-home",
                                               GTK_ICON_SIZE_MENU);
    
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                    menu_item_icon);

    gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                           menu_item);
    
    g_signal_connect_swapped (GTK_WIDGET (menu_item),
                        	  "button_press_event",
							  G_CALLBACK (menu_file_browser_on_directory_item_activate),
							  (gpointer)pair);
	
    separator = gtk_separator_menu_item_new();
    
    gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
                           separator);

    return 0;
}
/******************************************************************************/
gchar *
menu_file_browser_get_dir_contents (GPtrArray *files,
									GPtrArray *dirs,
									Pair *pair)
{
	GnomeVFSDirectoryHandle *vfs_dir_handle = NULL;
	GnomeVFSResult    		vfs_result;
	GnomeVFSFileInfo		*vfs_file_info = NULL;
	gchar					*error = NULL;

	/*make struct for getting file info, open the dir for reading and get the first entry*/
	vfs_file_info = gnome_vfs_file_info_new();
	
    vfs_result = gnome_vfs_directory_open (&vfs_dir_handle,
										   pair->path,
/*										   GNOME_VFS_FILE_INFO_GET_MIME_TYPE |*/
										   GNOME_VFS_FILE_INFO_FOLLOW_LINKS);

  	/* make sure the dir was opened OK. This fixes bug #3 */
	if (vfs_result == GNOME_VFS_OK)
	{	/* get the first entry */
    	vfs_result = gnome_vfs_directory_read_next (vfs_dir_handle,
													vfs_file_info);
	    /* if it opened OK and while its not empty, keep reading items */
		while (vfs_result == GNOME_VFS_OK)
	    {	
        	/*if it's not a hidden file...*/
    		if (g_ascii_strncasecmp (vfs_file_info->name, ".", 1))
		    {
				if (vfs_file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY)
        	    {	/*make an array that holds all the dirs in this dir*/
					g_ptr_array_add (dirs, (gpointer)g_strdup (vfs_file_info->name));
	            }
				if (vfs_file_info->type == GNOME_VFS_FILE_TYPE_REGULAR)
	            {	/*make an array that holds all the files in this dir*/
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
	else
	{
		error = g_strdup_printf ("(%s)",gnome_vfs_result_to_string (vfs_result));
		g_printf ("Error opening directory. GNOME_VFS error: %s\n",
				  error);
	}
    /**************************** Finished reading dir contents ************************/
	
	/*sort the arrays containing the directory and file listings*/
	g_ptr_array_sort (dirs, (GCompareFunc)&menu_file_browser_sort_alpha);
    g_ptr_array_sort (files, (GCompareFunc)&menu_file_browser_sort_alpha);

	gnome_vfs_file_info_clear (vfs_file_info);
	g_free (vfs_file_info);
	return error;
}
/******************************************************************************/
gint
menu_file_browser_on_file_item_activate (Pair *pair,
										 GdkEventButton *event)
{
	const gchar *file_name_and_path = pair->path;

    if (g_file_test (file_name_and_path, G_FILE_TEST_EXISTS))
    {
		if (event->button == 1)
		{
			menu_file_browser_on_file_left_click (file_name_and_path);
		}	
		else if (event->button == 2)
		{
			menu_file_browser_on_file_middle_click (file_name_and_path);
		}	
		else if (event->button == 3)
		{
			menu_file_browser_on_file_right_click (file_name_and_path);
		}	  
    }
    else
    {
        g_printf ("Error: file %s does not exist\n", file_name_and_path);
    }
	/*menu_file_browser_clean_up (pair->file_browser);*/
    return 0;
}
/******************************************************************************/
gint
menu_file_browser_on_directory_item_activate (Pair *pair,
											  GdkEventButton *event)
{
	const gchar *file_name_and_path = pair->path;

    if (g_file_test (file_name_and_path, G_FILE_TEST_EXISTS))
    {
    	if (event->button == 1)
	    {
		    menu_file_browser_on_dir_left_click (file_name_and_path);
    	}	
	    else if (event->button == 2)
    	{
	    	menu_file_browser_on_dir_middle_click (file_name_and_path);
    	}	
	    else if (event->button == 3)
    	{
	    	menu_file_browser_on_dir_right_click (file_name_and_path);
    	}
    }
    else
    {
        g_printf ("Error: directory %s does not exist\n", file_name_and_path);
    }
	/*menu_file_browser_clean_up (pair->file_browser);*/
    return 0;
}
/******************************************************************************/
gint
menu_file_browser_on_file_left_click (const gchar *file_name_and_path)
{
	return menu_file_browser_open_file (file_name_and_path);
}
/******************************************************************************/
gint
menu_file_browser_on_file_middle_click (const gchar *file_name_and_path)
{
	g_printf ("Middle click on file action not implemented\n");
	return 0;
}
/******************************************************************************/
gint
menu_file_browser_on_file_right_click (const gchar *file_name_and_path)
{
	g_printf ("Right click on file action not implemented\n");
	return 0;
}
/******************************************************************************/
gint
menu_file_browser_on_dir_left_click (const gchar *file_name_and_path)
{
	return menu_file_browser_open_file (file_name_and_path);
}
/******************************************************************************/
gint
menu_file_browser_on_dir_middle_click (const gchar *file_name_and_path)
{
    menu_file_browser_launch_terminal (file_name_and_path);

	return 0;
}
/******************************************************************************/
gint
menu_file_browser_on_dir_right_click (const gchar *file_name_and_path)
{
	g_printf ("Right click on directory action not implemented\n");
	return 0;
}
/******************************************************************************/
gint
menu_file_browser_open_file (const gchar *file_name_and_path)
{
	gchar **args = NULL;
	gchar *arg = NULL;
	gchar *file_mime_app_exec = NULL;
	gchar *working_dir = NULL;
	int i;

	working_dir = g_path_get_dirname (file_name_and_path);

    if (g_file_test (file_name_and_path, G_FILE_TEST_IS_EXECUTABLE) &&
		!g_file_test (file_name_and_path, G_FILE_TEST_IS_DIR))
	{
		arg = g_strdup_printf ("%s", file_name_and_path);    
    	args = g_strsplit (arg, "\1", 0);
	}
	else /* already checked if it exists */
	{
		file_mime_app_exec = menu_file_browser_get_mime_application	(file_name_and_path);

		if (file_mime_app_exec)
		{
			arg = g_strdelimit (file_mime_app_exec, " ", '\1');
			arg = g_strconcat (arg, "\1", file_name_and_path, NULL);    
    		args = g_strsplit (arg, "\1", 0);
    		g_printf ("%s ", file_mime_app_exec);
		}
		else
		{
			g_printf ("Error: failed to get mime application for %s\n", file_name_and_path);
			gchar *message = g_strdup_printf ("Cannot open %s:\n"
											  "No application is known for this kind of file.",
											  file_name_and_path);

			menu_file_browser_show_dialog ("Error: no application found",
										   message,
										   GTK_MESSAGE_ERROR);
			g_free (message);
			return -1;
		}
	}
    g_printf ("%s\n", file_name_and_path);
	menu_file_browser_launch_app (args, working_dir);
	
	g_free (arg);
	for (i = 0; args[i]; i++)
	{
		g_free (args[i]);
	}
	g_free (args);
	g_free (file_mime_app_exec);

	return 0;
}
/******************************************************************************/
gchar *
menu_file_browser_get_mime_application (const gchar *file_name_and_path)
{
	GnomeVFSMimeApplication *mime_application = NULL;
	gchar *mime_type = NULL;
	gchar *file_mime_app_exec = NULL;
	
	mime_type        = gnome_vfs_get_mime_type (file_name_and_path);	
	mime_application = gnome_vfs_mime_get_default_application (mime_type);

	if (mime_application)
	{
		file_mime_app_exec = g_strdup ((gchar *)mime_application->command);	
	}
    
    g_free (mime_type);
    g_free (mime_application);

    return file_mime_app_exec;
}
/******************************************************************************/
gint
menu_file_browser_launch_app (gchar **args, const gchar *working_dir)
{
	GError *error = NULL;
	gint child_pid;
	gboolean ret;
	gchar *message = NULL;
	
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
	if (error != NULL)
	{
		message = (gchar*)g_strdup_printf ("%s\n", error->message);
		g_printf ("Error: %s\n", message);
		g_free (message);
		g_error_free (error);
		return -1;
	}
	return 0;
}
/******************************************************************************/
gint
menu_file_browser_launch_terminal (const gchar *file_name_and_path)
{
	gchar **args = NULL;
	gchar *arg = NULL;
		
    arg = g_strdup ("gnome-terminal");
    args = g_strsplit (arg, "\1", 0);	
	
	menu_file_browser_launch_app (args, file_name_and_path);
	
	g_free (arg);
	g_free (args);	

	return 0;
}
/******************************************************************************/
gint
menu_file_browser_sort_alpha (const gchar **s1,
							  const gchar **s2)
{
	return g_ascii_strcasecmp ((gchar *)*s1, (gchar *)*s2);
	/* return strcmp ((gchar *)*s1, (gchar *)*s2); */
}
/******************************************************************************/
void
menu_file_browser_clear_menu (GtkWidget *menu_item)
{
    GtkWidget *menu = NULL;
	if (1)
	{
		menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu_item));
		gtk_container_foreach (GTK_CONTAINER (menu),
                           (GtkCallback) gtk_widget_destroy,
                           NULL);
	}
	else
	{
		GtkWidget *old_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu_item));
		gtk_menu_item_remove_submenu (GTK_MENU_ITEM (menu_item));
		menu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
								   menu);
		gtk_widget_destroy (GTK_WIDGET (old_menu));
	}
    return;
}
/******************************************************************************/
gint
menu_file_browser_clean_up (MenuFileBrowser *file_browser)
{
	g_printf ("freeing temporary data...\n");
	/* free the structure pointed to by each element */
	g_ptr_array_foreach (file_browser->priv->tmp_handle,
						 (GFunc)menu_file_browser_free_structure,
						 NULL);
	/* delete and recreate the array */
    g_ptr_array_free (file_browser->priv->tmp_handle, TRUE);
	file_browser->priv->tmp_handle = g_ptr_array_new();
	return 0;
}
/******************************************************************************/
void
menu_file_browser_free_structure (Pair *pair)
{	/* but don't ever delete ->file_browser as it is a ref to a global object
	   shared by all*/
/*printf ("deleting %s\n", pair->path);*/
	g_free (pair->path);
	g_free (pair);
	return;
}
/******************************************************************************/
void
menu_file_browser_show_dialog (gchar *title, gchar *message, GtkMessageType type)
{
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
                            					GTK_DIALOG_MODAL |
												GTK_DIALOG_DESTROY_WITH_PARENT |
												GTK_DIALOG_NO_SEPARATOR,
												type,
												GTK_BUTTONS_CLOSE,
												message);
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	return;
}
