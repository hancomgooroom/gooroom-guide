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

#include "config.h"

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "guide-resources.h"
#include "guide-window.h"
#include "guide-utils.h"

#include <locale.h>

#define MINIMUM_WIDTH 600
#define MINIMUM_HEIGHT 330

struct _GuideWindow
{
  GtkApplicationWindow     parent;
  GtkWidget               *header_bar;

  GtkWidget               *bar_stack;
  GtkWidget               *start_bar;
  GtkWidget               *content_bar;
  GtkWidget               *end_bar;
  GtkWidget               *auto_start_check;
  GtkWidget               *begin_button;
  GtkWidget               *close_button;

  GtkWidget               *event_box;
  GtkImage                *image_frame;
  GtkWidget               *page_label;

  GdkCursor               *pointer_cursor;
  GdkCursor               *default_cursor;

  GArray                  *order_list;

  int                      index;                // current page index
  int                      total;                  // total guide page index
};

typedef enum {
  CURSOR_DEFAULT = 0,
  CURSOR_POINTER,
} CursorTypeEnum;

G_DEFINE_TYPE(GuideWindow, guide_window, GTK_TYPE_APPLICATION_WINDOW);

static void
guide_window_check_show_at_begin (GtkButton* button, GuideWindow *window)
{
  g_autofree gchar *norun = get_norun_file_path ();

  if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
  {
    if (!g_file_test(norun, G_FILE_TEST_EXISTS))
    {
      FILE *fd = g_fopen (norun, "w");
      fwrite ("1", 1, 1, fd);
      fclose (fd);
    }
  }
  else
  {
    if (g_file_test(norun, G_FILE_TEST_EXISTS))
      g_remove(norun);
  }
}

static gboolean
guide_window_present (gpointer user_data)
{
  GuideWindow *window = GUIDE_WINDOW (user_data);
  gtk_window_present (GTK_WINDOW (window));
  return 0;
}

static void
set_page_label (GuideWindow *self)
{
  gchar *page = g_strdup_printf ("%d / %d", self->index, self->total);

  gtk_label_set_label (GTK_LABEL (self->page_label), page);
  g_free (page);
}

static void
load_image (int index, GuideWindow *self)
{
  gchar *uri = NULL;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  uri = g_array_index (self->order_list, gchar *, index);
  self->index = index;

  pixbuf = gdk_pixbuf_new_from_file (uri,error);

  if (pixbuf == NULL) {
      g_print ("Error loading file: #%d %s\n", error->code, error->message);
      g_error_free (error);
      exit (1);
  }
  gtk_image_set_from_pixbuf (self->image_frame, pixbuf);
  set_page_label(self);
  guide_window_present(self);
}

static void
guide_window_event_box_button_press_cb(GuideWindow *self, GtkEventBox *box)
{
  if (self->index == self->total+1)
    g_spawn_command_line_async ("/usr/bin/xdg-open https://www.hancom.com/product/productGooroomMain.do", NULL);
}

static void
guide_window_clicked_prev (GtkButton* button, GuideWindow *self)
{
  int index = self->index;
  int total = self->total;

  load_image (--index, self);

  if (index == 0)
  {
      gtk_stack_set_visible_child (GTK_STACK (self->bar_stack), self->start_bar);
  }
  else if (index == total)
      gtk_stack_set_visible_child (GTK_STACK (self->bar_stack), self->content_bar);
}

static void
guide_window_clicked_next (GtkButton* button, GuideWindow *self)
{
  if (self->index == 0)
    gtk_stack_set_visible_child (GTK_STACK (self->bar_stack), self->content_bar);

  // If last page that disable next button
  if (self->index == self->total)
    gtk_stack_set_visible_child (GTK_STACK (self->bar_stack), self->end_bar);

  load_image(++self->index, self);
  
}

static void
clear_tocs (gchar **item)
{
  g_free (item);
}

