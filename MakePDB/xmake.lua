add_rules('mode.debug', 'mode.release')

add_requires('llvm')
add_requires('nlohmann_json 3.11.3')
add_requires('argparse      3.1')

target('makepdb')
    set_kind('binary')
    add_files('src/**.cpp')
    add_includedirs('src')
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
