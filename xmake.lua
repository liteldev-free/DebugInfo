add_rules('mode.debug', 'mode.release')

add_requires('argparse      3.2')
add_requires('nlohmann_json 3.12.0')
add_requires('xxhash        0.8.3')
add_requires('libllvm       19.1.7')
add_requires('magic_enum    0.9.7')
add_requires('openssl3      3.5.1')
add_requires('boost         1.88.0', {
    system = false, 
    configs = {
        filesystem = false, 
        stacktrace = true
    }
})

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

        -- TODO: remove it.
        --   The native symbol resolver is completely unavailable,
        --   v1.12.0 Problem: RVA not subtracted from ImageBase.
        --   v1.13.0 Problem: pl_resolve_symbol does not return the real function address, and needs to handle thunk.
        if option:value() == 'native' then
            raise('sorry use builtin symbol resolver please.')
        end
    end)
option_end()

if is_config('symbol-resolver', 'native') then
    add_repositories('liteldev-free-repo https://github.com/liteldev-free/xmake-repo.git')

    -- TODO: need to add an option to select the PreLoader version.
    add_requires('preloader 1.13.0')
end

--- global settings

set_languages('c23', 'c++23')
set_warnings('all')

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

    if is_plat('linux') then
        add_cxflags('-fPIC')
    end

    add_packages('xxhash')
    add_packages(
        'libllvm',
        'boost',
        'nlohmann_json',
        'openssl3',
        'magic_enum',
        {public = true}
    )

    add_includedirs('src', {public = true})

    -- now use boost::stacktrace, due to MacOSX (libc++) compatibility issues.
    -- ~~workaround to fix std::stacktrace link problem~~
    --
    -- for gcc == 14
    -- see https://gcc.gnu.org/onlinedocs/gcc-14.2.0/libstdc++/manual/manual/using.html
    -- for gcc == 13
    -- see https://gcc.gnu.org/onlinedocs/gcc-13.2.0/libstdc++/manual/manual/using.html
    -- if is_plat('linux') then
    --     add_links('stdc++exp')
    -- end

    if is_plat('linux') or is_plat('macosx') then
        add_defines('BOOST_STACKTRACE_USE_ADDR2LINE=1', {public = true})
    end

    -- workaround to fix boost problem
    -- see https://github.com/boostorg/stacktrace/issues/88
    if is_plat('macosx') then
        add_defines('_GNU_SOURCE', {public = true})
    end

    -- to fix llvm link problem
    -- see https://stackoverflow.com/questions/53805007/compilation-failing-on-enableabibreakingchecks
    add_defines('LLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1', {public = true})

target('askrva')
    set_kind('binary')
    add_files('src/tools/askrva/**.cpp')
    set_pcxxheader('src/pch.h')

    add_deps('libdi')
    add_packages('argparse')

    if is_config('symbol-resolver', 'native') then
        add_packages('preloader')
        add_defines('DI_USE_NATIVE_SYMBOL_RESOLVER=1')
    end

target('blob-extractor')
    set_kind('binary')
    add_files('src/tools/blob-extractor/**.cpp')
    set_pcxxheader('src/pch.h')

    add_deps('libdi')
    add_packages(
        'argparse',
        'nlohmann_json'
    )

target('dumpsym')
    set_kind('shared')
    add_files('src/tools/dumpsym/**.cpp')
    set_pcxxheader('src/pch.h')
    add_deps('libdi')

target('extractsym')
    set_kind('binary')
    add_files('src/tools/extractsym/**.cpp')
    set_pcxxheader('src/pch.h')

    add_deps('libdi')
    add_packages('argparse')

target('makepdb')
    set_kind('binary')
    add_files('src/tools/makepdb/**.cpp')
    set_pcxxheader('src/pch.h')

    add_deps('libdi')
    add_packages('argparse')
