add_rules('mode.debug', 'mode.release')

add_requires('argparse      3.1')
add_requires('nlohmann_json 3.11.3')
add_requires('xxhash        0.8.3')

add_requires('llvm')

--- options

option('symbol-resolver')
    set_default('builtin')
    set_showmenu(true)
    set_description('Select a symbol resolver.')
    set_values('builtin', 'native')

if is_config('symbol-resolver', 'native') then
    add_repositories('liteldev-repo https://github.com/LiteLDev/xmake-repo.git')
    add_requires('preloader 1.12.0')
end

--- global settings

set_languages('c23', 'c++23')
set_warnings('all')

add_includedirs('src')

if is_mode('debug') then 
    add_defines('DI_DEBUG')
end

--- targets

target('askrva')
    set_kind('binary')
    add_files('src/tools/askrva/**.cpp')
    set_pcxxheader('src/pch.h')

    add_packages(
        'argparse',
        'nlohmann_json'
    )

    if is_config('symbol-resolver', 'native') then
        add_packages('preloader')
        add_defines('DI_USE_NATIVE_SYMBOL_RESOLVER')
    end

target('blob-extractor')
    set_kind('binary')
    add_files('src/tools/blob-extractor/**.cpp')
    set_pcxxheader('src/pch.h')

target('dumpsym')
    set_kind('shared')
    add_files('src/tools/dumpsym/**.cpp')
    set_pcxxheader('src/pch.h')
    
    add_packages(
        'llvm'
    )

target('extractsym')
    set_kind('binary')
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
