/*
 * File:				menu-browser.c
 * Created:				September 2005
 * Created by:			Axel von Bertoldi
 * Last Modified:		January 2008
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

#include <glib.h>
#include <glib/gprintf.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <libgnome/gnome-desktop-item.h>

#include "menu-browser.h"
#include "utils.h"
#include "vfs.h"

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
static void menu_browser_populate_menu (GtkWidget *parent_menu_item, MenuBrowser *self);
/******************************************************************************/
static void
menu_browser_clean_up (MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);
	/* free the structure pointed to by each element */

	g_ptr_array_foreach (self->priv->tmp_array,
						 (GFunc)g_free,
						 NULL);
	/* delete and recreate the array */
	g_ptr_array_free (self->priv->tmp_array, TRUE);
	self->priv->tmp_array = g_ptr_array_new();
	return;
}
/******************************************************************************/
gboolean
menu_browser_launch_desktop_file (const gchar *file_name) {
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
static void
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
GtkWidget*
menu_browser_add_desktop_file (gchar *file_name, GtkWidget *menu, MenuBrowser *self) {
	if (!vfs_file_is_desktop (file_name)) {
		return NULL;
	}

	GnomeDesktopItem	*ditem = NULL;
	GError				*error = NULL;
	gchar				*icon_name = NULL;
	const gchar			*app_name = NULL;
	GtkWidget			*icon = NULL;
	GtkWidget			*menu_item = NULL;
	GtkIconTheme		*icon_theme = gtk_icon_theme_get_default();	
	
	ditem = gnome_desktop_item_new_from_file (file_name, 0, &error);

	if (utils_check_gerror (&error) || ditem == NULL) {
		gnome_desktop_item_unref (ditem);
		return NULL;
	}

	icon_name = gnome_desktop_item_get_icon (ditem, icon_theme);
	app_name = gnome_desktop_item_get_string (ditem, GNOME_DESKTOP_ITEM_NAME);

	if (icon_name && app_name) {
		menu_item = gtk_image_menu_item_new_with_label (app_name);

		/* This sucks. Should really use gtk_image_new_from_stock */
		icon = utils_get_scaled_image_from_file (icon_name, 20);

		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
		g_free (icon_name);
	}
	gnome_desktop_item_unref (ditem);
	return menu_item;
}
/******************************************************************************/
GtkWidget*
menu_browser_add_default_file (gchar *file_name, GtkWidget *menu, MenuBrowser *self) {
	GtkWidget	*menu_item = NULL;
	GtkWidget	*menu_item_icon = NULL;
	gchar		*icon_name = NULL;
	gchar		*tmp = NULL;
	gchar		*base_name = g_path_get_basename (file_name);
	gboolean	clamped = TRUE;

	/*make a menu item for this dir*/
	tmp = utils_clamp_file_name (base_name, MAX_FILE_NAME_LENGTH, &clamped);
	menu_item = gtk_image_menu_item_new_with_label (tmp);
	if (clamped == TRUE) {
		gtk_widget_set_tooltip_text (menu_item, base_name);
	}

	/* bold executable files */
	if (vfs_file_is_executable (file_name)) {
		GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
		gchar *markup = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>", tmp);
		gtk_label_set_markup (GTK_LABEL (label), markup);
		g_free (markup);
	}
	g_free (tmp);
	g_free (base_name);

	/*lookup the mime icon name for this file type */
	icon_name = gnome_icon_lookup_sync (self->priv->icon_theme,
										NULL,
										file_name,
										NULL,
										0,
										NULL);		

	/*get the icon widget based on the returned icon name*/
	menu_item_icon = gtk_image_new_from_icon_name (icon_name,
												   GTK_ICON_SIZE_MENU);

	/*stick the icon in the menu item, the menu item in the menu and show it all*/
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
									menu_item_icon);
	g_free (icon_name);
	return menu_item;
}
/******************************************************************************/
static void
menu_browser_launch_terminal (const gchar *path, MenuBrowser *self) {

	gchar **args = NULL;
		
	args = g_strsplit (self->prefs->terminal,
					   " ", 0);
	
	menu_browser_launch_app (args, path);
	g_free (args);	

}
/******************************************************************************/
static void
menu_browser_edit_file (const gchar *file_name_and_path, MenuBrowser *self) {
	gchar **args = NULL;
	gchar *arg = NULL;
	gchar *working_dir = NULL;
	int i;

	working_dir = g_path_get_dirname (file_name_and_path);
	arg = g_strdelimit (self->prefs->editor, " ", '\1');
	arg = g_strconcat (arg, "\1", file_name_and_path, NULL);
	args = g_strsplit (arg, "\1", 0);
	menu_browser_launch_app (args, working_dir);
	
	g_free (arg);
	for (i = 0; args[i]; i++) {
		g_free (args[i]);
	}
	g_free (args);
	g_free (working_dir);
}
/******************************************************************************/
static void
menu_browser_open_file (const gchar *file_name_and_path, gint exec_action) {
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
	menu_browser_launch_app (args, working_dir);
	
	g_free (arg);
	for (i = 0; args[i]; i++) {
		g_free (args[i]);
	}
	g_free (args);
	g_free (file_mime_app_exec);

	return;
}
/******************************************************************************/
static void
menu_browser_on_dir_left_click (const gchar *file_name_and_path, MenuBrowser *self) {
	menu_browser_open_file (file_name_and_path, EXEC_OPEN);
	return;
}
/******************************************************************************/
static void
menu_browser_on_dir_middle_click (const gchar *path, MenuBrowser *self) {
	menu_browser_launch_terminal (path, self);
	return;
}
/******************************************************************************/
static void
menu_browser_on_dir_right_click (const gchar *file_name_and_path, MenuBrowser *self) {
	utils_show_dialog ("Error: Action not implemented.",
					   "Right click on directory action not yet implemented\n",
					   GTK_MESSAGE_INFO);
	return;
}
/******************************************************************************/
static void
menu_browser_on_file_left_click (const gchar *file_name_and_path, MenuBrowser *self) {
	/* try launching the desktop file first */
	if (!menu_browser_launch_desktop_file (file_name_and_path)) { 
		menu_browser_open_file (file_name_and_path, EXEC_OPEN);
	}
}
/******************************************************************************/
static void
menu_browser_on_file_middle_click (const gchar *file_name_and_path, MenuBrowser *self) {
	menu_browser_edit_file (file_name_and_path, self);
}
/******************************************************************************/
static void
menu_browser_on_file_right_click (const gchar *file_name_and_path, MenuBrowser *self) {
	utils_show_dialog ("Error: Action not implemented.",
					   "Right click on file action not yet implemented\n",
					   GTK_MESSAGE_INFO);
}
/******************************************************************************/
static void
menu_browser_on_dir_item_activate (GtkWidget *menu_item,
								   MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar *path = (gchar*)g_object_get_data (G_OBJECT (menu_item), G_OBJECT_DATA_NAME);

	if (g_file_test (path, G_FILE_TEST_EXISTS)) {
		GdkEvent *event = gtk_get_current_event ();

		if (event->type == GDK_KEY_PRESS) {
			menu_browser_on_dir_left_click (path, self);
		}
		else if (event->type == GDK_BUTTON_RELEASE) {
			GdkEventButton *button_event = (GdkEventButton *)event;

			if (button_event->button == 1) {
				menu_browser_on_dir_left_click (path, self);
			}	
			else if (button_event->button == 2) {
				menu_browser_on_dir_middle_click (path, self);
			}	
			else if (button_event->button == 3) {
				menu_browser_on_dir_right_click (path, self);
			}
		}
	}
	else {
		gchar *tmp = g_strdup_printf ("Error: \"%s\" does not exists.", path);
		utils_show_dialog ("Error: File not found.",
						   tmp,
						   GTK_MESSAGE_ERROR);
		g_free (tmp);
	}
	menu_browser_clean_up (self);
	return;
}
/******************************************************************************/
static void
menu_browser_on_file_item_activate (GtkWidget *menu_item,
									MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar *path = (gchar*)g_object_get_data (G_OBJECT (menu_item), G_OBJECT_DATA_NAME);

	if (g_file_test (path, G_FILE_TEST_EXISTS)) {
		GdkEvent *event = gtk_get_current_event ();

		if (event->type == GDK_KEY_PRESS) {
			menu_browser_on_file_left_click (path, self);
		}
		else if (event->type == GDK_BUTTON_RELEASE) {
			GdkEventButton *button_event = (GdkEventButton *)event;

			if (button_event->button == 1) {
				menu_browser_on_file_left_click (path, self);
			}	
			else if (button_event->button == 2) {
				menu_browser_on_file_middle_click (path, self);
			}	
			else if (button_event->button == 3) {
				menu_browser_on_file_right_click (path, self);
			}
		}
	}
	else {
		gchar *tmp = g_strdup_printf ("Error: \"%s\" does not exists.", path);
		utils_show_dialog ("Error: File not found.",
						   tmp,
						   GTK_MESSAGE_ERROR);
		g_free (tmp);
	}
	menu_browser_clean_up (self);
	return;
}
/******************************************************************************/
static void
menu_browser_clear_menu (GtkWidget *menu_item) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

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
static void
menu_browser_add_menu_header (GtkWidget *menu,
							  gchar *path,
							  MenuBrowser *self) {
	GtkWidget *menu_item = NULL;
	GtkWidget *separator = NULL;
	GtkWidget *menu_item_icon = NULL;

	gchar *dir = g_path_get_basename (path);
	gchar *tmp;
	gboolean clamped = TRUE;

	tmp = utils_clamp_file_name(dir, MAX_FILE_NAME_LENGTH, &clamped);
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

	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
						   menu_item);

	g_object_set_data (G_OBJECT (menu_item),
					   G_OBJECT_DATA_NAME,
					   path);

	g_signal_connect (G_OBJECT (menu_item),
					  "activate",
					  G_CALLBACK (menu_browser_on_dir_item_activate),
					  self);
	
	separator = gtk_separator_menu_item_new();

	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
						   separator);

	return;
}
/******************************************************************************/
static void
menu_browser_add_folders (GtkWidget		*menu,
						  gchar			*path,
						  GPtrArray		*dirs,
						  MenuBrowser	*self) {

	GtkWidget	*menu_item = NULL;
	GtkWidget	*menu_item_icon = NULL;
	GtkWidget	*child_menu = NULL;
	gchar		*file_name_and_path = NULL;
	gchar		*tmp = NULL;
	gboolean	clamped = TRUE;
	int i;

	for (i=0; i<dirs->len; i++) {
		file_name_and_path = g_strdup_printf ("%s/%s",
											  path,
											  (gchar *)g_ptr_array_index (dirs, i));

		/*make a menu item for this dir, limit the length of the text in the menu*/
		tmp = utils_clamp_file_name ((gchar*)g_ptr_array_index (dirs, i), MAX_FILE_NAME_LENGTH, &clamped);
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

		gtk_menu_shell_append (GTK_MENU_SHELL (menu),
							   menu_item);		

		/*make the sub menu to show all the files in this dir*/
		child_menu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
								   child_menu);

		g_ptr_array_add (self->priv->tmp_array, file_name_and_path);
		g_object_set_data (G_OBJECT (menu_item),
						   G_OBJECT_DATA_NAME,
						   file_name_and_path);

		g_signal_connect (GTK_MENU_ITEM (menu_item),
						  "activate",
						  G_CALLBACK (menu_browser_populate_menu),
						  (gpointer) self);
	}	
	return;
}
/******************************************************************************/
static void
menu_browser_add_files (GtkWidget	*menu,
						gchar		*path,
						GPtrArray	*files,
						MenuBrowser	*self) {

	GtkWidget	*menu_item = NULL;
	gchar		*file_name_and_path = NULL;
	int i;

	for (i=0; i<files->len; i++) {
		file_name_and_path = g_strdup_printf ("%s/%s",
											  path,
											  (gchar *)g_ptr_array_index (files, i));

		/* desktop files */
		menu_item = menu_browser_add_desktop_file (file_name_and_path, menu, self);
		if (menu_item == NULL) {
			menu_item = menu_browser_add_default_file (file_name_and_path, menu, self);
		}
		
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);
		g_ptr_array_add (self->priv->tmp_array, file_name_and_path);
		g_object_set_data (G_OBJECT (menu_item),
						   G_OBJECT_DATA_NAME,
						   file_name_and_path);

		g_signal_connect (menu_item,
						  "activate",
						  G_CALLBACK (menu_browser_on_file_item_activate),
						  self);
	}
	return;
}
/******************************************************************************/
static void
menu_browser_populate_menu (GtkWidget	*parent_menu_item,
							MenuBrowser *self) {
	gchar 		*current_path = NULL;
	gchar		*error = NULL;
	GtkWidget	*menu_item = NULL;
	GtkWidget	*current_menu = NULL;
	GPtrArray	*files = g_ptr_array_new();
	GPtrArray	*dirs = g_ptr_array_new();

	/* empty the menu of its existing contents first */
	menu_browser_clear_menu (parent_menu_item);

	current_path = (gchar*)g_object_get_data (G_OBJECT (parent_menu_item), G_OBJECT_DATA_NAME);

	/* get the menu widget to pack all the menu items for this dir into */
	current_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (parent_menu_item));

	/* add the dir name and events */
	menu_browser_add_menu_header (current_menu,
								  current_path,
								  self);

	/* read the contents of the dir. */
	error = vfs_get_dir_contents (files,
								  dirs,
								  self->prefs->show_hidden,
								  current_path);
	/* add the folders*/
	menu_browser_add_folders (current_menu,
							  current_path,
							  dirs,
							  self);

	if ((dirs->len > 0) & (files->len > 0)) {
		/*add a separator between dirs and files*/
		menu_item = gtk_separator_menu_item_new();
		gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
							   menu_item);
	}

	/* add the files */
	menu_browser_add_files (current_menu,
							current_path,
							files,
							self);

	/* add any error or other messages*/
	if (error != NULL) {
		menu_item = gtk_menu_item_new_with_label (error);
		g_free (error);
	gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
						   menu_item);
		gtk_widget_set_sensitive (GTK_WIDGET (menu_item),
								  FALSE);
	} /* the folder was empty */
	else if ((dirs->len == 0) & (files->len == 0)) {
		menu_item = gtk_menu_item_new_with_label ("(Empty)");
		gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
							   menu_item);
		gtk_widget_set_sensitive (GTK_WIDGET (menu_item),
								  FALSE);
	}

	gtk_widget_show_all (current_menu);

	/*clean up*/
	g_ptr_array_foreach (dirs, (GFunc)g_free, NULL);
	g_ptr_array_foreach (files, (GFunc)g_free, NULL);
	g_ptr_array_free (dirs, TRUE);
	g_ptr_array_free (files, TRUE);

	return;		
}
/******************************************************************************/
void
menu_browser_update (MenuBrowser *self,
					 gchar* path,
					 gchar* label) {
	g_return_if_fail (IS_MENU_BROWSER (self));

	GtkWidget *item_label = GTK_BIN (self)->child;
	
	gtk_label_set_text (GTK_LABEL (item_label), label);

	g_object_set_data (G_OBJECT (self),
					   G_OBJECT_DATA_NAME,
					   g_strdup (path));
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

	g_object_set_data (G_OBJECT (self), G_OBJECT_DATA_NAME, (char *)path);

	g_signal_connect (GTK_MENU_ITEM (self),
					  "activate",
					  G_CALLBACK (menu_browser_populate_menu),
					  (gpointer) self);

	/* unfortunately this signal is activated before the "activate" signal,
	 * which has the result of deleting all objects before the activate signal
	 * handler is called.*/
/*
	g_signal_connect_swapped (GTK_MENU_ITEM (self),
							  "deselect",
							G_CALLBACK (menu_browser_clean_up),
							self);
*/
	g_signal_connect (G_OBJECT (self),
					  "activate",
					  G_CALLBACK (menu_browser_reposition_menu),
					  self);

	return GTK_WIDGET (self);
}
/******************************************************************************/
