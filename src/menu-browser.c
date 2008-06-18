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
#include "context-menu.h"

/*#define NEW_MENU_SIGNAL*/
/******************************************************************************/
#define MAX_FILE_NAME_LENGTH 30 /* What's a good value here??? */
#define G_OBJECT_DATA_NAME "item_path"
/******************************************************************************/
struct _MenuBrowserPrivate {
	gchar			*dir_mime_icon_name;
	GtkWidget		*menu;
	GtkWidget		*menu_item_label;
	GtkMenuShell	*parent_menu_shell;
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
}
/******************************************************************************/
static void
menu_browser_clean_up (MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	/* free the structure pointed to by each element */
	g_ptr_array_foreach (self->priv->tmp_array,
						 (GFunc)g_free,
						 NULL);
	/* delete and recreate the array */
	g_ptr_array_free (self->priv->tmp_array, TRUE);
	self->priv->tmp_array = g_ptr_array_new();

	menu_browser_clear_menu (GTK_WIDGET (self));
}
/******************************************************************************/
GtkWidget*
menu_browser_add_desktop_file (gchar *file_name, GtkWidget *menu, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

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
		icon = utils_get_scaled_image_from_file (icon_name, ICON_MENU_SIZE);

		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
		g_free (icon_name);
	}
	gnome_desktop_item_unref (ditem);
	return menu_item;
}
/******************************************************************************/
GtkWidget*
menu_browser_add_default_file (gchar *file_name,
							   GtkWidget *menu,
							   MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	if (!IS_MENU_BROWSER (self)) return NULL;

	GtkWidget	*menu_item = NULL;
	GtkWidget	*menu_item_icon = NULL;
	gchar		*icon_name = NULL;
	gchar		*base_name = g_path_get_basename (file_name);

	/*make a menu item for this dir*/
	menu_item = gtk_image_menu_item_new_with_label (base_name);
	
	GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
	if (GTK_IS_LABEL (label)) {
		gtk_label_set_max_width_chars (GTK_LABEL (label), MAX_FILE_NAME_LENGTH);	
		gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
	}
	if (strlen (base_name) > MAX_FILE_NAME_LENGTH) {
		gtk_widget_set_tooltip_text (menu_item, base_name);
	}

	/* bold executable files */
	if (vfs_file_is_executable (file_name)) {
		gchar *markup = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>", base_name);
		gtk_label_set_markup (GTK_LABEL (label), markup);
		g_free (markup);
	}
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
menu_browser_on_dir_middle_click (const gchar *path, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));
	vfs_launch_terminal (path, self->prefs->terminal);
}
/******************************************************************************/
static void
menu_browser_on_file_left_click (const gchar *file_name_and_path, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	/* try launching the desktop file first */
	if (!vfs_launch_desktop_file (file_name_and_path)) {
		vfs_open_file (file_name_and_path, EXEC_OPEN);
	}
}
/******************************************************************************/
static void
menu_browser_on_file_middle_click (const gchar *file_name_and_path, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));
	vfs_edit_file (file_name_and_path, self->prefs->editor);
}
/******************************************************************************/
static gboolean
menu_browser_on_file_right_click (const gchar *file_name_and_path, GtkWidget *menu_item) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	return context_menu_display (file_name_and_path, menu_item);
/*
	utils_show_dialog ("Error: Action not implemented.",
					   "Right click on file action not yet implemented\n",
					   GTK_MESSAGE_INFO);
*/
}
/******************************************************************************/
/* This is for mouse button presses */
#ifndef NEW_MENU_SIGNAL
static gboolean
menu_browser_on_item_button_press (GtkWidget *menu_item,
								   GdkEventButton *event,
								   MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	if (!IS_MENU_BROWSER (self)) return FALSE;

	gchar *path = (gchar*)g_object_get_data (G_OBJECT (menu_item),
											 G_OBJECT_DATA_NAME);

	if (vfs_file_exists (path)) {
		if (event->button == 1) {
			menu_browser_on_file_left_click (path, self);
		}
		else if (event->button == 2) {
			if (vfs_file_is_directory (path)) {
				menu_browser_on_dir_middle_click (path, self);
			}
			else {
				menu_browser_on_file_middle_click (path, self);
			}
		}
		else if (event->button == 3) {
			g_object_set_data (G_OBJECT (menu_item), "menu_browser", self);
			g_object_set_data (G_OBJECT (menu_item), "button_event", event);
			return menu_browser_on_file_right_click (path, menu_item);
		}
	}
	else {
		gchar *tmp = g_strdup_printf ("Error: \"%s\" does not exists.", path);
		utils_show_dialog ("Error: File not found.",
						   tmp,
						   GTK_MESSAGE_ERROR);
		g_free (tmp);
	}
	gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
	return FALSE;
}
#endif
/******************************************************************************/
static gboolean
menu_browser_on_menu_key_press (GtkWidget *menu,
								GdkEventKey *event,
								MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar *path = NULL ;
	GtkMenuShell *menu_shell = GTK_MENU_SHELL (menu);
	GtkWidget *menu_item = menu_shell->active_menu_item;

	if (!IS_MENU_BROWSER (self) ||
		!self->priv->parent_menu_shell->active ||
		!menu_item) {
		return FALSE;
	}

	path = (gchar*)g_object_get_data (G_OBJECT (menu_item),
									  G_OBJECT_DATA_NAME);

	if (vfs_file_exists (path)) {
		if (event->keyval == GDK_KP_Space ||
			event->keyval == GDK_space ||
			event->keyval == GDK_Return ||
			event->keyval == GDK_KP_Enter ||
			event->keyval == GDK_ISO_Enter ||
			event->keyval == GDK_3270_Enter) {

			menu_browser_on_file_left_click (path, self);
			gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
		}
		else if (event->keyval == GDK_F4 ||
				 event->keyval == GDK_KP_F4) {
			if (vfs_file_is_directory (path)) {
				menu_browser_on_dir_middle_click (path, self);
			}
			else {
				menu_browser_on_file_middle_click (path, self);
			}
			gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
		}
		else if ((event->keyval == GDK_Menu) ||
				 (event->keyval == GDK_F10 &&
				 (event->state & gtk_accelerator_get_default_mod_mask ()) == GDK_SHIFT_MASK)){
			g_object_set_data (G_OBJECT (menu_item), "menu_browser", self);
			g_object_set_data (G_OBJECT (menu_item), "button_event", event);
			return menu_browser_on_file_right_click (path, menu_item);
		}
	}
	else {
		gchar *tmp = g_strdup_printf ("Error: \"%s\" does not exists.", path);
		utils_show_dialog ("Error: File not found.",
						   tmp,
						   GTK_MESSAGE_ERROR);
		g_free (tmp);
	}
	return FALSE;
}
/******************************************************************************/
#ifdef NEW_MENU_SIGNAL
static gboolean
menu_browser_on_menu_button_press (GtkWidget *menu,
								   GdkEventButton *event,
								   MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	gchar *path = NULL;
	GtkMenuShell *menu_shell = GTK_MENU_SHELL (menu);
	GtkWidget *menu_item = menu_shell->active_menu_item;
	GdkRectangle area;
	area.x = event->x;
	area.y = event->y;
	area.width = 1;
	area.height = 1; 
	
	if (!IS_MENU_BROWSER (self) ||
		!self->priv->parent_menu_shell->active ||
		!menu_item ||
		!gtk_widget_intersect (menu, &area, NULL)) {
		return FALSE;
	}

	path = (gchar*)g_object_get_data (G_OBJECT (menu_item),
									  G_OBJECT_DATA_NAME);

	if (vfs_file_exists (path)) {
		if (event->button == 1) {
			menu_browser_on_file_left_click (path, self);
			gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
		}
		else if (event->button == 2) {
			if (vfs_file_is_directory (path)) {
				menu_browser_on_dir_middle_click (path, self);
			}
			else {
				menu_browser_on_file_middle_click (path, self);
			}
			gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
		}
		else if (event->button == 3) {
			g_object_set_data (G_OBJECT (menu_item), "menu_browser", self);
			g_object_set_data (G_OBJECT (menu_item), "button_event", event);
			return menu_browser_on_file_right_click (path, menu_item);
		}
	}
	else {
		gchar *tmp = g_strdup_printf ("Error: \"%s\" does not exists.", path);
		utils_show_dialog ("Error: File not found.",
						   tmp,
						   GTK_MESSAGE_ERROR);
		g_free (tmp);
	}
	/*gtk_menu_shell_deactivate (self->priv->parent_menu_shell);*/
	return FALSE;
}
#endif
/******************************************************************************/
#ifndef NEW_MENU_SIGNAL
static void
menu_browser_add_menu_header (GtkWidget *menu,
							  gchar *path,
							  MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));
	GtkWidget *menu_item = NULL;
	GtkWidget *separator = NULL;
	GtkWidget *menu_item_icon = NULL;
	gchar *dir = g_path_get_basename (path);

	menu_item = gtk_image_menu_item_new_with_label (dir);

	GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
	if (GTK_IS_LABEL (label)) {
		gtk_label_set_max_width_chars (GTK_LABEL (label), MAX_FILE_NAME_LENGTH);	
		gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
	}
	if (strlen (dir) > MAX_FILE_NAME_LENGTH) {
		gtk_widget_set_tooltip_text (menu_item, dir);
	}
	g_free(dir);

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
					  "button_press_event",
					  G_CALLBACK (menu_browser_on_item_button_press),
					  self);

	separator = gtk_separator_menu_item_new();

	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
						   separator);
}
#endif
/******************************************************************************/
static void
menu_browser_add_folders (GtkWidget		*menu,
						  gchar			*path,
						  GPtrArray		*dirs,
						  MenuBrowser	*self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	GtkWidget	*menu_item = NULL;
	GtkWidget	*menu_item_icon = NULL;
	GtkWidget	*child_menu = NULL;
	gchar		*file_name_and_path = NULL;
	gchar		*dir = NULL;
	int i;

	for (i=0; i<dirs->len; i++) {
		dir = (gchar *)g_ptr_array_index (dirs, i);
		file_name_and_path = g_strdup_printf ("%s/%s",
											  path,
											  dir);

		/*make a menu item for this dir, limit the length of the text in the menu*/
		menu_item = gtk_image_menu_item_new_with_label (dir);

		GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
		if (GTK_IS_LABEL (label)) {
			gtk_label_set_max_width_chars (GTK_LABEL (label), MAX_FILE_NAME_LENGTH);	
			gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
		}
		if (strlen (dir) > MAX_FILE_NAME_LENGTH) {
			gtk_widget_set_tooltip_text (menu_item, dir);
		}

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
#ifndef NEW_MENU_SIGNAL
		g_signal_connect (G_OBJECT (menu_item),
						  "button_press_event",
						  G_CALLBACK (menu_browser_on_item_button_press),
						  self);
#endif
	}
}
/******************************************************************************/
static void
menu_browser_add_files (GtkWidget	*menu,
						gchar		*path,
						GPtrArray	*files,
						MenuBrowser	*self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

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
#ifndef NEW_MENU_SIGNAL
		g_signal_connect (menu_item,
						  "button_press_event",
						  G_CALLBACK (menu_browser_on_item_button_press),
						  self);
#endif
	}
}
/******************************************************************************/
static void
menu_browser_populate_menu (GtkWidget	*parent_menu_item,
							MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	gchar 		*current_path = NULL;
	gchar		*error = NULL;
	GtkWidget	*menu_item = NULL;
	GtkWidget	*current_menu = NULL;
	GPtrArray	*files = g_ptr_array_new();
	GPtrArray	*dirs = g_ptr_array_new();

	/* empty the menu of its existing contents first */
	menu_browser_clear_menu (parent_menu_item);

	current_path = (gchar*)g_object_get_data (G_OBJECT (parent_menu_item),
											  G_OBJECT_DATA_NAME);

	if (!self->priv->parent_menu_shell->active) return;

	/* get the menu widget to pack all the menu items for this dir into */
	current_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (parent_menu_item));
