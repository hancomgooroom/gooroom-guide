project(
  'gooroom-guide', 'c',
  version : '1.0.0',
  license : 'GPL2+',
  meson_version : '>=0.43.0'
)

guide_prefix = get_option('prefix')
guide_datadir = join_paths(guide_prefix, get_option('datadir'))
guide_bindir = join_paths(guide_prefix, get_option('bindir'))
guide_sysconfdir = join_paths(guide_prefix, get_option('sysconfdir'))
guide_localedir = join_paths(guide_prefix, get_option('localedir'))
#guide_sysconfdir = join_paths('/', get_option('sysconfdir'))

guide_icondir = join_paths(guide_datadir, 'icons')
guide_desktopdir = join_paths(guide_datadir, 'applications')

#cc = meson.get_compiler('cpp')
cc = meson.get_compiler('c')

guide_gettext = meson.project_name() + '-1.0.0'

config_h = configuration_data()
set_defines = [
  ['PACKAGE', meson.project_name()],
  ['PACKAGES_VERSION', meson.project_version()],
  ['VERSION', meson.project_version()],
  ['GETTEXT_PACKAGE', guide_gettext]
]

foreach define: set_defines
  config_h.set_quoted(define[0], define[1])
endforeach

#configure_file(
#  output: 'config.h',
#  configuration:config_h
#)

glib_dep = dependency('glib-2.0', version: '>= 2.58.3')
gnome_desktop_dep = dependency('gnome-desktop-3.0', version: '>= 3.27.90')
webkit2gtk_dep = dependency('webkit2gtk-4.0', version: '>=2.24.2')

common_deps = [
  glib_dep,
  dependency('gio-unix-2.0'),
  dependency('gtk+-3.0', version: '> 3.22.20'),
  dependency('glib-2.0', version: '2.58.3')
]

gnome = import('gnome')
i18n = import('i18n')
pkg = import('pkgconfig')

desktop_conf = configuration_data()
desktop_conf.set('VERSION', meson.project_version())

po_dir = join_paths(meson.source_root(), 'po')
its_dir = join_paths(meson.source_root(), 'gettext')

install_subdir(
  'gettext',
  install_dir: guide_datadir
)

top_inc = include_directories('.')

subdir('app-icons')
subdir('data')
subdir('po')
subdir('src')

configure_file(
  output: 'config.h',
  configuration: config_h
)

output = ''
output += '\n        ' + meson.project_name() + ' - ' + meson.project_version() + '\n'

message(output)
