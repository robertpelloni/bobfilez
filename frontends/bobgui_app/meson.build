project('bobfilez-bobgui-demo', 'c',
  version: '0.1.0',
  default_options: ['c_std=gnu11', 'warning_level=1'])

bobgui_dep = dependency('bobgui4', required: true)

authorized_sources = files('main.c')

executable('bobfilez-bobgui-demo',
  authorized_sources,
  dependencies: [bobgui_dep],
  install: false)
