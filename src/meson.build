cflags = [
  '-DPACKAGE_GUIDEDIR="@0@"'.format(join_paths(guide_datadir, 'gooroom-guide', 'guide')),
  '-DTESTDIR="@0@"'.format(join_paths(guide_sysconfdir, 'xdg', 'autostart')),
  '-DGNOMELOCALEDIR="@0@"'.format(guide_localedir)
]

common_deps += [
  webkit2gtk_dep,
  dependency('json-glib-1.0', version:'>=1.4.4'),
]

resources_data = files(
  'guide.ui'
)

sources = files(
  'main.c',
  'guide-application.c',
  'guide-window.c',
  'guide-utils.c'
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
  c_args : cflags,
  install : true
)
