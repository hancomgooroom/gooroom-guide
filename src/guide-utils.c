#include "config.h"

#include <glib.h>
#include <glib/gstdio.h>
#include "guide-utils.h"
//#include <gstdio.h>

gchar *
get_norun_file_path ()
{
  g_autofree gchar *gooroom_dir;
  g_autofree gchar *guide_dir;

  gooroom_dir = g_strdup_printf ("%s/.config/gooroom", g_get_home_dir());//(dir, "./config", "/gooroom");
  if (!g_file_test (gooroom_dir, G_FILE_TEST_IS_DIR))
    g_mkdir(gooroom_dir, 0777);

  guide_dir = g_strdup_printf ("%s/gooroom-guide", gooroom_dir);
  if (!g_file_test (guide_dir, G_FILE_TEST_EXISTS))
    g_mkdir(guide_dir, 0777);

  return g_strdup_printf ("%s/norun", guide_dir);
}
