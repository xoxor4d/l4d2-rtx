dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

dependencies.load()

workspace "l4d2-rtx"

	startproject "l4d2-rtx"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
	
    configurations { 
        "Debug", 
        "Release"
    }

	platforms "Win32"
	architecture "x86"

	cppdialect "C++20"
	systemversion "latest"
    symbols "On"
    staticruntime "On"

    disablewarnings {
		"4239",
		"4369",
		"4505",
		"4996",
		"6001",
		"6385",
		"6386",
		"26812"
	}

    defines { 
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" 
    }

    filter "platforms:Win*"
		defines {
			"_WINDOWS", 
			"WIN32"
		}
	filter {}

	-- Release

	filter "configurations:Release"
		optimize "Full"

		buildoptions {
			"/GL"
		}

		defines {
			"NDEBUG"
		}
		
		flags { 
            "MultiProcessorCompile", 
            "LinkTimeOptimization", 
            "No64BitChecks",
			"FatalCompileWarnings"
        }
	filter {}

	-- Debug

	filter "configurations:Debug"
		optimize "Debug"

		defines { 
            "DEBUG", 
            "_DEBUG" 
        }

		flags { 
            "MultiProcessorCompile", 
            "No64BitChecks" 
        }
	filter {}

	-- Project

	project "l4d2-rtx"
		kind "SharedLib"
		language "C++"

		linkoptions {
			"/PDBCompress"
		}

		pchheader "std_include.hpp"
		pchsource "src/std_include.cpp"

		files {
			"./src/**.rc",
			"./src/**.hpp",
			"./src/**.cpp",
		}

		includedirs {
			"%{prj.location}/src",
			"./src",
		}

		resincludedirs {
			"$(ProjectDir)src"
		}

        buildoptions { 
            "/Zm100 -Zm100" 
        }

		filter "configurations:Debug or configurations:Release"
			if(os.getenv("L4D2_ROOT")) then
				print ("Setup paths using environment variable 'L4D2_ROOT' :: '" .. os.getenv("L4D2_ROOT") .. "'")
				targetdir(os.getenv("L4D2_ROOT"))
				debugdir (os.getenv("L4D2_ROOT"))
				debugcommand (os.getenv("L4D2_ROOT") .. "/" .. "run-l4d2-rtx.bat")
			end
		filter {}
		
        -- Specific configurations
		flags { 
			"UndefinedIdentifiers" 
		}

		warnings "Extra"

		dependencies.imports()

        group "Dependencies"
            dependencies.projects()
		group ""
	

project "installer"
    kind "ConsoleApp"
	targetname "L4D2-Remix-CompMod-Installer"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"
    targetdir "./bin"
    

	files {
		"./src_installer/**.hpp",
		"./src_installer/**.cpp",
		"./deps/miniz/miniz.c",
		"./deps/miniz/miniz.h",
		"./src_installer/installer.rc",
		"./src_installer/installer.manifest"
	}

	includedirs {
		"%{prj.location}/src_installer",
		"./src_installer",
		"./deps/miniz",
	}

    filter "configurations:Release*"
        optimize "Full"
        flags { "LinkTimeOptimization" }
	filter {}

	flags { "NoManifest" }     -- prevents VS/mt.exe from generating its own

	dependencies.imports()

	group "Dependencies"
		dependencies.projects()
	group ""