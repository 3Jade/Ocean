#!/usr/bin/python

import csbuild
import platform

csbuild.Toolchain("gcc").SetCxxCommand("clang++")
csbuild.Toolchain("gcc").Compiler().SetCppStandard("c++11")
csbuild.Toolchain("gcc").SetCcCommand("clang")

if platform.system() == "Windows":
	csbuild.SetUserData("subdir", csbuild.GetOption("vs_ver"))
else:
	csbuild.SetUserData("subdir", platform.system())

csbuild.SetOutputDirectory("lib/{project.userData.subdir}/{project.activeToolchainName}/{project.outputArchitecture}/{project.targetName}")
csbuild.SetIntermediateDirectory("Intermediate/{project.userData.subdir}/{project.activeToolchainName}/{project.outputArchitecture}/{project.targetName}/{project.name}")

@csbuild.project("libocean", "libocean/src")
def h2o():
	@csbuild.scope(csbuild.ScopeDef.Final)
	def FinalScope():
		csbuild.AddLibraries("sprawl_string", "sprawl_hash", "sprawl_serialization", "sprawl_time")
		csbuild.AddLibraryDirectories("third-party/sprawl/lib/{project.targetName}")

	@csbuild.scope(csbuild.ScopeDef.All)
	def AllScope():
		csbuild.AddIncludeDirectories("third-party/sprawl/include")

	csbuild.SetOutput("libocean", csbuild.ProjectType.SharedLibrary)

@csbuild.project("h2o", "h2o/src", ["libocean"])
def h2o():
	csbuild.AddLibraries("re2", "sprawl_time")
	csbuild.AddLibraryDirectories("third-party/re2/lib")
	csbuild.AddIncludeDirectories("third-party/re2/include")

	csbuild.SetOutput("h2o", csbuild.ProjectType.Application)

@csbuild.project("ocean", "ocean-vm/src", ["libocean"])
def ocean():
	csbuild.SetOutput("ocean", csbuild.ProjectType.Application)

@csbuild.project("seasnake", "seasnake/src", ["libocean"])
def seasnake():
	csbuild.SetOutput("seasnake", csbuild.ProjectType.SharedLibrary)
