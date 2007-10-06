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

#include <glib-object.h>
#include "panel-menu-bar.h"
#include "preferences.h"

#define PANEL_MENU_BAR_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_PANEL_MENU_BAR, PanelMenuBarPrivate))
#define TOOLTIP_TEXT		"Browse and open files in you home directory"
#define ICON_SIZE			22 /* 22x22 pixels looks about right... */

/******************************************************************************/
struct _PanelMenuBarPrivate {
	PanelApplet			*applet;
	GPtrArray			*file_browsers;
	AppletPreferences	*prefs;
	PanelAppletOrient	orientation;
	gint				panel_size;
	gboolean			text_vertical;
	GtkTooltips			*tooltip;
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
static void
file_browser_applet_hide_tooltip (GtkWidget *widget,
								  GtkTooltips *tooltip) {
	gtk_tooltips_disable (tooltip);
	/* reposition the menu too. */
	GtkWidget *menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));
	gtk_menu_reposition (GTK_MENU (menu));
	return;
}
/******************************************************************************/
static void
file_browser_applet_show_tooltip (GtkWidget *widget,
								  GtkTooltips *tooltip) {
	gtk_tooltips_enable (tooltip);
	return;
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
	MenuFileBrowser		*tmp_file_browser = NULL;
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
		tmp_file_browser = (MenuFileBrowser *)(g_ptr_array_index (self->priv->file_browsers, i));
		gtk_widget_size_request (GTK_WIDGET (tmp_file_browser->menu_item),
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
		tmp_file_browser = (MenuFileBrowser *)(g_ptr_array_index (self->priv->file_browsers, i));
		GtkWidget *label = GTK_BIN (tmp_file_browser->menu_item)->child;
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
PanelMenuBar* panel_menu_bar_new (PanelApplet* applet) {
	PanelMenuBar * self;

	g_return_val_if_fail (applet == NULL || PANEL_IS_APPLET (applet), NULL);
	self = g_object_newv (TYPE_PANEL_MENU_BAR, 0, NULL);

/*============================================================================*/
	MenuFileBrowser *tmp_file_browser = NULL;
	self->priv->file_browsers = g_ptr_array_new ();
	gint i;

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

	/* make the tooltip */
	self->priv->tooltip = gtk_tooltips_new ();
	gtk_tooltips_set_tip (self->priv->tooltip,
						  GTK_WIDGET (self),
						  _(TOOLTIP_TEXT),
						  NULL);
	
	/* get the applet configuration */
	self->priv->prefs = preferences_get ();

	/* for each path in the config, make a browser object */
	for (i=0; i < self->priv->prefs->dirs->len; i++)
	{	/* make it */
		tmp_file_browser = menu_browser_new ((gchar*)(g_ptr_array_index (self->priv->prefs->labels, i)),
				  								  (gchar*)(g_ptr_array_index (self->priv->prefs->dirs, i)),
												  self->priv->prefs->browser_prefs);
		/* add it to the list and to the menu bar*/
		g_ptr_array_add (self->priv->file_browsers, tmp_file_browser);

		gtk_menu_shell_append (GTK_MENU_SHELL (self),
							   tmp_file_browser->menu_item);

		/* attach the signal to hide the tooltips when selected*/
		g_signal_connect (G_OBJECT (tmp_file_browser->menu_item),
						  "activate",
						  G_CALLBACK (file_browser_applet_hide_tooltip),
						  self->priv->tooltip);
	}

    /* add the image to the menu item */
	if (self->priv->prefs->show_icon)
	{
		tmp_file_browser = (MenuFileBrowser *)(g_ptr_array_index (self->priv->file_browsers, 0));

		GdkPixbuf *orig   = gdk_pixbuf_new_from_file (self->priv->prefs->icon ,NULL);
		/* FIXME look at gnome menu for icon size*/
		GdkPixbuf *scaled = gdk_pixbuf_scale_simple (orig,
                                          			 ICON_SIZE,
										  			 ICON_SIZE,
										  			 GDK_INTERP_HYPER);
		GtkWidget *icon = gtk_image_new_from_pixbuf (scaled);
	    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (tmp_file_browser->menu_item),
    	                                					icon);
		g_object_unref (orig);
		g_object_unref (scaled);
	}

	/* attach the signal to show the tooltip again when deselected*/
    g_signal_connect (G_OBJECT (self),
                	  "deactivate",
					  G_CALLBACK (file_browser_applet_show_tooltip),
					  self->priv->tooltip);

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

static void panel_menu_bar_init (PanelMenuBar * self) {
	self->priv = PANEL_MENU_BAR_GET_PRIVATE (self);
}
/******************************************************************************/
static void panel_menu_bar_dispose (GObject * obj) {
	PanelMenuBar * self;
	PanelMenuBarClass * klass;
/*	GObjectClass * parent_class;*/
	self = PANEL_MENU_BAR (obj);
	(self->priv->applet == NULL ? NULL : (self->priv->applet = (g_object_unref (self->priv->applet), NULL)));
	(self->priv->file_browsers == NULL ? NULL : (self->priv->file_browsers = (gpointer)(g_ptr_array_free (self->priv->file_browsers, TRUE))));
	klass = PANEL_MENU_BAR_CLASS (g_type_class_peek (TYPE_PANEL_MENU_BAR));
	G_OBJECT_CLASS (panel_menu_bar_parent_class)->dispose (obj);
}
/******************************************************************************/
GType panel_menu_bar_get_type (void) {
	static GType panel_menu_bar_type_id = 0;
	if (G_UNLIKELY (panel_menu_bar_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (PanelMenuBarClass),
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


