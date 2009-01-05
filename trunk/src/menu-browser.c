/*
 * File:				menu-browser.c
 * Created:				September 2005
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

#include <glib/gprintf.h>
#include <gdk/gdkkeysyms.h>

#include "menu-browser.h"
#include "utils.h"
#include "vfs.h"
#include "context-menu.h"
#include "config.h"

/*#define NEW_MENU_SIGNAL*/
/******************************************************************************/
#define MAX_FILE_NAME_LENGTH 30 /* What's a good value here??? */
#define G_OBJECT_DATA_NAME "item_path"
/******************************************************************************/
struct _MenuBrowserPrivate {
	GtkWidget		*menu_item_label;
	GtkMenuShell	*parent_menu_shell;
	Garbage			garbage;
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

	GtkWidget *menu = menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu_item));
	gtk_container_foreach (GTK_CONTAINER (menu), (GtkCallback) gtk_widget_destroy, NULL);
}
/******************************************************************************/
static void
menu_browser_clean_up (MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	menu_browser_clear_menu (GTK_WIDGET (self));

	garbage_empty (&(self->priv->garbage), TRUE);
}
/******************************************************************************/
static void
menu_browser_on_dir_middle_click (const gchar *path, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	gchar **args = g_strv_new (2);
	args[ARG_APP]  = g_strdup (self->prefs->terminal);
	args[ARG_FILE] = g_strdup (path);
	vfs_launch_application ((const gchar*const*)args);
	g_strfreev (args);
}
/******************************************************************************/
static void
menu_browser_on_file_left_click (const gchar *file_name_and_path, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	vfs_file_do_default_action (file_name_and_path);
}
/******************************************************************************/
static void
menu_browser_on_file_middle_click (const gchar *file_name_and_path, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	gchar **args = g_strv_new (2);
	args[ARG_APP]  = g_strdup (self->prefs->editor);
	args[ARG_FILE] = g_strdup (file_name_and_path);
	vfs_launch_application ((const gchar*const*)args);
	g_strfreev (args);
}
/******************************************************************************/
static gboolean
menu_browser_on_file_right_click (const gchar *file_name_and_path, GtkWidget *menu_item) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);
#ifdef ENABLE_CONTEXT_MENU
	return context_menu_display (file_name_and_path, menu_item);
#else
	utils_show_dialog (_("Error"),
					   _("Right-click on file action not yet implemented."),
					   GTK_MESSAGE_INFO);
	return TRUE;
