#!lua
solution "L"
	configurations {"Debug", "Release"}

	-- General options
	location("prj/".._ACTION)
	defines {"GLEW_STATIC"}
	flags {"StaticRuntime","C++11"}
	exceptionhandling "Off"
	rtti "Off"

	-- Platform
	configuration {"windows"}
		defines {"L_WINDOWS","WIN32_LEAN_AND_MEAN","NOMINMAX","NOCRYPT"}
	configuration {"linux"}
		defines {"L_UNIX"}

	-- Visual Studio
	configuration {"vs*"}
		includedirs {"ext/include"}
		libdirs {"ext/lib"}
		linkoptions {"/NODEFAULTLIB:libc.lib","/NODEFAULTLIB:msvcrt.lib","/NODEFAULTLIB:libcd.lib","/NODEFAULTLIB:msvcrtd.lib"}
		defines {"_CRT_SECURE_NO_WARNINGS"}
		buildoptions {"/wd4100","/wd4146","/wd4200","/wd4244","/wd4706"}
		characterset "MBCS" -- Don't use UNICODE
		flags {"ExtraWarnings"}
	configuration {"vs*","Debug"}
		linkoptions {"/NODEFAULTLIB:libcmt.lib"}
	configuration {"vs*","Release"}
		linkoptions {"/NODEFAULTLIB:libcmtd.lib"}

	-- GMake
	configuration {"gmake"}
		buildoptions {"-fno-operator-names"}
    links {"GL","GLU","GLEW","X11"}

	-- Sample project (startup)
	project "Sample"
		language "C++"
		files {"src/main.cpp"}
		debugdir "smp"
		targetdir "smp"
		includedirs {".."}
		libdirs {"bin"}
		links {"L"}

		configuration {"Debug"}
			targetname "Ld"
			kind "ConsoleApp"
			objdir("obj/".._ACTION.."/smp/debug")
			defines {"L_DEBUG"}
			flags {"Symbols"}

		configuration {"Release"}
			targetname "L"
			kind "WindowedApp"
			objdir("obj/".._ACTION.."/smp/release")
			flags {"Optimize"}

	-- Library project
	project "L"
		targetdir "bin"
		kind "StaticLib"
		language "C++"
		files {"src/**.h","src/**.cpp"}
		excludes {"src/main.cpp","src/interface/**"} -- Interface files are not to be compiled by the library

		-- Exclude system-specific files
		configuration {"not windows"}
			excludes {"**_win**"}
		configuration {"not linux"}
			excludes {"**_unix**"}
    configuration {}

		-- PCH
    if _ACTION ~= "gmake" then
  		pchheader "pc.h"
  		pchsource "src/pc.cpp"
			forceincludes {"pc.h"}
		else
      pchheader "src/pc.h"
    end

		-- Visual Studio
		configuration {"vs*"}
			postbuildcommands {[[lib.exe /LTCG /NOLOGO /IGNORE:4006,4221 /LIBPATH:../../ext/lib /OUT:"$(TargetPath)" "$(TargetPath)" user32.lib opengl32.lib glew32s.lib ws2_32.lib]]}

		configuration {"Debug"}
			targetdir "bin/debug"
			objdir("obj/".._ACTION.."/debug")
			defines {"L_DEBUG"}
			flags {"Symbols"}

		configuration  {"Release"}
			targetdir "bin/release"
			objdir("obj/".._ACTION.."/release")
			flags {"Optimize"}
