add_rules("mode.debug", "mode.release")

set_warnings("all", "error")
set_targetdir("bin")

target("ced")
    set_kind("binary")
    add_files("src/*.cpp")

--
-- If you want to known more usage about xmake, please see https://xmake.io

