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

#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#include "guide-application.h"
#include "guide-window.h"

struct _GuideApplication {
  GtkApplication          parent;

  GuideWindow             *window;
};

G_DEFINE_TYPE(GuideApplication, guide_application, GTK_TYPE_APPLICATION)

static void
guide_application_startup (GApplication *app)
{
  GuideApplication *self = GUIDE_APPLICATION (app);

  G_APPLICATION_CLASS (guide_application_parent_class)->startup (app);

  self->window = guide_window_new (GTK_APPLICATION (app));
}

static void
guide_application_activate (GApplication *app)
{
  GuideApplication *self = GUIDE_APPLICATION (app);

  guide_window_activate (self->window);

  //gtk_window_present (GTK_WINDOW (self->window));
}

static void
guide_application_finalize (GObject *obj)
{
  G_OBJECT_CLASS (guide_application_parent_class)->finalize (obj);
}

static int
guide_application_command_line (GApplication            *app,
                                GApplicationCommandLine *command_line)
{
  GuideApplication *self = GUIDE_APPLICATION (app);
  int retval = 0;

  guide_window_activate (self->window);
  return retval;
}

static void
guide_application_class_init (GuideApplicationClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GApplicationClass *application_class = G_APPLICATION_CLASS (class);

  object_class->finalize = guide_application_finalize;
  application_class->activate = guide_application_activate;
  application_class->startup = guide_application_startup;
  application_class->command_line = guide_application_command_line;

}

static void
guide_application_init (GuideApplication *app)
{
}

GtkApplication *
guide_application_new (void)
{
  return g_object_new (GUIDE_TYPE_APPLICATION,
                       "application-id", "org.gooroom.guide",
                       "flags", G_APPLICATION_HANDLES_COMMAND_LINE, NULL);
}
