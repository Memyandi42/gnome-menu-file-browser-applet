
#ifndef __PANEL_MENU_BAR_H__
#define __PANEL_MENU_BAR_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <panel-applet.h>
#include "menu-browser.h"

G_BEGIN_DECLS


#define TYPE_PANEL_MENU_BAR (panel_menu_bar_get_type ())
#define PANEL_MENU_BAR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_PANEL_MENU_BAR, PanelMenuBar))
#define PANEL_MENU_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_PANEL_MENU_BAR, PanelMenuBarClass))
#define IS_PANEL_MENU_BAR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_PANEL_MENU_BAR))
#define IS_PANEL_MENU_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_PANEL_MENU_BAR))
#define PANEL_MENU_BAR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_PANEL_MENU_BAR, PanelMenuBarClass))

typedef struct _PanelMenuBar PanelMenuBar;
typedef struct _PanelMenuBarClass PanelMenuBarClass;
typedef struct _PanelMenuBarPrivate PanelMenuBarPrivate;

struct _PanelMenuBar {
	GtkMenuBar parent;
	PanelMenuBarPrivate * priv;
};
struct _PanelMenuBarClass {
	GtkMenuBarClass parent;
};

PanelMenuBar* panel_menu_bar_new (PanelApplet* applet);
GType panel_menu_bar_get_type (void);

G_END_DECLS

#endif
