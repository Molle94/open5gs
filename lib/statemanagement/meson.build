libstatemanagement_sources = files('''
    statemanagement.c
'''.split())

# dependencies
zmqdep = dependency ('libzmq', version: '>=4.2.0')
czmqdep = dependency ('libczmq', version: '>=3.0.0')

protoc = find_program('protoc', required : true)
protodep = dependency('protobuf', required : true)
protocdep = dependency('libprotobuf-c', required : true)

if not protoc.found() or not protodep.found()
  error('MESON_SKIP_TEST: protoc tool and/or protobuf pkg-config dependency not found')
endif


gen = generator(protoc, \
  output    : ['@BASENAME@.pb-c.c', '@BASENAME@.pb-c.h'],
  arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--c_out=@BUILD_DIR@', '@INPUT@'])

generated = gen.process('defs.proto')

proto_interface = declare_dependency(
  sources : generated,
  dependencies : [protodep, protocdep],
)
# libstatemanagement_sources += generated

libstatemanagement_inc = include_directories('.')

libstatemanagement = library('ogsstatemanagement',
    sources : libstatemanagement_sources,
    version : libogslib_version,
    c_args : '-DOGS_STATEMANAGEMENT_COMPILATION',
    include_directories : [libstatemanagement_inc, libinc],
    dependencies : [zmqdep, czmqdep, proto_interface, libcore_dep],
    install : true)

libstatemanagement_dep = declare_dependency(
    link_with : libstatemanagement,
    include_directories : [libstatemanagement_inc, libinc],
    dependencies : [zmqdep, czmqdep, proto_interface, libcore_dep],
)