#!/usr/bin/python

import csbuild
import platform

csbuild.Toolchain("gcc").SetCxxCommand("clang++")
csbuild.Toolchain("gcc").Compiler().SetCppStandard("c++11")
csbuild.Toolchain("gcc").SetCcCommand("clang")
csbuild.Toolchain("gcc").Compiler().AddCompilerFlags("-pthread")

if platform.system() == "Windows":
	csbuild.SetUserData("subdir", csbuild.GetOption("vs_ver"))
else:
	csbuild.SetUserData("subdir", platform.system())

csbuild.SetOutputDirectory("lib/{project.userData.subdir}/{project.activeToolchainName}/{project.outputArchitecture}/{project.targetName}")
csbuild.SetIntermediateDirectory("Intermediate/{project.userData.subdir}/{project.activeToolchainName}/{project.outputArchitecture}/{project.targetName}/{project.name}")

@csbuild.project("libocean", "libocean/src")
def libocean():
	@csbuild.scope(csbuild.ScopeDef.Final)
	def FinalScope():
		csbuild.AddLibraries("sprawl_string", "sprawl_hash", "pthread")
		csbuild.AddLibraryDirectories("third-party/sprawl/lib/{project.targetName}")

	@csbuild.scope(csbuild.ScopeDef.All)
	def AllScope():
		csbuild.AddIncludeDirectories("third-party/sprawl/include")

	csbuild.SetOutput("libocean", csbuild.ProjectType.StaticLibrary)

@csbuild.project("libh2o", "libh2o/src")
def libh2o():
	@csbuild.scope(csbuild.ScopeDef.Final)
	def FinalScope():
		csbuild.AddLibraries("sprawl_string", "sprawl_hash", "re2", "pthread")
		csbuild.AddLibraryDirectories("third-party/sprawl/lib/{project.targetName}")

	@csbuild.scope(csbuild.ScopeDef.All)
	def AllScope():
		csbuild.AddLibraryDirectories("third-party/re2/lib")
		csbuild.AddIncludeDirectories("third-party/re2/include")
		csbuild.AddIncludeDirectories("third-party/sprawl/include")

	csbuild.SetOutput("libh2o", csbuild.ProjectType.StaticLibrary)

@csbuild.project("ocean", "ocean-vm/src", ["libocean", "libh2o"])
def ocean():
	csbuild.SetOutput("ocean", csbuild.ProjectType.Application)

@csbuild.project("seasnake", "seasnake/src", ["libocean"])
def seasnake():
	csbuild.SetOutput("seasnake", csbuild.ProjectType.SharedLibrary)
