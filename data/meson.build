#subdir('etc')

po_dir = join_paths (meson.source_root (), 'po')

desktop_autostart = i18n.merge_file ('desktop-autostart',
                                     input: 'gooroom-guide.autostart.desktop.in',
                                     output: 'gooroom-guide.autostart.desktop',
                                     install: true,
                                     install_dir: join_paths (guide_sysconfdir, 'xdg', 'autostart'),
                                     po_dir: po_dir,
                                     type: 'desktop')
desktop_file = i18n.merge_file(
  input: 'gooroom-guide.desktop.in',
  output: 'gooroom-guide.desktop',
  type: 'desktop',
  po_dir: po_dir,
  install: true,
  install_dir: join_paths(get_option('datadir'), 'applications')
)

install_subdir(
  'guide',
  install_dir : join_paths(guide_datadir, 'gooroom-guide')
)
