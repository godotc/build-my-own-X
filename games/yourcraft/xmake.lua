---@diagnostic disable: undefined-global
add_rules("mode.debug", "mode.release")

if is_plat("windows") then
    add_requires("opengl")
    add_packages("opengl")
end

set_targetdir("bin")


set_languages("cpp17")

add_requires("glfw", "glad", "glm")
add_packages("glfw", "glad", "glm")

add_requires("imgui docking", {
    configs = {
        shared = false,
        debug = true,
        opengl3 = true,
        glfw = true
    }
})
add_packages("imgui")

target("yourcraft")
set_kind("binary")
add_files("src/*.cpp")
