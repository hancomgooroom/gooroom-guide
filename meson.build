project(
  'gooroom-guide', 'cpp', 'c',
  version : '1.0.0',
  license : 'GPL2+',
  meson_version : '>=0.43.0'
)

guide_prefix = get_option('prefix')
guide_datadir = join_paths(guide_prefix, get_option('datadir'))
guide_bindir = join_paths(guide_prefix, get_option('bindir'))
guide_icondir = join_paths(guide_datadir, 'icons')
guide_sysconfdir = join_paths(guide_prefix, get_option('sysconfdir'))
#guide_sysconfdir = join_paths('/', get_option('sysconfdir'))
guide_desktopdir = join_paths(guide_datadir, 'applications')

cc = meson.get_compiler('cpp')

guide_gettext = meson.project_name() + '-1.0'

config_h = configuration_data()
configure_file(
  output: 'config.h',
  configuration:config_h
)

glib_dep = dependency('glib-2.0', version: '>= 2.58.3')
gnome_desktop_dep = dependency('gnome-desktop-3.0', version: '>= 3.27.90')

common_deps = [
  glib_dep,
  dependency('gio-unix-2.0'),
  dependency('gtk+-3.0', version: '> 3.22.20'),
  dependency('glib-2.0', version: '2.58.3')
]

webkit2gtk_dep = dependency('webkit2gtk-4.0', version: '2.24.2')

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

#contigure_file(
#  output: 'config.h',
#  configuration: config_h
#)

#output = ''
#output += '\n        ' + meson.project_name() + ' - ' + meson.project_version() + '\n'

#message(output)
