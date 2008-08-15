# -*- python -*-
# $Header$
# Authors: Tracy Usher <usher@SLAC.Stanford.edu>
# Version: OnboardFilter-04-12-01
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('OnboardFilterLib', depsOnly = 1)

progEnv.AppendUnique(CPPDEFINES = ['GLEAM'])
progEnv.AppendUnique(CPPDEFINES = ['__i386'])
progEnv.AppendUnique(CPPDEFINES = ['EFC_FILTER'])

if baseEnv['PLATFORM'] == 'win32':
	progEnv.AppendUnique(CPPDEFINES = ['_WIN32'])

OnboardFilter = libEnv.SharedLibrary('OnboardFilter', listFiles(['src/*.cxx']) + listFiles(['src/*.h']) + listFiles(['src/Dll/*.cxx']))

progEnv.Tool('OnboardFilterLib')
test_OnboardFilter = progEnv.Program('test_OnboardFilter', listFiles(['src/test/*.cxx']))
progEnv.Tool('registerObjects', package = 'OnboardFilter', libraries = [OnboardFilter], testApps = [test_OnboardFilter], includes = listFiles(['OnboardFilter/*.h']))