#endif
}
/******************************************************************************/
/* This is for mouse button presses */
#ifndef NEW_MENU_SIGNAL
static gboolean
menu_browser_on_item_button_press (GtkWidget *menu_item,
								   GdkEventButton *event,
								   MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_val_if_fail (IS_MENU_BROWSER (self), FALSE);

	gchar *path = (gchar*)g_object_get_data (G_OBJECT (menu_item), G_OBJECT_DATA_NAME);

	if (!vfs_file_exists (path)) {
		gchar *tmp = g_strdup_printf (_("Error: The file \"%s\" does not exists."), path);
		utils_show_dialog (_("Error"), tmp, GTK_MESSAGE_ERROR);
		g_free (tmp);
		return FALSE;
	}

	if (event->button == 1) {
		menu_browser_on_file_left_click (path, self);
	}
	else if (event->button == 2) {
		vfs_file_is_directory (path) ?
			menu_browser_on_dir_middle_click (path, self) :
			menu_browser_on_file_middle_click (path, self);
	}
	else if (event->button == 3) {
		g_object_set_data (G_OBJECT (menu_item), "menu_browser", self);
		g_object_set_data (G_OBJECT (menu_item), "button_event", event);
		return menu_browser_on_file_right_click (path, menu_item);
	}

	gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
	return FALSE;
}
#endif
/******************************************************************************/
static gboolean
menu_browser_on_menu_key_press (GtkWidget *menu, GdkEventKey *event, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_val_if_fail (IS_MENU_BROWSER (self), FALSE);

	GtkMenuShell *menu_shell = GTK_MENU_SHELL (menu);
	GtkWidget *menu_item = menu_shell->active_menu_item;

	/* huh!? */
	if (!self->priv->parent_menu_shell->active || !menu_item) {
		return FALSE;
	}

	gchar *path = (gchar*)g_object_get_data (G_OBJECT (menu_item), G_OBJECT_DATA_NAME);

	if (!vfs_file_exists (path)) {
		gchar *tmp = g_strdup_printf (_("Error: The file \"%s\" does not exists."), path);
		utils_show_dialog (_("Error"), tmp, GTK_MESSAGE_ERROR);
		g_free (tmp);
		return FALSE;
	}

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

		vfs_file_is_directory (path) ?
			menu_browser_on_dir_middle_click (path, self) :
			menu_browser_on_file_middle_click (path, self);
		gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
	}
	else if ((event->keyval == GDK_Menu) ||
			 (event->keyval == GDK_F10 &&
			 (event->state & gtk_accelerator_get_default_mod_mask ()) == GDK_SHIFT_MASK)){
		g_object_set_data (G_OBJECT (menu_item), "menu_browser", self);
		g_object_set_data (G_OBJECT (menu_item), "button_event", event);
		return menu_browser_on_file_right_click (path, menu_item);
	}

	return FALSE;
}
/******************************************************************************/
#ifdef NEW_MENU_SIGNAL
static gboolean
menu_browser_on_menu_button_press (GtkWidget *menu, GdkEventButton *event, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_val_if_fail (IS_MENU_BROWSER (self), FALSE);

	gchar *path = NULL;
	GtkMenuShell *menu_shell = GTK_MENU_SHELL (menu);
	GtkWidget *menu_item = menu_shell->active_menu_item;
	GdkRectangle area;
	area.x = event->x;
	area.y = event->y;
	area.width = 1;
	area.height = 1; 
	
	if (!self->priv->parent_menu_shell->active || !menu_item ||
		!gtk_widget_intersect (menu, &area, NULL)) {
		return FALSE;
	}

	path = (gchar*)g_object_get_data (G_OBJECT (menu_item), G_OBJECT_DATA_NAME);

	if (!vfs_file_exists (path)) {
		gchar *tmp = g_strdup_printf (_("Error: The file \"%s\" does not exists."), path);
		utils_show_dialog (_("Error"), tmp, GTK_MESSAGE_ERROR);
		g_free (tmp);
		return FALSE;
	}

	if (event->button == 1) {
		menu_browser_on_file_left_click (path, self);
		gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
	}
	else if (event->button == 2) {
		vfs_file_is_directory (path) ?
			menu_browser_on_dir_middle_click (path, self) :
			menu_browser_on_file_middle_click (path, self);
		gtk_menu_shell_deactivate (self->priv->parent_menu_shell);
	}
	else if (event->button == 3) {
		g_object_set_data (G_OBJECT (menu_item), "menu_browser", self);
		g_object_set_data (G_OBJECT (menu_item), "button_event", event);
		return menu_browser_on_file_right_click (path, menu_item);
	}

	/*gtk_menu_shell_deactivate (self->priv->parent_menu_shell);*/
	return FALSE;
}
#endif
/******************************************************************************/
#ifndef NEW_MENU_SIGNAL
static void
menu_browser_add_menu_header (GtkWidget *menu, gchar *path, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	gchar *dir = g_path_get_basename (path);
	GtkWidget *menu_item = gtk_image_menu_item_new_with_label (dir);

	GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
	if (GTK_IS_LABEL (label)) {
		gtk_label_set_max_width_chars (GTK_LABEL (label), MAX_FILE_NAME_LENGTH);	
		gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
	}
	if (strlen (dir) > MAX_FILE_NAME_LENGTH) {
		gtk_widget_set_tooltip_text (menu_item, dir);
	}
	g_free(dir);

	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   vfs_get_icon_for_file (path));

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	g_object_set_data (G_OBJECT (menu_item), G_OBJECT_DATA_NAME, path);

	g_signal_connect (G_OBJECT (menu_item),
					  "button_release_event",
					  G_CALLBACK (menu_browser_on_item_button_press),
					  self);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
						   gtk_separator_menu_item_new());
}
#endif
/******************************************************************************/
static void
menu_browser_add_folders (GtkWidget *menu, GPtrArray *dirs, MenuBrowser	*self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	int i;

	for (i=0; i<dirs->len; i++) {
		VfsFileInfo *vfs_file_info = (VfsFileInfo*)g_ptr_array_index (dirs, i);

		/*make a menu item for this dir, limit the length of the text in the menu*/
		/* WTH is going on here! Because we're using a mnemonic, we have to
		 * replace all _ with __ for the name to appear correctly */
		gchar *tmp_str = utils_escape_str (vfs_file_info->display_name, "_", "__");
		gchar *escaped_str = g_strdup_printf ("_%s", tmp_str);
		GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (escaped_str);
		g_free (escaped_str);
		g_free (tmp_str);

		GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
		if (GTK_IS_LABEL (label)) {
			gtk_label_set_max_width_chars (GTK_LABEL (label), MAX_FILE_NAME_LENGTH);	
			gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
		}
		if (strlen (vfs_file_info->display_name) > MAX_FILE_NAME_LENGTH) {
			gtk_widget_set_tooltip_text (menu_item, vfs_file_info->display_name);
		}

		/*get the icon widget based on the returned icon name (always the same icon, can speed up here)*/
		/*stick the icon in the menu item, the menu item in the menu and show it all*/
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
									   vfs_file_info->icon);

		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		/*make the sub menu to show all the files in this dir*/
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), gtk_menu_new ());

		garbage_add_item (self->priv->garbage, vfs_file_info->file_name);
		garbage_add_item (self->priv->garbage, vfs_file_info->display_name);

		g_object_set_data (G_OBJECT (menu_item),
						   G_OBJECT_DATA_NAME,
						   vfs_file_info->file_name);

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
menu_browser_add_files (GtkWidget *menu, GPtrArray *files, MenuBrowser *self) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	int i;

	for (i=0; i<files->len; i++) {
		VfsFileInfo *vfs_file_info = (VfsFileInfo*)g_ptr_array_index (files, i);

		/*make a menu item for this dir*/
		gchar *tmp_str = utils_escape_str (vfs_file_info->display_name, "_", "__");
		gchar *escaped_str = g_strdup_printf ("_%s", tmp_str);
		GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (escaped_str);
		g_free (escaped_str);
		g_free (tmp_str);
	
		/* set the ellipsizig and tooltip */
		GtkWidget *label = gtk_bin_get_child (GTK_BIN (menu_item));
		if (GTK_IS_LABEL (label)) {
			gtk_label_set_max_width_chars (GTK_LABEL (label), MAX_FILE_NAME_LENGTH);	
			gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
		}
		if (strlen (vfs_file_info->display_name) > MAX_FILE_NAME_LENGTH) {
			gtk_widget_set_tooltip_text (menu_item, vfs_file_info->display_name);
		}

		/* bold executable files */
		if (vfs_file_info->is_executable) {
			gchar *markup = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>",
													 vfs_file_info->display_name);
			gtk_label_set_markup (GTK_LABEL (label), markup);
			g_free (markup);
		}

		/*stick the icon in the menu item, the menu item in the menu and show it all*/
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
									   vfs_file_info->icon);

		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);

		garbage_add_item (self->priv->garbage, vfs_file_info->file_name);
		garbage_add_item (self->priv->garbage, vfs_file_info->display_name);

		g_object_set_data (G_OBJECT (menu_item),
						   G_OBJECT_DATA_NAME,
						   vfs_file_info->file_name);
