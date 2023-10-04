add_requires("fmt", "gtest")
set_targetdir("bin")
set_languages("c++20")


target("db")
	set_kind("binary")
	add_files("src/**.cc")
	add_packages("fmt")


target("ts")
	set_kind("binary")
	add_files("test/*.cc")
	add_packages("gtest","fmt")
