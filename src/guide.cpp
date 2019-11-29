/*
 * Copyright (C) 2019 Gooroom <gooroom@gooroom.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.

 */

#include "guide-resources.h"

#include <config.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <json/json.h>

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gdk/gdk.h>
#include <webkit2/webkit2.h>
#include <glib/gi18n.h>
#include <bits/stdc++.h>

#define fopen_s(fp, fmt, mode)	*(fp)=fopen((fmt), (mode))

std::vector<std::string> content_file_paths = std::vector<std::string>();
uint total_page_index;

GtkBuilder *builder;

GtkWidget *window;
//GtkWidget *top_stack;
GtkWidget *bar_stack;

//GtkWidget *start_view;
GtkWidget *webview_frame;
GtkWidget *start_bar;
GtkWidget *auto_start_check;
GtkWidget *begin_button;

GtkWidget *end_bar;
GtkWidget *prev_button1;

//GtkWidget *content_view;
GtkWidget *content_bar;
GtkWidget *page_label;
GtkWidget *prev_button;
GtkWidget *next_button;

GtkWidget *header_bar;
static WebKitWebView* webview = NULL;

guint current_index;
gboolean is_show_at_begin = FALSE;

#define WID(y) (GtkWidget *) gtk_builder_get_object (builder, y)

std::string get_home_dir()
{
    std::string dir = g_get_home_dir();
    if (0 == dir.compare("/root"))
    {
        dir.clear();

        char pwd[PATH_MAX];
        if (NULL == getcwd(pwd, sizeof(pwd)))
            return "";

        char* tok;
        tok = strtok(pwd, "/");
        dir.append("/");
        dir.append(tok);
        dir.append("/");

        tok = strtok(NULL, "/");
        dir.append(tok);
    }

    return dir;
}

