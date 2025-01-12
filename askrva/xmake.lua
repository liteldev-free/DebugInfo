add_rules('mode.debug', 'mode.release')

set_allowedplats('windows')
set_allowedarchs('x64')

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- from xmake-repo
add_requires('argparse      3.1')
add_requires('nlohmann_json 3.11.3')

-- from liteldev-repo
add_requires('preloader     1.12.0')

target('askrva')
    set_kind('binary')
    add_files('src/**.cpp')
    add_includedirs('src')
    set_warnings('all')
    set_languages('c23', 'c++23')

    add_packages(
        'argparse',
        'nlohmann_json',
        'preloader'
    )

    if is_mode('debug') then 
        add_defines('DEBUG')
    end
