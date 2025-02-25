add_rules('mode.debug', 'mode.release')

set_allowedplats('windows')
set_allowedarchs('x64')

add_repositories('liteldev-repo https://github.com/LiteLDev/xmake-repo.git')

-- from xmake-repo
add_requires('argparse      3.1')
add_requires('nlohmann_json 3.11.3')
add_requires('xxhash')
add_requires('llvm')

-- from liteldev-repo
add_requires('preloader     1.12.0')

target('askrva')
    set_kind('binary')
    add_files('src/askrva/**.cpp')
    add_includedirs('src', 'src/askrva')
    set_warnings('all')
    set_languages('c23', 'c++23')
    set_pcxxheader('src/pch.h')

    add_packages(
        'argparse',
        'nlohmann_json',
        'preloader'
    )

    if is_mode('debug') then 
        add_defines('DEBUG')
    end

target('blob-extractor')
    set_kind('binary')
    add_files('src/blob-extractor/**.cpp')
    add_includedirs('src', 'src/blob-extractor')
    set_warnings('all')
    set_languages('c23', 'c++23')

    if is_mode('debug') then 
        add_defines('DEBUG')
    end

target('dumpsym')
    set_kind('shared')
    add_files('src/dumpsym/**.cpp')
    add_includedirs('src', 'src/dumpsym')
    set_languages('c23', 'c++23')
    
    add_packages('llvm')

    if is_mode('debug') then 
        add_defines('DEBUG')
    end

target('extractsym')
    set_kind('binary')
    add_files('src/extractsym/**.cpp')
    add_includedirs('src', 'src/extractsym')
    set_warnings('all')
    set_languages('c23', 'c++23')

    add_packages(
        'llvm',
        'argparse'
    )

    add_links('LLVM')

    if is_mode('debug') then 
        add_defines('DEBUG')
    end

target('makepdb')
    set_kind('binary')
    add_files('src', 'src/makepdb/**.cpp')
    add_includedirs('src')
    add_includedirs('src/makepdb')
    set_warnings('all')
    set_languages('c23', 'c++23')
    set_pcxxheader('src/pch.h')

    add_packages(
        'llvm',
        'nlohmann_json',
        'argparse'
    )

    add_links('LLVM')

    if is_mode('debug') then 
        add_defines('DEBUG')
    end
