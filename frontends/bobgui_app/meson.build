project('bobfilez-bobgui-demo', 'c',
  version: '0.1.0',
  default_options: ['c_std=gnu11', 'warning_level=1'])

cc = meson.get_compiler('c')
bobgui_dep = dependency('bobgui4', required: true)

authorized_sources = files('main.c')
repo_root = join_paths(meson.project_source_root(), '..', '..')
c_api_include_dir = include_directories(join_paths(repo_root, 'core', 'include'))
c_api_library_dirs = [
  join_paths(repo_root, 'build-msvc', 'core'),
  join_paths(repo_root, 'build', 'core'),
]

c_api_dep = dependency('', required: false)
if get_option('bobfilez_c_api').disabled()
  c_api_dep = dependency('', required: false)
else
  c_api_library = cc.find_library('fo_c_api', dirs: c_api_library_dirs, required: get_option('bobfilez_c_api').enabled())
  if c_api_library.found()
    c_api_dep = declare_dependency(
      dependencies: [c_api_library],
      include_directories: c_api_include_dir,
      compile_args: ['-DBOBFILEZ_HAVE_C_API=1']
    )
  endif
endif

app_dependencies = [bobgui_dep]
if c_api_dep.found()
  app_dependencies += c_api_dep
endif

executable('bobfilez-bobgui-demo',
  authorized_sources,
  dependencies: app_dependencies,
  install: false)
