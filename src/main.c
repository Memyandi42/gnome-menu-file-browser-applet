/*
 * File:                main.c
 * Created:             September 2005
 * Created by:          Axel von Bertoldi
 * Last Modified:       April 2009
 * Last Modified by:    Axel von Bertoldi
 * (C) 2005-2008        Axel von Bertoldi
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

#include <panel-applet.h>

#include "panel-menu-bar.h"
#include "utils.h"
#include "config.h"

#define APPLET_IID          "OAFIID:GNOME_FileBrowserApplet"
#define APPLET_FACTORY_IID  "OAFIID:GNOME_FileBrowserApplet_Factory"
#define VERSION             "0.6.4"

/******************************************************************************/
static const gchar file_browser_applet_menu_xml [] =
    "<popup name=\"button3\">\n"
    "   <menuitem name=\"Preferences Item\" verb=\"Preferences\" _label=\"Preferences\"\n"
    "             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
    "   <menuitem name=\"Help Item\" verb=\"Help\" _label=\"Help\"\n"
    "             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
    "   <menuitem name=\"About Item\" verb=\"About\" _label=\"About\"\n"
    "             pixtype=\"stock\" pixname=\"gtk-about\"/>\n"
    "</popup>\n";
/******************************************************************************/
static gboolean
file_browser_applet_display_properties_dialog (GtkWidget *widget, PanelMenuBar *panel_menu_bar) {

    panel_menu_bar_edit_prefs (panel_menu_bar);
    return FALSE;
}
/******************************************************************************/
static gboolean
file_browser_applet_display_help_dialog (GtkWidget *widget) {

#ifdef ENABLE_HELP_DOC
    GError *error = NULL;

    gnome_help_display_desktop_on_screen (NULL,
                                          APP_NAME,
                                          APP_NAME,
                                          NULL,
                                          gtk_widget_get_screen (widget),
                                          &error);
    utils_gerror_ok (&error, TRUE);
#else
    utils_show_dialog (_("Error"), _("Sorry, no help or documentation yet..."), GTK_MESSAGE_ERROR);
#endif
    return FALSE;
}
/******************************************************************************/
static gboolean
file_browser_applet_display_about_dialog (GtkWidget *widget) {

    const gchar *authors[] = {
        "Axel von Bertoldi <bertoldia@gmail.com>",
        "",
        _("Contributions by:"),
        "Ivan N. Zlatev <contact@i-nz.net>",
        "Stefano Maggiolo <maggiolo@mail.dm.unipi.it>",
        "Deji Akingunola <dakingun@gmail.com>",
        "Serkan Kaba <serkan@gentoo.org>",
        "Silvio Ricardo Cordeiro <silvioricardoc@gmail.com>",
        "pachoramos",
        NULL
    };
    const gchar *documenters [] = {
        _("You!!! That's right! You can help!"),
/*      "Axel von Bertoldi", */
        NULL
    };
    const gchar *translator_credits = _("You!!! That's right! You can help!");

    static GtkWidget *about_dialog = NULL;
    if (about_dialog) {
        gtk_window_set_screen (GTK_WINDOW (about_dialog),
                gtk_widget_get_screen (widget));
        gtk_window_present (GTK_WINDOW (about_dialog));
        return 0;
    }
    about_dialog = gtk_about_dialog_new ();
    g_object_set (about_dialog, 
                  "name", _("File Browser Applet"),
                  "version", VERSION,
                  "copyright", _("Copyright \xc2\xa9 2006-2009 Axel von Bertoldi."),
                  "comments", _("Browse and open files in your home directory from the panel"),
                  "authors", authors,
                  "documenters", documenters,
                  "translator-credits", strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
                  "logo-icon-name", APP_NAME,
                  "icon-name", APP_NAME,
                  NULL);
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (about_dialog),
            "http://code.google.com/p/gnome-menu-file-browser-applet/");
    gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (about_dialog), "File Browser Applet Website");
    gtk_window_set_screen (GTK_WINDOW (about_dialog), gtk_widget_get_screen (widget));
    g_signal_connect (about_dialog, "destroy", G_CALLBACK (gtk_widget_destroyed), &about_dialog);
    g_signal_connect (about_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
    gtk_widget_show (about_dialog);

    return 0;
}
/******************************************************************************/
static const
BonoboUIVerb file_browser_applet_menu_verbs [] = {
    BONOBO_UI_UNSAFE_VERB ("Preferences", file_browser_applet_display_properties_dialog),
    BONOBO_UI_UNSAFE_VERB ("Help",        file_browser_applet_display_help_dialog),
    BONOBO_UI_UNSAFE_VERB ("About",       file_browser_applet_display_about_dialog),
    BONOBO_UI_VERB_END
};
/******************************************************************************/
static gboolean
file_browser_applet_create (PanelApplet *applet) {

    setlocale (LC_ALL, "");
    bindtextdomain (APP_NAME, LOCALEDIR);
    textdomain (APP_NAME);

    PanelMenuBar* panel_menu_bar = panel_menu_bar_new (applet);

    panel_applet_set_flags (applet,
/*                          PANEL_APPLET_EXPAND_MAJOR |*/
/*                          PANEL_APPLET_HAS_HANDLE |*/
                            PANEL_APPLET_EXPAND_MINOR
                            );

    panel_applet_setup_menu (PANEL_APPLET (applet),
                             file_browser_applet_menu_xml,
                             file_browser_applet_menu_verbs,
                             panel_menu_bar);

    gtk_widget_show_all (GTK_WIDGET(applet));
    gtk_main ();

    return TRUE;
}
/******************************************************************************/
static gboolean
file_browser_applet_factory (PanelApplet *applet,
                             const  gchar *iid,
                             gpointer data) {

    if (strcmp (iid, APPLET_IID) == 0) {
        return file_browser_applet_create (applet);
    }

    return FALSE;
}
/******************************************************************************/
PANEL_APPLET_BONOBO_FACTORY (APPLET_FACTORY_IID,
                            PANEL_TYPE_APPLET,
                            APP_NAME,
                            "0",
                            file_browser_applet_factory,
                            NULL)
/******************************************************************************/
