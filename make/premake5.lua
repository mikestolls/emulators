-- emulators build script

rootdir = path.join(path.getdirectory(_SCRIPT), "..")

-- Solution
solution "emulators"
	language "C++"
	system "Windows"
	architecture "x64"
	configurations { "Debug", "Release" }
	location("../_prj/" .. _ACTION)
    debugdir "../data"
    characterset "MBCS"

	defines {
		"_CRT_SECURE_NO_WARNINGS",
	}

    --linkoptions "/opt:ref"
    editandcontinue "on"

    rtti "off"
    exceptionhandling "off"
	
	filter "configurations:Debug*"
		defines { "_DEBUG" }
		-- flags { "FatalWarnings" }
		symbols "on"
	
	filter "configurations:Release*"
		defines { "NDEBUG" }
		optimize "full"

	-- Projects
	project("emulators")
		location("../_prj/" .. _ACTION)
		targetdir "../_build/%{cfg.buildcfg}/%{prj.name}"
		objdir "../_obj/%{cfg.buildcfg}/%{prj.name}"
        kind "ConsoleApp"
	
		cppdialect "c++17"
		
		flags {
			"NoRuntimeChecks",
		}
			
		files {
            "../src/**.h",
			"../src/**.cpp",
			"../data/**"
		}
		excludes {
			"../data/**.s" 
		}
        includedirs {
            "../src",
            "../src/emulators",
			"../include",
			"../lib/argparse",
			"../lib/sfml/include"
        }
        links {
            "opengl32.lib",
            "winmm.lib",
			"freetype.lib",
        }
        defines {
            "SFML_STATIC",
        }

        libdirs {
            "../lib/Exts/msvc"
        }
		
		filter "configurations:Debug*"
			libdirs {
				"../lib/sfml/build/lib/Debug",
			}
			
            links {
                "sfml-audio-s-d.lib",
                "sfml-graphics-s-d.lib",
                "sfml-main-d.lib",
                "sfml-network-s-d.lib",
                "sfml-system-s-d.lib",
                "sfml-window-s-d.lib"
            }

		filter "configurations:Release*"
			libdirs {
				"../lib/sfml/build/lib/Release",
			}
			
            links {
                "sfml-audio-s.lib",
                "sfml-graphics-s.lib",
                "sfml-main.lib",
                "sfml-network-s.lib",
                "sfml-system-s.lib",
                "sfml-window-s.lib"
            }

        -- postbuildcommands {
        --     "copy \"" .. path.translate(path.join(rootdir, "data", "*.*")) .. '" "' ..
        --         path.translate(path.join(rootdir, "_Bin", "%{cfg.platform}", "%{cfg.buildcfg}", "%{prj.name}")) .. '"'
        -- }

		filter "configurations:Win*"
			defines {
				"WIN32",
			}
			