---@diagnostic disable: undefined-global
add_rules("mode.debug", "mode.release")

if is_plat("windows") then
    add_requires("opengl")
    add_packages("opengl")
    -- add_defines("_MSVC_LANG=202002L")
    add_defines("NOMINMAX")
    add_cxflags("/Zc:preprocessor")
    add_cxflags("/Ehsc")
else

end

set_targetdir("bin")


set_languages("c++20")

-- add_requires("glfw", "glad", "glm")
-- add_packages("glfw", "glad", "glm")


add_requires("imgui docking", {
    configs = {
        shared = false,
        -- debug = true,
        opengl3 = true,
        glfw = true
    }
})
add_packages("imgui")

local function global_packages(...)
    local names = { ... }
    for _, name in ipairs(names) do
        add_packages(name)
        add_requires(name)
    end
end

global_packages(
    "glfw"
    , "glad"
    , "glm"
    , "rang"
)

-- local lfs = require("lfs")
print(cur_dir)



---@format disable
target("yourcraft")
    set_kind("binary")
    add_files("src/**.cpp")
    add_includedirs("src")
target_end()
