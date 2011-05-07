srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")
  opt.tool_options("compiler_cc")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("compiler_cc")
  conf.check_tool("node_addon")
  conf.env.append_value('CCFLAGS', ['-O3'])

def build(bld):
  bld.exec_command('./configure', cwd='cityhash')

  libcity = bld.new_task_gen("cxx", "shlib")
  libcity.source = "cityhash/src/city.cc"
  libcity.includes = ["cityhash", "cityhash/src"]
  libcity.name = "libcity"
  libcity.target = "libcity"

  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "cityhash"
  obj.source = "binding.cc"
  obj.includes = "libcity"
  obj.add_objects = "libcity"