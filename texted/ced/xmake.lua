add_rules("mode.debug", "mode.release")

set_warnings("all")--, "error")
set_targetdir("bin")

target("ced")
    set_kind("binary")
    add_files("src/*.cpp")