#ifndef NEW_MENU_SIGNAL
		g_signal_connect (menu_item,
						  "button_release_event",
						  G_CALLBACK (menu_browser_on_item_button_press),
						  self);
#endif
	}
}
/******************************************************************************/
static void
menu_browser_populate_menu (GtkWidget *parent_menu_item, MenuBrowser *self) {
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

	current_path = (gchar*)g_object_get_data (G_OBJECT (parent_menu_item), G_OBJECT_DATA_NAME);

	/* huh!? */
	if (!self->priv->parent_menu_shell->active) return;

	/* get the menu widget to pack all the menu items for this dir into */
	current_menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (parent_menu_item));

	g_signal_connect (current_menu,
					  "key_press_event",
					  G_CALLBACK (menu_browser_on_menu_key_press),
					  self),

#ifdef NEW_MENU_SIGNAL
	g_signal_connect (current_menu,
					  "button_release_event",
					  G_CALLBACK (menu_browser_on_menu_button_press),
					  self),
#else 
	/* add the dir name and events */
	menu_browser_add_menu_header (current_menu, current_path, self);
#endif

	/* read the contents of the dir. */
	error = vfs_get_dir_listings (files,
								  dirs,
								  self->prefs->show_hidden,
								  current_path);

	/* add the folders*/
	menu_browser_add_folders (current_menu, dirs, self);

	if ((dirs->len > 0) & (files->len > 0)) {
		/*add a separator between dirs and files*/
		gtk_menu_shell_append (GTK_MENU_SHELL (current_menu),
							   gtk_separator_menu_item_new());
	}

	/* add the files */
	menu_browser_add_files (current_menu, files, self);

	/* add any error or other messages */
	if (error != NULL) {
		menu_item = gtk_menu_item_new_with_label (error);
		g_free (error);
		gtk_menu_shell_append (GTK_MENU_SHELL (current_menu), menu_item);
		gtk_widget_set_sensitive (GTK_WIDGET (menu_item), FALSE);
	} /* the folder was empty */
	else if ((dirs->len == 0) & (files->len == 0)) {
		menu_item = gtk_menu_item_new_with_label (_("(Empty)"));
		gtk_menu_shell_append (GTK_MENU_SHELL (current_menu), menu_item);
		gtk_widget_set_sensitive (GTK_WIDGET (menu_item), FALSE);
	}
	gtk_widget_show_all (current_menu);

	/* clean up */
	g_ptr_array_foreach (dirs, (GFunc)g_free, NULL);
	g_ptr_array_foreach (files, (GFunc)g_free, NULL);
	g_ptr_array_free (dirs, TRUE);
	g_ptr_array_free (files, TRUE);
}
/******************************************************************************/
void
menu_browser_update (MenuBrowser *self, const gchar *path, const gchar *label) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	g_return_if_fail (IS_MENU_BROWSER (self));

	GtkWidget *item_label = GTK_BIN (self)->child;
	gtk_label_set_text (GTK_LABEL (item_label), label);

	g_free (g_object_get_data (G_OBJECT (self), G_OBJECT_DATA_NAME));
	g_object_set_data (G_OBJECT (self), G_OBJECT_DATA_NAME, g_strdup (path));
}
/******************************************************************************/
static void
menu_browser_class_init (MenuBrowserClass *klass) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	menu_browser_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (MenuBrowserPrivate));
	G_OBJECT_CLASS (klass)->dispose = menu_browser_dispose;
}
/******************************************************************************/
static void
menu_browser_init (MenuBrowser *self) {
	
	g_return_if_fail (IS_MENU_BROWSER (self));

	self->priv = MENU_BROWSER_GET_PRIVATE (self);
}
/******************************************************************************/
static void
menu_browser_dispose (GObject *obj) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	MenuBrowser *self = MENU_BROWSER (obj);
/*	MenuBrowserClass *klass = MENU_BROWSER_CLASS (g_type_class_peek (TYPE_MENU_BROWSER));*/

	(self->priv->menu_item_label == NULL ? NULL : (self->priv->menu_item_label = (gtk_widget_destroy (self->priv->menu_item_label), NULL)));
	(self->priv->garbage == NULL ? NULL : garbage_empty (&(self->priv->garbage), FALSE));

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
menu_browser_add_main_menu_header (GtkWidget *menu, gchar *path, MenuBrowser *self) {
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

	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
								   vfs_get_icon_for_file (path));

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

	g_return_val_if_fail (IS_MENU_BROWSER (self), FALSE);

	self->priv->parent_menu_shell =
			GTK_MENU_SHELL (gtk_widget_get_parent (GTK_WIDGET (self)));

	GtkWidget *menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (self));

	/*GdkEvent *event = gdk_event_get ();*/
	/*if (event->type == GDK_2BUTTON_PRESS) {*/
		/*gchar *path = g_object_get_data (G_OBJECT (self), G_OBJECT_DATA_NAME);*/
		/*vfs_file_do_default_action (path);*/
		/*gdk_event_free (event);*/
		/*gtk_menu_popdown (GTK_MENU (menu));*/
		/*gtk_menu_shell_deactivate (GTK_MENU_SHELL (self->priv->parent_menu_shell));*/
		/*return TRUE;*/
	/*}*/

	/* this is here because of the solution to the focus problem caused by the
	 * popped context menu */
	gtk_widget_set_sensitive (GTK_WIDGET (menu), TRUE);

	menu_browser_populate_menu (GTK_WIDGET (self), self);