#ifdef NEW_MENU_SIGNAL
	g_signal_connect (current_menu,
					  "button_press_event",
					  G_CALLBACK (menu_browser_on_menu_button_press),
					  self),
#endif
	g_signal_connect (current_menu,
					  "key_press_event",
					  G_CALLBACK (menu_browser_on_menu_key_press),
					  self),
#ifndef NEW_MENU_SIGNAL
	/* add the dir name and events */
	menu_browser_add_menu_header (current_menu,
								  current_path,
								  self);
#endif
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
}
/******************************************************************************/
void
menu_browser_update (MenuBrowser *self,
					 gchar* path,
					 gchar* label) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	g_return_if_fail (IS_MENU_BROWSER (self));

	GtkWidget *item_label = GTK_BIN (self)->child;

	gtk_label_set_text (GTK_LABEL (item_label), label);

	g_object_set_data (G_OBJECT (self),
					   G_OBJECT_DATA_NAME,
					   g_strdup (path));
}
/******************************************************************************/
static void
menu_browser_class_init (MenuBrowserClass * klass) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

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
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

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
}
/******************************************************************************/
GType
menu_browser_get_type (void) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

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
#ifdef NEW_MENU_SIGNAL
static void
menu_browser_add_main_menu_header (GtkWidget *menu,
								   gchar *path,
								   MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);


	g_return_if_fail (IS_MENU_BROWSER (self));

	GtkWidget *menu_item = NULL;
	GtkWidget *separator = NULL;
	GtkWidget *menu_item_icon = NULL;
	gchar *dir = g_path_get_basename (path);

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu),
							separator);

	menu_item = gtk_image_menu_item_new_with_label (dir);

	GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
	if (GTK_IS_LABEL (label)) {
		gtk_label_set_max_width_chars (GTK_LABEL (label), MAX_FILE_NAME_LENGTH);	
		gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
	}
	if (strlen (dir) > MAX_FILE_NAME_LENGTH) {
		gtk_widget_set_tooltip_text (menu_item, dir);
	}
	g_free(dir);

	menu_item_icon = gtk_image_new_from_icon_name (self->priv->dir_mime_icon_name,
												   GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   menu_item_icon);
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu),
							menu_item);
	g_object_set_data (G_OBJECT (menu_item),
					   G_OBJECT_DATA_NAME,
					   path);
	gtk_widget_show_all (menu);
}
#endif
/******************************************************************************/
static gboolean
menu_browser_activate_main_menu (MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	if (!IS_MENU_BROWSER (self)) return FALSE;

	self->priv->parent_menu_shell =
			GTK_MENU_SHELL (gtk_widget_get_parent (GTK_WIDGET (self)));

	GtkWidget *menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (self));
	menu_browser_populate_menu (GTK_WIDGET (self), self);
#ifdef NEW_MENU_SIGNAL
	gchar *path = g_object_get_data (G_OBJECT (self),
									 G_OBJECT_DATA_NAME);
	menu_browser_add_main_menu_header (menu,
									   path,
									   self);
#endif
	gtk_menu_reposition (GTK_MENU (menu));
	return TRUE;
}
/******************************************************************************/
GtkWidget*
menu_browser_new (const gchar* path,
				  const gchar* label,
				  BrowserPrefs *prefs) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	MenuBrowser *self;
	self = g_object_newv (TYPE_MENU_BROWSER, 0, NULL);

	self->priv->parent_menu_shell = NULL;

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

	g_signal_connect (G_OBJECT (self),
					  "activate",
					  G_CALLBACK (menu_browser_activate_main_menu),
					  self);

	g_signal_connect_swapped (GTK_MENU_ITEM (self),
							  "deselect",
							  G_CALLBACK (menu_browser_clean_up),
							  self);

	return GTK_WIDGET (self);
}
/******************************************************************************/
