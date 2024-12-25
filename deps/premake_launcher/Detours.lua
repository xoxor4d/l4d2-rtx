Detours = {
	source = path.join(dependencies_launcher.basePath, "Detours"),
}

function Detours.import()
	libdirs { path.join(Detours.source, "lib") }
	Detours.includes()
end

function Detours.includes()
	includedirs {
		path.join(Detours.source, "include")
	}
end

function Detours.project()
    project "Detours"
		language "C++"

		Detours.includes()

		files {
            path.join(Detours.source, "include/*.h"),
		}

		warnings "Off"
		kind "None"

end

table.insert(dependencies_launcher, Detours)
