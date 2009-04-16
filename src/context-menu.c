/*
 * File:                context_menu.c
 * Created:             April 2008
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

#include <glib/gprintf.h>

#include "context-menu.h"
#include "vfs.h"
#include "utils.h"
#include "config.h"

/******************************************************************************/
static char *archive_mime_types[] = {
    "application/x-ar",
    "application/x-arj",
    "application/x-bzip",
    "application/x-bzip-compressed-tar",
    "application/x-compress",
    "application/x-compressed-tar",
    "application/x-deb",
    "application/x-gtar",
    "application/x-gzip",
    "application/x-lha",
    "application/x-lhz",
    "application/x-rar",
    "application/x-rar-compressed",
    "application/x-tar",
    "application/x-zip",
    "application/x-zip-compressed",
    "application/zip",
    "multipart/x-zip",
    "application/x-rpm",
    "application/x-jar",
    "application/x-java-archive",
    "application/x-lzop",
    "application/x-zoo",
    "application/x-cd-image",
    "application/x-7z-compressed",
    "application/x-gzpostscript",
    "application/x-ms-dos-executable",
    NULL
};
/******************************************************************************/
static void
tree_set_sensitive (GtkWidget *menu_item, gboolean sensitive) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);
    /* walk up the menu browser tree from the menu item that causes the popup
     * menu and disable all the menu shells, stopping at the menu bar. Need to
     * do this to get around the focus bug. Would be nicer if we could do this
     * w/o changing the appearance of the widgets  */
    GtkWidget *current = menu_item->parent;
    while (current) {
        gtk_widget_set_sensitive (current, sensitive);

        if (GTK_IS_MENU_BAR (current)) return;

        current = GTK_MENU_SHELL (current)->parent_menu_shell;
    }
}
/******************************************************************************/
/* Stolen from menu.c from the gnome-panel */
static void
restore_grabs (GtkWidget *w, gpointer data) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    GtkWidget *menu_item = data;
    GtkMenu *menu = GTK_MENU(menu_item->parent); 
    GtkWidget *xgrab_shell;
    GtkWidget *parent;

    tree_set_sensitive (menu_item, TRUE);

    /* Find the last viewable ancestor, and make an X grab on it */
    parent = GTK_WIDGET (menu);
    xgrab_shell = NULL;
    while (parent) {
        gboolean viewable = TRUE;
        GtkWidget *tmp = parent;

        while (tmp) {
            if (!GTK_WIDGET_MAPPED (tmp)) {
                viewable = FALSE;
                break;
            }
            tmp = tmp->parent;
        }

        if (viewable)
            xgrab_shell = parent;

        parent = GTK_MENU_SHELL (parent)->parent_menu_shell;
    }

    /*only grab if this HAD a grab before*/
    if (xgrab_shell && (GTK_MENU_SHELL (xgrab_shell)->have_xgrab)) {
        if (gdk_pointer_grab (xgrab_shell->window, TRUE,
                              GDK_BUTTON_PRESS_MASK |
                              GDK_BUTTON_RELEASE_MASK |
                              GDK_ENTER_NOTIFY_MASK |
                              GDK_LEAVE_NOTIFY_MASK,
                              NULL, NULL, 0) == 0) {

            if (gdk_keyboard_grab (xgrab_shell->window, TRUE, GDK_CURRENT_TIME) == 0)
                GTK_MENU_SHELL (xgrab_shell)->have_xgrab = TRUE;
            else
                gdk_pointer_ungrab (GDK_CURRENT_TIME);

        }
    }
    gtk_grab_add (GTK_WIDGET (menu));
}
/******************************************************************************/
static void
close_menu_browser (GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    GtkWidget *browser = g_object_get_data (G_OBJECT (menu), "menu_browser");
    GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (browser));
    gtk_menu_shell_deactivate (GTK_MENU_SHELL (parent));
}
/******************************************************************************/
static void
context_menu_clean_up (GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    gtk_widget_destroy (menu);
}
/******************************************************************************/
static void
context_menu_setup_callback (const gchar *app,
                             const gchar* file,
                             GtkMenuItem *menu_item,
                             GtkMenu *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    gchar **args = g_strv_new (ARGS_SIZE);
    args[ARG_APP]  = g_strdup (app);
    args[ARG_FILE] = g_strdup (file);

    g_signal_connect_data (GTK_MENU_ITEM (menu_item),
                           "activate",
                           G_CALLBACK (vfs_launch_application),
                           args,
                           (GClosureNotify)g_strfreev,
                           G_CONNECT_SWAPPED);

    g_signal_connect_swapped (menu_item,
                              "activate",
                              G_CALLBACK (close_menu_browser),
                              menu);
}
/******************************************************************************/
static void
context_menu_add_new_dir_callback (GtkWidget *menu_item, gchar *file_name) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    GtkWidget *menu = gtk_widget_get_parent (menu_item);
    close_menu_browser (menu);
    GError *error = NULL;

    GtkBuilder* builder = gtk_builder_new();
    /*gtk_builder_add_from_file (builder, BUILDER_UI_PATH, &error);*/
    gchar *toplevel = "new_dir_dialog";
    gtk_builder_add_objects_from_file (builder, BUILDER_UI_PATH, &toplevel, &error);
    if (!utils_gerror_ok(&error, TRUE)) {
        return;
    }
    GtkWidget *new_dir_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "new_dir_dialog"));
    GtkWidget *new_dir_entry = GTK_WIDGET (gtk_builder_get_object (builder, "new_dir_entry"));
    GtkWidget *current_path_label = GTK_WIDGET (gtk_builder_get_object (builder, "current_path_label"));
    
    gtk_label_set_text (GTK_LABEL (current_path_label),
                        file_name);

    if (gtk_dialog_run (GTK_DIALOG (new_dir_dialog)) == GTK_RESPONSE_ACCEPT) {
        const gchar *entry_text = gtk_entry_get_text (GTK_ENTRY (new_dir_entry));
        gchar *new_dir = g_strdup_printf ("%s/%s", file_name, entry_text);

        GFile *file = g_file_new_for_path (new_dir);
        g_file_make_directory (file, NULL, &error);
        g_object_unref (file);

        /* open the dir if we succeeded in creating it */
        if (utils_gerror_ok (&error, TRUE)) {
            vfs_file_do_default_action (new_dir);
        }
    }
    gtk_widget_destroy (new_dir_dialog);
}
/******************************************************************************/
static void
context_menu_add_new_dir (const gchar *file_name, GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    if (!vfs_file_is_directory (file_name)) return;

    GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (_("_New Folder Here"));
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                   gtk_image_new_from_stock (GTK_STOCK_NEW,
                                                             GTK_ICON_SIZE_MENU));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    g_signal_connect_data (GTK_MENU_ITEM (menu_item),
                           "activate",
                           G_CALLBACK (context_menu_add_new_dir_callback),
                           g_strdup (file_name),
                           (GClosureNotify)g_free,
                           0);
}
/******************************************************************************/
static void
context_menu_add_burn (const gchar *file_name, GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Create CD/DVD"));
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                   gtk_image_new_from_icon_name ("nautilus-cd-burner",
                                                                 GTK_ICON_SIZE_MENU));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    context_menu_setup_callback ("brasero",
                                 file_name,
                                 GTK_MENU_ITEM (menu_item),
                                 GTK_MENU (menu));
}
/******************************************************************************/
static void
context_menu_add_compile_tex (const gchar *file_name, GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    if (!g_str_has_suffix (file_name, "tex")) return;

    GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Build Latex Document"));
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                   gtk_image_new_from_icon_name ("build",
                                                                 GTK_ICON_SIZE_MENU));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    context_menu_setup_callback ("rubber -f --inplace -d",
                                 file_name,
                                 GTK_MENU_ITEM (menu_item),
                                 GTK_MENU (menu));
}
/******************************************************************************/
static gboolean
is_archive (const gchar *file_name) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    gboolean ret = FALSE;
    GFile*     file = g_file_new_for_path (file_name);
    GFileInfo* file_info =  g_file_query_info (file,
                                               G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                               0,
                                               NULL,                                               
                                               NULL);

    const gchar* content_type = g_file_info_get_content_type (file_info);

    int i;
    for (i = 0; archive_mime_types[i] != NULL; i++) {
        if (g_strcmp0 (content_type, archive_mime_types[i]) == 0) {
            ret = TRUE;
            break;
        }
    }
    g_object_unref (file_info);
    g_object_unref (file);

    return ret;
}
/******************************************************************************/
static void
context_menu_add_archive_action (const gchar *file_name, GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    gchar *archive_label = NULL;
    gchar *archive_action = NULL;

    if (is_archive (file_name)) {
        archive_label = _("_Extract Here");
        archive_action = "file-roller -h";
    }
    else {
        archive_label = _("Create _Archive");
        archive_action = "file-roller -d";
    }

    GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (archive_label);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                   gtk_image_new_from_icon_name ("package",
                                                                 GTK_ICON_SIZE_MENU));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    context_menu_setup_callback (archive_action,
                                 file_name,
                                 GTK_MENU_ITEM (menu_item),
                                 GTK_MENU (menu));
}
/******************************************************************************/
static void
context_menu_add_open_with_item (const gchar *file_name, GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    GList *root = vfs_get_all_mime_applications (file_name);
    GList *apps = root;
    
    g_return_if_fail (root != NULL);

    GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Open With"));
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                   gtk_image_new_from_stock (GTK_STOCK_OPEN,
                                                             GTK_ICON_SIZE_MENU));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    GtkWidget *sub_menu = gtk_menu_new ();

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);

    while (apps != NULL) {
        menu_item = gtk_image_menu_item_new_with_label (g_app_info_get_name (apps->data));
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                       gtk_image_new_from_gicon (g_app_info_get_icon (apps->data),
                                                                 GTK_ICON_SIZE_MENU));

        gtk_menu_shell_append (GTK_MENU_SHELL (sub_menu), menu_item);

        context_menu_setup_callback (g_app_info_get_executable (apps->data),
                                     file_name,
                                     GTK_MENU_ITEM (menu_item),
                                     GTK_MENU (menu));

        g_object_unref (apps->data);
        apps = apps->next;
    }
    g_list_free (root);
}
/******************************************************************************/
static void
context_menu_add_trash_item (const gchar *file_name, GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);
                          
    GtkWidget *menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Move to Trash"));
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                                        gtk_image_new_from_icon_name ("user-trash-full",
                                                                                      GTK_ICON_SIZE_MENU));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    g_signal_connect_data (GTK_MENU_ITEM (menu_item),
                           "activate",
                           G_CALLBACK (vfs_file_trash),
                           g_strdup (file_name),
                           (GClosureNotify)g_free,
                           G_CONNECT_SWAPPED);

    g_signal_connect_swapped (menu_item,
                              "activate",
                              G_CALLBACK (close_menu_browser),
                              menu);
}
/******************************************************************************/
static void
context_menu_populate (const gchar *file_name, GtkWidget *menu) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    context_menu_add_open_with_item (file_name, menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());
    context_menu_add_new_dir        (file_name, menu);
    context_menu_add_trash_item     (file_name, menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());
    context_menu_add_archive_action (file_name, menu);
    context_menu_add_compile_tex    (file_name, menu);
    context_menu_add_burn           (file_name, menu);

    gtk_widget_show_all (menu);
}
/******************************************************************************/
gboolean
context_menu_display (const gchar *file_name, GtkWidget *menu_item) {
    if (DEBUG) g_printf ("In %s\n", __FUNCTION__);

    int event_button;
    int event_time;

    GdkEventButton *event = g_object_get_data (G_OBJECT (menu_item), "button_event");
    if (event) {
        event_button = event->button;
        event_time = event->time;
    }
    else {
        event_button = 3;
        event_time = gtk_get_current_event_time();
    }

    GtkWidget *menu = gtk_menu_new ();

    /* add some data to the popup menu so we can get to it later */
    GtkWidget *menu_browser = g_object_get_data (G_OBJECT (menu_item), "menu_browser");
    g_object_set_data (G_OBJECT (menu), "menu_browser", menu_browser);
    g_object_set_data (G_OBJECT (menu), "menu_item", menu_item);

    g_signal_connect_swapped (menu_item,
                              "destroy",
                              G_CALLBACK (context_menu_clean_up),
                              menu);

    g_signal_connect (GTK_MENU_SHELL (menu),
                      "deactivate",
                      G_CALLBACK (restore_grabs),
                      menu_item);

    context_menu_populate (file_name, menu);

    /* disable the menu browser to get around the focus bug */
    tree_set_sensitive (menu_item, FALSE);

    gtk_menu_popup (GTK_MENU (menu),
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    event_button,
                    event_time);
    return TRUE;
}
/******************************************************************************/
