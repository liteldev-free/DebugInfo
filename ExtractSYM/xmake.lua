add_rules('mode.debug', 'mode.release')

add_requires('llvm')
add_requires('argparse 3.1')

target('extractsym')
    set_kind('binary')
    add_files('src/**.cpp')
    add_includedirs('src')
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