gchar*
get_language ()
{
  gchar *locale = NULL;
  gchar *result = NULL;
  const gchar **split = NULL;
  const g_autofree gchar *lang = NULL;

  locale = setlocale (LC_MESSAGES, NULL);
  if (locale == NULL)
  {
    result = g_strdup_printf ("ko");
    goto END;
  }

  split = g_strsplit (locale, ".", -1);
  lang = g_strdup_printf ("%s", split[0]);

  if (g_strcmp0 (lang, "ko") == 0 ||
      g_strcmp0 (lang, "ko_KR") == 0) {
    result = g_strdup_printf ("ko");
  }
  else if (g_strcmp0 (lang, "en") == 0 ||
      g_strcmp0 (lang, "en_GB") == 0 ||
      g_strcmp0 (lang, "en_US") == 0) {
    result = g_strdup_printf ("en");
  }

END:
  setlocale (LC_MESSAGES, locale);

  g_strfreev (split);

  return result;
}

static gboolean
get_image_list(GuideWindow *self)
{
  g_autofree gchar *toc_path = NULL;
  g_autofree gchar *toc = NULL;
  g_autoptr(GError) error = NULL;
  JsonParser *parser;
  JsonObject *ro;
  JsonObjectIter iter;
  JsonArray *array;
  g_autofree gchar *lang = get_language ();

  int cnt = 0, length = 0;

  toc_path = g_strdup_printf ("%s", PACKAGE_GUIDEDIR);
  toc = g_strdup_printf ("%s/%s", toc_path, "order");
  if (!g_file_test (toc, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
    return FALSE;

  parser = json_parser_new ();

  if (!json_parser_load_from_file (parser, toc, &error))
  {
    g_error ("%s", error->message);
    return FALSE;
  }

  ro = json_node_get_object (json_parser_get_root (parser));
  array = json_object_get_array_member (ro, "paths");

  length = json_array_get_length (array);
  self->total = length - 2;

  self->order_list = g_array_new (TRUE, TRUE, sizeof (gchar *));
  g_array_set_clear_func (self->order_list, (GDestroyNotify) clear_tocs);

  while (cnt < length)
  {
    const gchar *content = json_array_get_string_element (array, cnt);
    gchar *path = g_strdup_printf ("%s/%s%s", toc_path, lang, content);

    g_array_append_val (self->order_list, path);

    cnt++;
  }

  load_image (0, self);

  return TRUE;
}

static void
init_cursor (GuideWindow *self)
{
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (self));
  GdkDisplay *display = gdk_screen_get_display (screen);
  self->default_cursor = gdk_cursor_new_from_name (display, "default");
  self->pointer_cursor = gdk_cursor_new_from_name (display, "pointer");
}

static gboolean
set_cursor (GuideWindow *self, CursorTypeEnum type)
{
  GdkCursor *cursor = NULL;
  GdkWindow *window = NULL;

  switch (type) {
    case CURSOR_DEFAULT:
      cursor = self->default_cursor;
    break;
    case CURSOR_POINTER:
      cursor = self->pointer_cursor;
    break;
  }

  window = gtk_widget_get_window (GTK_WIDGET (self));
  gdk_window_set_cursor (window, cursor);

  return FALSE;
}

static gboolean
guide_window_enter_button (GtkButton *button, GdkEvent *event, GuideWindow *self)
{
  return set_cursor (self, CURSOR_POINTER);
}

static gboolean
guide_window_leave_button (GtkButton *button, GdkEvent *event, GuideWindow *self)
{
  return set_cursor (self, CURSOR_DEFAULT);
}

static void
guide_window_click_close (GtkButton* button, GuideWindow *self)
{
  gtk_window_close (GTK_WINDOW (self));
}

void
guide_window_activate (GuideWindow *self)
{
  gboolean              show_at_begin = FALSE;
  gchar                *norun;

  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (self->header_bar), TRUE);

  // ------------ norun check ------------
  norun = get_norun_file_path ();
  if (!g_file_test (norun, G_FILE_TEST_EXISTS))
      show_at_begin = TRUE;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->auto_start_check), show_at_begin);

  get_image_list(self);
  init_cursor (self);
}

