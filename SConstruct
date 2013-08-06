# -*- mode: python; -*-
import os
import itertools
import libdeps

env = Environment()

base_dir=Dir('.').abspath + "/bin/";
thirds_cpppath=[
	base_dir + "boost/include/",
	base_dir + "glog/include/"
]
thirds_libpath=[
	base_dir + "boost/lib/",
	base_dir + "glog/lib/"
]
thirds_libs=[
	'boost_program_options',
	'boost_random',
	'boost_regex',
	'boost_serialization',
	'boost_signals',
	'boost_system',
	'boost_thread',
	'glog',
	'boost_chrono'
]
ccflags=['-Wall', '-static', '-g']
cxxflags=['-std=c++0x']

# ------    ENV SETUP -----------
env['CPPPATH']=thirds_cpppath
env['LIBPATH']=thirds_libpath
env['LIBS']=thirds_libs
env['CCFLAGS']=ccflags
env['CXXFLAGS']=cxxflags

libdeps.setup_environment( env )

# ------    SOURCE FILE SETUP -----------
env.StaticLibrary('proxy',
	['./src/connection.cpp',
	'./src/io_service_pool.cpp',
	'./src/mime_types.cpp',
	'./src/request_handler.cpp',
	'./src/request_parser.cpp',
	'./src/server.cpp',
	'./src/text.cpp',
	'./src/config.cpp',
	'./src/backend_selector.cpp',
	'./src/reply.cpp'])

# ------    PROGRAM SETUP -----------
env.Program('proxy', ['./src/main.cpp'], LIBDEPS=['proxy'])



