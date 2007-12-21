/*
 * File:				panel-menu-bar.c
 * Created:				October 2007
 * Created by:			Axel von Bertoldi
 * Last Modified:		October 2007
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
#include "panel-menu-bar.h"
#include "preferences.h"
#include "menu-browser.h"

#define PANEL_MENU_BAR_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_PANEL_MENU_BAR, PanelMenuBarPrivate))
#define TOOLTIP_TEXT		"Browse and open files in your home directory"
#define ICON_SIZE			24 /* 22x22 pixels looks about right... */

/******************************************************************************/
struct _PanelMenuBarPrivate {
	PanelApplet			*applet;
	GPtrArray			*file_browsers;
	AppletPreferences	*prefs;
	PanelAppletOrient	orientation;
	gint				panel_size;
	gboolean			text_vertical;
};
/******************************************************************************/
enum {
	PANEL_MENU_BAR_DUMMY_PROPERTY
};

enum {
	PROP_0,
	PROP_ORIENTATION,
};
/******************************************************************************/
static gpointer panel_menu_bar_parent_class = NULL;
static void panel_menu_bar_dispose (GObject * obj);
/******************************************************************************/
/******************************************************************************/
void
panel_menu_bar_edit_prefs (PanelMenuBar *self) {
	AppletPreferences *prefs = self->priv->prefs;
	applet_preferences_make_dialog (prefs);
}
/******************************************************************************/
static void	/* Originally from the Main Menu Bar Applet */
panel_menu_bar_change_orient (PanelApplet		*applet,
							  PanelAppletOrient	orientation,
							  PanelMenuBar		*self) {
	gint				i;
	gboolean			text_vertical = FALSE;
	gdouble				text_angle;
	gfloat				text_xalign;
	gfloat				text_yalign;
	GtkWidget			*menu_browser = NULL;
	GtkRequisition		requisition;
	GtkPackDirection	pack_direction;

	/* update the panel's orientation */
	self->priv->orientation = orientation;

	/* set the default (horizontal panel) pack and text direction */
	pack_direction = GTK_PACK_DIRECTION_LTR;
	text_angle  = 0.0;
	text_xalign = 0.0;
	text_yalign = 0.5;

	/* iterate through each menu_item in the menu bar and check it's width */
	for (i=0; i < self->priv->file_browsers->len; i++) {
		menu_browser = (GtkWidget *)(g_ptr_array_index (self->priv->file_browsers, i));
		gtk_widget_size_request (menu_browser,
								 &requisition);
		/* if any of the menu_items are wider that the panel, set the text to be rotated */
		if ((!self->priv->text_vertical & (requisition.width  > self->priv->panel_size)) ||
			( self->priv->text_vertical & (requisition.height > self->priv->panel_size))) {
			text_vertical = TRUE;
		}
	}
	self->priv->text_vertical = text_vertical;
	/* set the text and pack direction and */
	switch (orientation) {
		case PANEL_APPLET_ORIENT_UP:
		case PANEL_APPLET_ORIENT_DOWN:
			self->priv->text_vertical = FALSE;
			break;
		case PANEL_APPLET_ORIENT_RIGHT:
			pack_direction = GTK_PACK_DIRECTION_TTB;
			if (text_vertical) {
				pack_direction = GTK_PACK_DIRECTION_BTT;
				text_angle = 90.0;
			}
				text_xalign = 0.5;
				text_yalign = 0.0;
			break;
		case PANEL_APPLET_ORIENT_LEFT:
			pack_direction = GTK_PACK_DIRECTION_TTB;
			if (text_vertical) {
				text_angle = 270.0;
			}
				text_xalign = 0.5;
				text_yalign = 0.0;
			break;
		default:
			g_assert_not_reached ();
			break;
	}
	/* update the menu_bar pack direction */	
	gtk_menu_bar_set_pack_direction (GTK_MENU_BAR (self), pack_direction);
	gtk_menu_bar_set_child_pack_direction (GTK_MENU_BAR (self), pack_direction);

	/* update the menu_item text direction */
	for (i=0; i < self->priv->file_browsers->len; i++) {
		menu_browser = (GtkWidget *)(g_ptr_array_index (self->priv->file_browsers, i));
		GtkWidget *label = GTK_BIN (menu_browser)->child;
		gtk_label_set_angle (GTK_LABEL (label), text_angle);
		gtk_misc_set_alignment (GTK_MISC (label), text_xalign, text_yalign);
	}
	g_printf ("Changing panel orientation : %d\n", orientation);
	return;
}
/******************************************************************************/
static void	/* Taken from the Main Menu Bar Applet */
panel_menu_bar_size_allocate (GtkWidget *widget,
							  GtkAllocation *allocation) {
	PanelMenuBar *self = (PanelMenuBar *)widget;

	/* WTF!? */
	allocation->x = 0;
	allocation->y = 0;
	allocation->height += 2;
	allocation->width += 2;

	/* update the panel's width */
	self->priv->panel_size = allocation->height;
	if (self->priv->orientation == PANEL_APPLET_ORIENT_RIGHT || 
		self->priv->orientation == PANEL_APPLET_ORIENT_LEFT) {
		self->priv->panel_size = allocation->width;
	}

	GTK_WIDGET_CLASS (panel_menu_bar_parent_class)->size_allocate (widget, allocation);

	PanelAppletOrient orientation = panel_applet_get_orient (PANEL_APPLET (self->priv->applet));
	panel_menu_bar_change_orient (self->priv->applet,
								  orientation,
								  self);

	g_printf ("Resizing panel : ");
	g_printf ("x %d, "
			  "y %d, "
			  "height %d, "
			  "width %d\n",
			  allocation->x,
			  allocation->y,
			  allocation->height,
			  allocation->width);
	return;
}
/******************************************************************************/
static void	/* Taken from the Trash Applet */
panel_menu_bar_change_background (PanelApplet				*applet,
                          		  PanelAppletBackgroundType	 type,
								  GdkColor                  *color,
								  GdkPixmap                 *pixmap,
								  PanelMenuBar				*self) {
	GtkRcStyle *rc_style;
	GtkStyle *style;
	
	/* reset style */
	gtk_widget_set_style (GTK_WIDGET (self), NULL);
	rc_style = gtk_rc_style_new ();
	gtk_widget_modify_style (GTK_WIDGET (self), rc_style);
	gtk_rc_style_unref (rc_style);	
	
	switch (type) {
		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg (GTK_WIDGET (self),
					      GTK_STATE_NORMAL, 
					      color);
			break;
		
		case PANEL_PIXMAP_BACKGROUND:
			style = gtk_style_copy (GTK_WIDGET (self)->style);
			if (style->bg_pixmap[GTK_STATE_NORMAL] != NULL)
				g_object_unref (style->bg_pixmap[GTK_STATE_NORMAL]);

			style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
			gtk_widget_set_style (GTK_WIDGET (self), style);
			g_object_unref (style);
			break;
		
		case PANEL_NO_BACKGROUND:
		default:
			break;
	}
	g_printf ("Changing Applet Background\n");
	return;
}
/******************************************************************************/
static void
panel_menu_bar_update_image (PanelMenuBar *self) {
	GtkWidget *image = NULL;
	AppletPreferences *a_prefs = self->priv->prefs;
	GtkWidget *menu_browser = (GtkWidget *)(g_ptr_array_index (self->priv->file_browsers, 0));

	image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (menu_browser));

	if (image) {
		gtk_widget_destroy (image);
	}

	if (a_prefs->menu_bar_prefs->show_icon) {
		GtkWidget *icon = utils_get_scaled_image_from_file (a_prefs->menu_bar_prefs->icon,
															ICON_SIZE);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_browser),
															icon);
	}
	return;
}
/******************************************************************************/
static void
panel_menu_bar_update_entry (PanelMenuBar *self,
							 PrefsChangedSignalData *signal_data) {
	GtkWidget *menu_browser = (GtkWidget *)(g_ptr_array_index (self->priv->file_browsers, signal_data->instance));

	menu_browser_update (MENU_BROWSER (menu_browser),
						 signal_data->path,
						 signal_data->label);
	return;
}
/******************************************************************************/
static void
panel_menu_bar_move_entry (PanelMenuBar *self,
						   PrefsChangedSignalData *signal_data) {
	int new_pos;
	GtkWidget *menu_browser = NULL;
	GtkWidget *menu_browser_affected = NULL;

	if (signal_data->signal_id == PREFS_SIGNAL_DIR_MOVE_UP) {
		new_pos = signal_data->instance - 1;
	}
	else if (signal_data->signal_id == PREFS_SIGNAL_DIR_MOVE_DOWN) {
		new_pos = signal_data->instance + 1;
	}
	else {
		g_printf("shitzer\n");
		return;
	}

	if (new_pos < 0) {
		g_printf("shitzer\n");
		return;
	}

	menu_browser = (GtkWidget *)(g_ptr_array_index (self->priv->file_browsers, signal_data->instance));
	menu_browser_affected = (GtkWidget *)(g_ptr_array_index (self->priv->file_browsers, new_pos));

	if (menu_browser != NULL && menu_browser_affected != NULL) {
		g_object_ref (G_OBJECT (menu_browser));
		gtk_container_remove (GTK_CONTAINER (self), menu_browser);
		gtk_menu_shell_insert (GTK_MENU_SHELL (self),
							   menu_browser,
							   new_pos);
		g_object_unref (G_OBJECT (menu_browser));
		self->priv->file_browsers->pdata[new_pos] = menu_browser;
		self->priv->file_browsers->pdata[signal_data->instance] = menu_browser_affected;
	}
	else g_printf("shitzer\n");

	return;
}
/******************************************************************************/
static void
panel_menu_bar_remove_entry (PanelMenuBar *self,
							 gint instance) {
	GtkWidget *menu_browser = (GtkWidget *)(g_ptr_array_index (self->priv->file_browsers, instance));

	if (menu_browser != NULL) {
		gtk_container_remove (GTK_CONTAINER (self), menu_browser);
		g_ptr_array_remove_index (self->priv->file_browsers, instance);
	}
	else g_printf("shitzer\n");

	return;
}
/******************************************************************************/
static void
panel_menu_bar_add_entry (PanelMenuBar *self,
						  gchar *label,
						  gchar *path) {

	GtkWidget *menu_browser = menu_browser_new (path,
												label,
												self->priv->prefs->menu_bar_prefs->browser_prefs);

	/* add it to the list and to the menu bar*/
	g_ptr_array_add (self->priv->file_browsers, menu_browser);

	gtk_menu_shell_append (GTK_MENU_SHELL (self),
						   menu_browser);

	gtk_widget_show_all (menu_browser);
	return;
}
/******************************************************************************/
static void
panel_menu_bar_on_preferences_changed (AppletPreferences *a_prefs,
									   PrefsChangedSignalData *signal_data,
									   gpointer data) {
	PanelMenuBar *self = (PanelMenuBar *)data;

	switch (signal_data->signal_id) {
		case PREFS_SIGNAL_TERMINAL :
		case PREFS_SIGNAL_SHOW_HIDDEN :
			break;
		case PREFS_SIGNAL_SHOW_ICON :
		case PREFS_SIGNAL_ICON_CHANGED :
			panel_menu_bar_update_image (self);
			break;
		case PREFS_SIGNAL_DIR_MOVE_UP :
		case PREFS_SIGNAL_DIR_MOVE_DOWN :
			panel_menu_bar_move_entry (self, signal_data);
			break;
		case PREFS_SIGNAL_DIR_CHANGED :
			panel_menu_bar_update_entry (self, signal_data);
			break;
		case PREFS_SIGNAL_DIR_ADD :
			panel_menu_bar_add_entry (self, signal_data->label, signal_data->path);
			break;
		case PREFS_SIGNAL_DIR_DEL :
			panel_menu_bar_remove_entry (self, signal_data->instance);
			break;
	}
	g_printf ("caught the signal: %d, ", signal_data->signal_id);
	g_printf ("instance %d, label: %s, path %s\n",
			  signal_data->instance,
			  signal_data->label,
			  signal_data->path);

	g_free (signal_data->label);
	g_free (signal_data->path);
	g_free (signal_data);

	return;
}
/******************************************************************************/
PanelMenuBar*
panel_menu_bar_new (PanelApplet* applet) {
	PanelMenuBar *self;

	g_return_val_if_fail (applet == NULL || PANEL_IS_APPLET (applet), NULL);
	self = g_object_newv (TYPE_PANEL_MENU_BAR, 0, NULL);

/*============================================================================*/
	self->priv->file_browsers = g_ptr_array_new ();

	self->priv->panel_size= 0;
	self->priv->applet = applet;

	/* setup callbacks to handle changes in the panel */
	g_signal_connect (G_OBJECT (applet),
					  "change_background",
					  G_CALLBACK (panel_menu_bar_change_background),
					  self);
	g_signal_connect (G_OBJECT (applet),
					  "change_orient",
					  G_CALLBACK (panel_menu_bar_change_orient),
					  self);

    gtk_widget_set_tooltip_text (GTK_WIDGET (self), _(TOOLTIP_TEXT));
	
	/* get the applet configuration */
	self->priv->prefs = applet_preferences_new (applet);

	MenuBarPrefs *mb_prefs = self->priv->prefs->menu_bar_prefs;
	g_signal_connect (G_OBJECT (self->priv->prefs),
					  "prefs_changed",
					  G_CALLBACK (panel_menu_bar_on_preferences_changed), 
					  self);

	/* for each path in the config, make a browser object */
	GSList *tmp_dir   = mb_prefs->dirs;
	GSList *tmp_label = mb_prefs->labels;
	while (tmp_label && tmp_dir) {
		panel_menu_bar_add_entry (self,
								  (gchar*)tmp_label->data,
								  (gchar*)tmp_dir->data);
		tmp_dir   = tmp_dir->next;
		tmp_label = tmp_label->next;
	}

    /* add the image to the menu item */
	panel_menu_bar_update_image (self);

	gtk_container_add (GTK_CONTAINER (applet), GTK_WIDGET (self));	

	/* make sure the text orientation is correct on startup */
	self->priv->text_vertical = FALSE;
/*============================================================================*/
	return self;
}
/******************************************************************************/
static void panel_menu_bar_class_init (PanelMenuBarClass * klass) {
	panel_menu_bar_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (PanelMenuBarPrivate));
	G_OBJECT_CLASS (klass)->dispose = panel_menu_bar_dispose;

