-- emulators build script

rootdir = path.join(path.getdirectory(_SCRIPT), "..")

-- Solution
solution "emulators"
	language "C++"
	system "Windows"
	architecture "x64"
	configurations { "Debug", "Debug - Assembler", "Debug - Disassembler", "Release" }
	location "../prj"
    debugdir "../data"
    characterset "MBCS"

	defines {
		"_CRT_SECURE_NO_WARNINGS",
	}

    --linkoptions "/opt:ref"
    editandcontinue "on"

    rtti "off"
    exceptionhandling "off"

	configuration "Debug*"
		defines { "_DEBUG" }
		-- flags { "FatalWarnings" }
		symbols "on"
		
	configuration "Debug - Assembler"
		debugargs "-a"

	configuration "Debug - Disassembler"
		debugargs "-d"
		
	configuration "Release*"
		defines { "NDEBUG" }
		optimize "full"

	-- Projects
	project("emulators")
		location("../prj/emulators")
		targetdir "../build/%{cfg.buildcfg}/%{prj.name}"
		objdir "../obj/%{cfg.buildcfg}/%{prj.name}"
        kind "ConsoleApp"
		files {
            "../src/**.h",
			"../src/**.cpp",
		}
        includedirs {
            "../src",
            "../src/emulators",
			"../include",
        }
        links {
            "opengl32.lib",
            "winmm.lib",
        }
        defines {
            "SFML_STATIC",
        }

        libdirs {
            "../lib/msvc",
            "../lib/Exts/msvc"
        }

        configuration "Debug*"
            links {
                "sfml-audio-s-d.lib",
                "sfml-graphics-s-d.lib",
                "sfml-main-d.lib",
                "sfml-network-s-d.lib",
                "sfml-system-s-d.lib",
                "sfml-window-s-d.lib"
            }

        configuration "Release*"
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

		configuration "Win*"
			defines {
				"WIN32",
			}
			flags {
				--"StaticRuntime",
				--"NoMinimalRebuild",
				--"NoIncrementalLink",
			}
			