static void
guide_window_dispose (GObject *obj)
{
  GuideWindow *self = GUIDE_WINDOW (obj);

  G_OBJECT_CLASS (guide_window_parent_class)->dispose (obj);

  g_clear_object (&self->default_cursor);
  g_clear_object (&self->pointer_cursor);
}

static void
guide_window_finalize (GObject *obj)
{
  GuideWindow *self = GUIDE_WINDOW (obj);

  G_OBJECT_CLASS (guide_window_parent_class)->finalize (obj);
}

static void
guide_window_constructed (GObject *obj)
{
  GuideWindow *self = GUIDE_WINDOW (obj);
  GtkRequisition req;
  GdkDisplay *display;
  GdkMonitor *monitor;
  GdkRectangle geo;
  GdkPoint point;

  G_OBJECT_CLASS (guide_window_parent_class)->constructed (obj);

  display = gdk_display_get_default ();
  monitor = gdk_display_get_primary_monitor (display);

  gdk_monitor_get_geometry (monitor, &geo);
  gtk_widget_get_preferred_size (GTK_WIDGET (self), &req, NULL);

  point.x = geo.x + geo.width/2 - req.width/2;
  point.y = geo.y + geo.height/2 - req.height/2;

  if (geo.width <= req.width || geo.height <= req.height)
    gtk_widget_set_size_request (GTK_WIDGET (self->image_frame), MINIMUM_WIDTH, MINIMUM_HEIGHT);

  gtk_window_move (self,point.x ,point.y );
}

static void
guide_window_class_init (GuideWindowClass *class)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->dispose = guide_window_dispose;
  object_class->finalize = guide_window_finalize;
  object_class->constructed = guide_window_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, 
                                               "/org/gooroom/guide/guide.ui");

  gtk_widget_class_bind_template_child (widget_class, GuideWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, bar_stack);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, start_bar);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, content_bar);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, end_bar);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, auto_start_check);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, page_label);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, event_box);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, image_frame);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, begin_button);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, close_button);

  gtk_widget_class_bind_template_callback (widget_class, guide_window_clicked_prev);
  gtk_widget_class_bind_template_callback (widget_class, guide_window_clicked_next);
  gtk_widget_class_bind_template_callback (widget_class, guide_window_check_show_at_begin);
  gtk_widget_class_bind_template_callback (widget_class, guide_window_click_close);
  gtk_widget_class_bind_template_callback (widget_class, guide_window_enter_button);
  gtk_widget_class_bind_template_callback (widget_class, guide_window_leave_button);
}

static void
guide_window_init (GuideWindow *self)
{
  GtkStyleProvider  *provider;

  gtk_widget_init_template (GTK_WIDGET (self));

  // ------------ load css ------------
  provider = GTK_STYLE_PROVIDER (gtk_css_provider_new ());
  gtk_css_provider_load_from_resource (GTK_CSS_PROVIDER (provider),"/org/gooroom/guide/styles.css");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                  provider,
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_signal_connect_swapped (self->event_box, "button-press-event",
							(GCallback)guide_window_event_box_button_press_cb, self);

  g_object_unref (provider);

  return;
}

GuideWindow *
guide_window_new (GtkApplication *app)
{
  g_return_val_if_fail (GTK_IS_APPLICATION (app), NULL);

  return g_object_new (GUIDE_WINDOW_TYPE,
                       "application", app,
                       "resizable", FALSE,
                       "title", _("Gooroom Guide"),
                       "icon-name", "gooroom-guide",
                       "window-position", GTK_WIN_POS_CENTER,
                       "show-menubar", FALSE,
                       NULL);

}
