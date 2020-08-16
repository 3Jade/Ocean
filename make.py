#!/usr/bin/python

import csbuild
import platform

with csbuild.Toolchain("msvc"):
	csbuild.AddCompilerCxxFlags("/std:c++17", "/EHsc")


with csbuild.Project("libocean", "libocean/src"):
	with csbuild.Scope(csbuild.ScopeDef.All):
		csbuild.AddIncludeDirectories("third-party/sprawl/include")
		csbuild.AddIncludeDirectories("third-party/include")

	csbuild.SetOutput("libocean", csbuild.ProjectType.StaticLibrary)

with csbuild.Project("libh2o", "libh2o/src"):
	with csbuild.Scope(csbuild.ScopeDef.Final):
		csbuild.AddLibraries("re2")

	with csbuild.Scope(csbuild.ScopeDef.All):
		csbuild.AddLibraryDirectories("third-party/re2/lib")
		csbuild.AddIncludeDirectories("third-party/re2/include")
		csbuild.AddIncludeDirectories("third-party/sprawl/include")
		csbuild.AddIncludeDirectories("third-party/include")

	csbuild.SetOutput("libh2o", csbuild.ProjectType.StaticLibrary)

with csbuild.Project("ocean", "ocean-vm/src", ["libocean", "libh2o"]):
	csbuild.SetOutput("ocean", csbuild.ProjectType.Application)

with csbuild.Project("seasnake", "seasnake/src", ["libocean"]):
	csbuild.SetOutput("seasnake", csbuild.ProjectType.SharedLibrary)
