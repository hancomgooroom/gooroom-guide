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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GUIDE_TYPE_APPLICATION (guide_application_get_type())
G_DECLARE_FINAL_TYPE (GuideApplication, guide_application, GUIDE, APPLICATION, GtkApplication)

GtkApplication          *guide_application_new                  (void);

G_END_DECLS