/*	GObjectClass	 *gobject_class = (GObjectClass   *) klass;*/
	GtkWidgetClass	 *widget_class  = (GtkWidgetClass *) klass;
	widget_class->size_allocate = panel_menu_bar_size_allocate;

	gtk_rc_parse_string ("style \"panel-menubar-style\"\n"
						 "{\n"
						 "  GtkMenuBar::shadow-type = none\n"
						 "  GtkMenuBar::internal-padding = 0\n"
						 "}\n"
						 "class \"PanelMenuBar\" style \"panel-menubar-style\"");
}
/******************************************************************************/

static void
panel_menu_bar_init (PanelMenuBar * self) {
	self->priv = PANEL_MENU_BAR_GET_PRIVATE (self);
	return;
}
/******************************************************************************/
static void
panel_menu_bar_dispose (GObject *obj) {
	PanelMenuBar *self;
	PanelMenuBarClass *klass;
/*	GObjectClass * parent_class;*/

	self = PANEL_MENU_BAR (obj);
	(self->priv->applet == NULL ? NULL : (self->priv->applet = (g_object_unref (self->priv->applet), NULL)));
	(self->priv->file_browsers == NULL ? NULL : (self->priv->file_browsers = (gpointer)(g_ptr_array_free (self->priv->file_browsers, TRUE))));
	klass = PANEL_MENU_BAR_CLASS (g_type_class_peek (TYPE_PANEL_MENU_BAR));
	G_OBJECT_CLASS (panel_menu_bar_parent_class)->dispose (obj);
	return;
}
/******************************************************************************/
GType
panel_menu_bar_get_type (void) {
	static GType panel_menu_bar_type_id = 0;
	if (G_UNLIKELY (panel_menu_bar_type_id == 0)) {
		static const GTypeInfo g_define_type_info = {sizeof (PanelMenuBarClass),
													 (GBaseInitFunc) NULL,
													 (GBaseFinalizeFunc) NULL,
													 (GClassInitFunc) panel_menu_bar_class_init,
													 (GClassFinalizeFunc) NULL,
													 NULL,
													 sizeof (PanelMenuBar),
													 0,
													 (GInstanceInitFunc) panel_menu_bar_init };

		panel_menu_bar_type_id = g_type_register_static (GTK_TYPE_MENU_BAR,
														 "PanelMenuBar",
														 &g_define_type_info,
														 0);
	}
	return panel_menu_bar_type_id;
}
/******************************************************************************/
