srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
    opt.tool_options("compiler_cxx")

def configure(conf):
    conf.check_tool("compiler_cxx")
    conf.check_tool("node_addon")
    
    conf.check_cfg(package='openni', args='--cflags --libs', uselib_store='LIBOPENNI')

def build(bld):
    obj = bld.new_task_gen("cxx", "shlib", "node_addon")
    obj.cxxflags = ["-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall", "-I/usr/include/ni"]
    obj.target = "skelesense"
    obj.source = "src/log.cpp src/nitools.cpp src/scene.cc src/skelesense.cc"
    obj.linkflags = ['-lopenni']
    obj.uselib = ['LIBOPENNI']
