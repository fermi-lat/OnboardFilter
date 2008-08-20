# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/OnboardFilter/SConscript,v 1.1 2008/08/15 21:22:43 ecephas Exp $
# Authors: Tracy Usher <usher@SLAC.Stanford.edu>
# Version: OnboardFilter-04-12-01
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('OnboardFilterLib', depsOnly = 1)

libEnv.AppendUnique(CPPDEFINES = ['GLEAM'])
libEnv.AppendUnique(CPPDEFINES = ['__i386'])
libEnv.AppendUnique(CPPDEFINES = ['EFC_FILTER'])

if baseEnv['PLATFORM'] == 'win32':
	libEnv.AppendUnique(CPPDEFINES = ['_WIN32'])

OnboardFilter = libEnv.SharedLibrary('OnboardFilter', listFiles(['src/*.cxx','src/*.h',
						'src/Dll/*.cxx']))

progEnv.Tool('OnboardFilterLib')
test_OnboardFilter = progEnv.Program('test_OnboardFilter', listFiles(['src/test/*.cxx']))
progEnv.Tool('registerObjects', package = 'OnboardFilter', libraries = [OnboardFilter], 
	testApps = [test_OnboardFilter], 
	includes = listFiles(['OnboardFilter/*.h']))
