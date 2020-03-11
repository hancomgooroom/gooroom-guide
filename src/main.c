#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "guide-application.h"
#include "guide-utils.h"

int
main (int argc, char **argv)
{
  g_autoptr(GtkApplication) application = NULL;
  if (NULL != argv[1])
  {
      if (0 == strcmp(argv[1], "autostart"))
      {
          g_autofree gchar *norun = get_norun_file_path ();
          if (g_file_test(norun, G_FILE_TEST_EXISTS))
              return 0;
      }
  }

  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  return g_application_run (G_APPLICATION (guide_application_new ()), argc, argv);
}
