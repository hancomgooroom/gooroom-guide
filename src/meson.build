desktop = 'gooroom-guide.desktop'
desktop_in = configure_file(
  input : desktop + '.in.in',
  output : desktop + '.in',
  configuration : desktop_conf
)

i18n.merge_file(
  desktop,
  type : 'desktop',
  input : desktop_in,
  output : desktop,
  po_dir : po_dir,
  install : true,
  install_dir : guide_desktopdir
)

cflags = [
  '-DPACKAGE_GUIDEDIR="@0@"'.format(join_paths(guide_datadir, 'gooroom-guide', 'guide')),
  '-DTESTDIR="@0@"'.format(join_paths(guide_sysconfdir, 'xdg', 'autostart'))
]

common_deps += [
  webkit2gtk_dep,
  #dependency('json-c', version: '>= 0.12.1'),
  dependency('jsoncpp', version: '>= 1.7.4'),
  #dependency('json-glib-1.0', version:'1.4.4')
]

resources_data = files(
  'guide.ui'
)

sources = files(
  'guide.cpp'
)

common_sources = []
common_sources += gnome.compile_resources(
  '@0@-resources'.format('guide'),
  'guide.gresource.xml',
  c_name : 'guide',
  dependencies: resources_data,
  export: true
)

sources += common_sources

executable(
  meson.project_name(),
  sources,
  include_directories : top_inc,
  dependencies : common_deps,
  cpp_args : cflags,
  install : true
)
