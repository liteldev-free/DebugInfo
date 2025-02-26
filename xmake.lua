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
    add_files('src/askrva/**.cpp')
    add_includedirs('src/askrva')
    
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
    add_files('src/blob-extractor/**.cpp')
    add_includedirs('src/blob-extractor')

target('dumpsym')
    set_kind('shared')
    add_files('src/dumpsym/**.cpp')
    add_includedirs('src/dumpsym')
    
    add_packages('llvm')

target('extractsym')
    set_kind('binary')
    add_files('src/extractsym/**.cpp')
    add_includedirs('src/extractsym')

    add_packages(
        'llvm',
        'argparse'
    )

    add_links('LLVM')

target('makepdb')
    set_kind('binary')
    add_files('src/makepdb/**.cpp')
    add_includedirs('src/makepdb')
    set_pcxxheader('src/pch.h')

    add_packages(
        'llvm',
        'nlohmann_json',
        'argparse'
    )

    add_links('LLVM')