#ifdef NEW_MENU_SIGNAL
	gchar *path = g_object_get_data (G_OBJECT (self), G_OBJECT_DATA_NAME);
	menu_browser_add_main_menu_header (menu, path, self);
#endif
	gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), FALSE);
	gtk_menu_reposition (GTK_MENU (menu));
	return TRUE;
}
/******************************************************************************/
GtkWidget*
menu_browser_new (const gchar* path,
				  const gchar* label,
				  BrowserPrefs *prefs) {
	if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

	MenuBrowser *self = g_object_newv (TYPE_MENU_BROWSER, 0, NULL);

	self->priv->parent_menu_shell = NULL;

	if (prefs == NULL) {
		/* get the default config if none passed. */
	/*	prefs = preferences_browser_get_default ();*/
	}
	self->prefs = prefs;

	garbage_init(&(self->priv->garbage));

	GtkWidget *item_label = gtk_label_new (label);
	gtk_misc_set_alignment (GTK_MISC (item_label), 0.0, 0.5);
	gtk_container_add (GTK_CONTAINER (self), item_label);
	self->priv->menu_item_label = item_label;

	/*make the main menu*/
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (self), gtk_menu_new());

	g_object_set_data (G_OBJECT (self), G_OBJECT_DATA_NAME, g_strdup (path));

	g_signal_connect_after (G_OBJECT (self),
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
