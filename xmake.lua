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

set_policy("build.optimization.lto", true)

set_languages('c23', 'c++23')
set_warnings('all')

add_includedirs('src')

-- workaround to fix std::stacktrace link problem
-- for gcc == 14
-- see https://gcc.gnu.org/onlinedocs/gcc-14.2.0/libstdc++/manual/manual/using.html
-- for gcc == 13
-- see https://gcc.gnu.org/onlinedocs/gcc-13.2.0/libstdc++/manual/manual/using.html
if is_plat('linux') then
    add_links('stdc++exp')
end

if is_mode('debug') then 
    add_defines('DI_DEBUG')

    -- to fix llvm link problem
    -- see https://stackoverflow.com/questions/53805007/compilation-failing-on-enableabibreakingchecks
    add_defines('LLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1')
end

--- targets

target('libdi')
    set_kind('static')
    add_files('src/**.cpp')
    set_pcxxheader('src/pch.h')
    
    remove_files('src/tools/**')
    set_basename('di')

    add_packages(
        'xxhash',
        'nlohmann_json',
        'llvm'
    )

    if is_plat('linux') then -- workaround to fix link problem.
        add_links('LLVM')
    end

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
        'nlohmann_json',
        'argparse'
    )

target('makepdb')
    set_kind('binary')
    add_deps('libdi')
    add_files('src/tools/makepdb/**.cpp')
    set_pcxxheader('src/pch.h')

    add_packages(
        'nlohmann_json',
        'argparse'
    )
