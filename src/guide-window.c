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

#define MINIMUM_WIDTH 600
#define MINIMUM_HEIGHT 330

struct _GuideWindow
{
  GtkApplicationWindow     parent;

  GtkWidget *auto_start_check;
  GtkWidget *overlay;
  GtkWidget *btn_startgrm;
  GtkImage  *image_frame;
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
open_help (GtkAccelGroup *accel,
           GObject *acceleratoable,
           guint keyval,
           GdkModifierType modifier,
           gpointer user_data)
{
  gtk_show_uri_on_window (GTK_WINDOW(user_data), "help:gooroom-guide",
                          gtk_get_current_event_time (),NULL);
}

static void
guide_window_accel_init(GtkWindow *window, gpointer uer_data)
{
  GtkAccelGroup *accel_group;
  guint accelerator_key;
  GdkModifierType accelerator_mod;
  GClosure *closure;

  accel_group = gtk_accel_group_new();
  gtk_accelerator_parse ( "F1", &accelerator_key, &accelerator_mod);
  closure = g_cclosure_new_object (G_CALLBACK (open_help),G_OBJECT(window));
  gtk_accel_group_connect (accel_group, accelerator_key,accelerator_mod, GTK_ACCEL_VISIBLE, closure);
  gtk_window_add_accel_group (window, accel_group);
}

static void
guide_window_event_box_button_press_cb(GuideWindow *self, GtkEventBox *box)
{
  g_spawn_command_line_async ("/usr/bin/xdg-open https://www.hancom.com/product/productGooroomMain.do", NULL);
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

void
guide_window_activate (GuideWindow *self)
{
  gboolean  show_at_begin = FALSE;
  GtkWidget	*check_button;
  gchar *norun;
  gchar *lang;

  // ------------ norun check ------------
  norun = get_norun_file_path ();
  if (!g_file_test (norun, G_FILE_TEST_EXISTS))
      show_at_begin = TRUE;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->auto_start_check), show_at_begin);

  lang = g_strdup (get_language());

  if (g_str_equal (lang, "en"))
    gtk_image_set_from_resource(GTK_IMAGE (self->image_frame), "/org/gooroom/guide/info_eng.jpg" );
  else 
    gtk_image_set_from_resource(GTK_IMAGE (self->image_frame), "/org/gooroom/guide/info_kor.jpg" );
}

static void
guide_window_dispose (GObject *obj)
{
  GuideWindow *self = GUIDE_WINDOW (obj);

  G_OBJECT_CLASS (guide_window_parent_class)->dispose (obj);
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

  gtk_widget_class_bind_template_child (widget_class, GuideWindow, auto_start_check);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, image_frame);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, overlay);
  gtk_widget_class_bind_template_child (widget_class, GuideWindow, btn_startgrm);
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

  guide_window_accel_init (self, NULL);

  g_object_unref (provider);
  g_signal_connect (GTK_BUTTON (self->btn_startgrm) , "clicked",
					(GCallback)guide_window_event_box_button_press_cb, self);
  g_signal_connect (GTK_BUTTON (self->auto_start_check) , "toggled",
					(GCallback)guide_window_check_show_at_begin, self);

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