std::string get_expand_user()
{
    std::string expand = get_home_dir();

    expand.append("/.config");
    if (!g_file_test(expand.c_str(), G_FILE_TEST_IS_DIR))
        mkdir(expand.c_str(), 0777);

    expand.append("/gooroom");
    if (!g_file_test(expand.c_str(), G_FILE_TEST_IS_DIR))
        mkdir(expand.c_str(), 0777);

    expand.append("/gooroom-guide");
    if (!g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
        mkdir(expand.c_str(), 0777);

    return expand;
}

static bool
init_uri_list()
{
    std::string toc_path;
    toc_path.append(PACKAGE_GUIDEDIR);
    toc_path.append("/toc.json");

    std::ifstream json_doc(toc_path.c_str(), std::ifstream::binary);

    Json::Value value;
    Json::Reader reader;

    bool success = reader.parse(json_doc, value, true);
    if (!success)
    {
        std::cout<< reader.getFormatedErrorMessages();
    }

    Json::Value toc = value["paths"];
    std::string path;
    path.append ("file://");
    path.append (PACKAGE_GUIDEDIR);
    total_page_index = toc.size() - 2;

    for (guint i = 0; i < toc.size(); i++)
    {
        std::string str;
        str.append(path.c_str());
        str.append(toc[i].asString().c_str());

        content_file_paths.push_back(str.c_str());
    }

    return TRUE;
}

static void
set_page_label ()
{
    gchar *page = g_strdup_printf ("%d / %d", current_index, total_page_index);

    gtk_label_set_label (GTK_LABEL (page_label), page);
    g_free (page);
}

static void
load_uri (int index)
{
    webkit_web_view_load_uri (webview, content_file_paths[index].c_str());

    set_page_label();
}

static void
check_show_at_begin(GtkButton* button, gpointer userData)
{
    is_show_at_begin = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    std::string expand = get_expand_user();

    if (!g_file_test(expand.c_str(), G_FILE_TEST_IS_DIR))
    {
        mkdir(expand.c_str(), 0777);
    }

    expand.append("/norun");
    if (!is_show_at_begin)
    {
        if (!g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
        {
            std::ofstream file;
            file.open(expand.c_str());
            file.close();
        }
    }
    else
    {
        if (g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
        {
            std::ofstream file;
            remove(expand.c_str());
        }
    }

}

static gboolean
call_webview_context_menu(WebKitWebView* web_view, WebKitContextMenu* context_menu, GdkEvent* e, WebKitHitTestResult* htr, gpointer user_data)
{
    return true;
}

static void
web_view_load_changed (WebKitWebView *web_view,
                       WebKitLoadEvent load_event,
                       gpointer user_data)
{
    switch (load_event)
    {
        case WEBKIT_LOAD_COMMITTED:
        {
            const gchar *uri = webkit_web_view_get_uri (web_view);
            gchar **sp = g_strsplit (uri, "/", 15);
            guint length = g_strv_length (sp);

            //printf ("= = = = => started load: %s\n", uri);
            if (g_strcmp0 (sp[length - 1], "redirect.html") == 0)
            {
                // execute gooroom site
                g_spawn_command_line_async ("/usr/bin/gooroom-browser https://www.gooroom.kr", NULL);
                // load end page
                webkit_web_view_stop_loading (web_view);
                load_uri (current_index);
            }

            g_strfreev (sp);
        }
        break;
        //case WEBKIT_LOAD_REDIRECTED:
        //{
        //    const gchar *uri = webkit_web_view_get_uri (web_view);

        //    printf ("= = = = => started load: %s\n", uri);
        //    if (g_strcmp0 (uri, "https://www.gooroom.kr/") == 0)
        //    {
        //        // execute gooroom site
        //        gchar *cmd = g_strdup_printf ("gooroom-browser %s", uri);
        //        system(cmd);
        //        // load end page
        //        webkit_web_view_stop_loading (web_view);
        //        load_uri (current_index);
        //        g_free (cmd);
        //    }
        //}
        break;
        case WEBKIT_LOAD_FINISHED:
        case WEBKIT_LOAD_STARTED:
        {
        }
        break;
    }
}

static void
create_webview ()
{
    init_uri_list();

    webview = WEBKIT_WEB_VIEW (webkit_web_view_new ());
    gtk_container_add (GTK_CONTAINER (webview_frame), GTK_WIDGET (webview));

    g_signal_connect (webview, "context-menu", G_CALLBACK (call_webview_context_menu), NULL);
    g_signal_connect (webview, "load-changed", G_CALLBACK (web_view_load_changed), NULL);

    load_uri (0);
}

static void
clicked_begin_button (GtkButton* button, gpointer data)
{
    gtk_stack_set_visible_child (GTK_STACK (bar_stack), content_bar);

    load_uri (++current_index);
}

static void
clicked_prev_button (GtkButton* button, gpointer data)
{
    load_uri (--current_index);
    if (current_index == 0)
    {
        gtk_stack_set_visible_child (GTK_STACK (bar_stack), start_bar);
    }
    else if (current_index == total_page_index)
        gtk_stack_set_visible_child (GTK_STACK (bar_stack), content_bar);

}

static void
clicked_next_button (GtkButton* button, gpointer data)
{
    load_uri (++current_index);

    // If last page that disable next button
    if (current_index == total_page_index + 1)
    {
        gtk_stack_set_visible_child (GTK_STACK (bar_stack), end_bar);
    }
}

static void
activate (GtkApplication* app)
{
    builder = gtk_builder_new_from_resource ("/org/gooroom/guide/guide.ui");

    //top_stack = WID ("top-stack");
    bar_stack = WID ("bar-stack");

    //start_view = WID ("start-view");
    webview_frame = WID ("webview-frame");
    start_bar = WID ("start-bar");
    auto_start_check = WID ("auto-start-check");
    begin_button = WID ("begin-button");

    //content_view = WID ("content-view");
    content_bar = WID ("content-bar");
    page_label = WID ("current-page-label");
    prev_button = WID ("prev-button");
    next_button = WID ("next-button");

    end_bar = WID ("end-bar");
    prev_button1 = WID("prev-button1");

    header_bar = WID ("headerbar");
    window = WID ("window");

    create_webview ();

    // norun check
    std::string expand = get_expand_user ();
    expand.append ("/norun");
    if (!g_file_test (expand.c_str(), G_FILE_TEST_EXISTS))
        is_show_at_begin = TRUE;
    else
        is_show_at_begin = FALSE;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (auto_start_check), is_show_at_begin);

    //gtk_stack_set_transition_type (GTK_STACK (top_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_type (GTK_STACK (bar_stack), GTK_STACK_TRANSITION_TYPE_NONE);
    gtk_widget_set_sensitive (prev_button, TRUE);

    g_signal_connect (auto_start_check, "toggled", G_CALLBACK (check_show_at_begin), NULL);
    g_signal_connect (begin_button, "clicked", G_CALLBACK (clicked_begin_button), NULL);
    g_signal_connect (prev_button, "clicked", G_CALLBACK (clicked_prev_button), NULL);
    g_signal_connect (next_button, "clicked", G_CALLBACK (clicked_next_button), NULL);
    g_signal_connect (prev_button1, "clicked", G_CALLBACK (clicked_prev_button), NULL);
    g_signal_connect (G_OBJECT (window), "destroy", gtk_main_quit, NULL);

    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

    gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), TRUE);

    gtk_widget_show_all (window);

    g_object_unref(builder);
    gtk_main ();
}

int
main (int argc, char **argv)
{
    if (NULL != argv[1])
    {
        if (0 == strcmp(argv[1], "autostart"))
        {
            std::string expand = get_expand_user();
            expand.append("/norun");

            if (g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
                return 0;
        }
    }

    bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    GtkApplication *app;

    app = gtk_application_new ("org.gooroom.guide", G_APPLICATION_NON_UNIQUE);

    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    g_application_run (G_APPLICATION (app), 0, &argv[0]);

    return 1;
}
