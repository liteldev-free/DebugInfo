add_rules('mode.debug', 'mode.release')

add_requires('argparse      3.1')
add_requires('nlohmann_json 3.11.3')
add_requires('xxhash        0.8.3')
add_requires('boost         1.87.0')

add_requires('llvm')

--- options

option('symbol-resolver')
    set_default('builtin')
    set_showmenu(true)
    set_description('Select a symbol resolver.')
    set_values('builtin', 'native')
    before_check(function (option)
        -- the native symbol resolution backend is only available under windows, because liteldev
        -- has not released a linux version.
        if option:value() == 'native' and not is_plat('windows') then 
            raise('the native symbol resolver does not support this platform.')
        end
    end)
option_end()

if is_config('symbol-resolver', 'native') then
    add_repositories('liteldev-repo https://github.com/LiteLDev/xmake-repo.git')
    add_requires('preloader 1.12.0')
end

--- global settings

set_languages('c23', 'c++23')
set_warnings('all')

add_includedirs('src')

set_policy("build.optimization.lto", true)

if is_mode('debug') then 
    add_defines('DI_DEBUG')
end

--- targets

target('libdi')
    set_kind('static')
    add_files('src/**.cpp')
    set_pcxxheader('src/pch.h')
    
    remove_files('src/tools/**')
    set_basename('di')

    add_packages(
        'nlohmann_json'
    )

target('askrva')
    set_kind('binary')
    add_deps('libdi')
    add_files('src/tools/askrva/**.cpp')
    set_pcxxheader('src/pch.h')

    add_packages(
        'argparse',
        'nlohmann_json'
    )

    if is_config('symbol-resolver', 'native') then
        add_packages('preloader')
        add_defines('DI_USE_NATIVE_SYMBOL_RESOLVER=1')
    end

target('blob-extractor')
    set_kind('binary')
    add_deps('libdi')
    add_files('src/tools/blob-extractor/**.cpp')
    set_pcxxheader('src/pch.h')

    add_packages(
        'nlohmann_json',
        'argparse'
    )

target('dumpsym')
    set_kind('shared')
    add_deps('libdi')
    add_files('src/tools/dumpsym/**.cpp')
    set_pcxxheader('src/pch.h')
    
    add_packages(
        'llvm'
    )

target('extractsym')
    set_kind('binary')
    add_deps('libdi')
    add_files('src/tools/extractsym/**.cpp')
    set_pcxxheader('src/pch.h')

    add_packages(
        'llvm',
        'argparse'
    )

    if is_plat('linux') then -- workaround to fix link problem.
        add_links('LLVM')
    end

target('makepdb')
    set_kind('binary')
    add_deps('libdi')
    add_files('src/tools/makepdb/**.cpp')
    set_pcxxheader('src/pch.h')

    add_packages(
        'llvm',
        'nlohmann_json',
        'argparse'
    )

    if is_plat('linux') then
        add_links('LLVM')
    end
