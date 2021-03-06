project('agentd', 'c', 'cpp',
    version : '0.4.2-snapshot',
    default_options : ['c_std=gnu11', 'cpp_std=c++14', 'buildtype=release'],
    meson_version : '>=0.53.0'
)

message('Using build type: ' + get_option('buildtype'))

force_velo_toolchain = get_option('force_velo_toolchain')

# Should we enable attestation?
enable_attestation = get_option('enable_attestation')
if enable_attestation
    add_project_arguments('-DATTESTATION=1', language: 'c')
    add_project_arguments('-DATTESTATION=1', language: 'cpp')
else
    add_project_arguments('-DATTESTATION=0', language: 'c')
    add_project_arguments('-DATTESTATION=0', language: 'cpp')
endif

# TODO: Move this into mesons built in warning level.
add_project_arguments('-Wall', '-Werror', '-Wextra', language : 'c')
add_project_arguments('-Wall', '-Werror', '-Wextra', language : 'cpp')

flex = find_program('flex', required : true)
bison = find_program('bison', required : true)

lgen = generator(
    flex,
    output : ['@PLAINNAME@.yy.c', '@BASENAME@.yy.h'],
    arguments : ['--header-file=@OUTPUT1@', '-o', '@OUTPUT0@', '@INPUT@']
)

pgen = generator(
    bison,
    output : ['@BASENAME@.tab.c', '@BASENAME@.tab.h'],
    arguments : ['@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@']
)

lfiles = lgen.process('src/config/agentd.l')
pfiles = pgen.process('src/config/agentd.y')

src_not_main = run_command(
    'find', './src', '(', '-name', '*.c', '-or', '-name', '*.h' , ')', 
            '-not', '-name', 'main.c', '-and', '-not', '-name', 'parser.y',
    check : true
).stdout().strip().split('\n')

test_src = run_command(
    'find', './test', '-name', '*.cpp', '-or', '-name', '*.h',
    check : true
).stdout().strip().split('\n')

vcblockchain = dependency(
    'vcblockchain',
    required : true,
    fallback : ['vcblockchain', 'vcblockchain_dep']
)

gtest = dependency(
    'gtest',
    main : true,
    required : true,
    fallback : ['gtest', 'gtest_dep']
)

threads = dependency('threads')

agentd_include = include_directories('include')
config_include = include_directories('.')

#add vcblockchain include directories
vcblockchain_sub = subproject('vcblockchain')
vcblockchain_include_directories = \
    vcblockchain_sub.get_variable('vcblockchain_include_directories')

agentd_exe = executable(
    'agentd',
    './src/agentd/main.c',
    src_not_main,
    lfiles,
    pfiles,
    include_directories : [
        agentd_include,
        config_include,
        vcblockchain_include_directories],
    dependencies : [threads, vcblockchain]
)

agentd_test = executable(
    'agentd-test',
    src_not_main, test_src, lfiles, pfiles,
    include_directories : [agentd_include, vcblockchain_include_directories],
    dependencies : [threads, vcblockchain, gtest]
)

test_env = environment()

where_is_the_cat = run_command(
    'which', 'cat', check : true
).stdout().strip().split('\n')

# There is the cat
test_env.set(
    'TEST_BIN',
    where_is_the_cat
)

test_env.set(
    'AGENTD_PATH',
    meson.current_build_dir()
)

test_env.set(
    'TEST_DB_PATH',
    meson.current_build_dir() / 'testdb'
)

message(meson.current_build_dir() / 'testdb')

test(
    'agentd-test',
    agentd_test,
    env : test_env,
    is_parallel : false,
    timeout: 900
)

VERSION=meson.project_version()
package_sh = files('scripts/package.sh')
default_agentd_conf = files('share/default-agentd.conf')

package = custom_target(
    'package',
    input : [agentd_exe, 'share/default-agentd.conf'],
    output : ('agentd-' + VERSION + '.tar.xz'),
    command: [package_sh, meson.project_version(), default_agentd_conf],
    build_by_default : true
)

conf_data = configuration_data()
conf_data.set('VERSION', VERSION)
configure_file(
    input : 'config.h.in',
    output : 'config.h',
    configuration : conf_data
)

#Model checking
run_target(
    'model-check',
    command : ['scripts/model-check.sh', 'models'])
