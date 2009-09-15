# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/SConscript,v 1.9 2009/01/23 00:07:12 ecephas Exp $
# Authors: Tracy Usher <usher@SLAC.Stanford.edu>
# Version: OnboardFilter-04-14-02
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('OnboardFilterLib', depsOnly = 1)

libEnv.AppendUnique(CPPDEFINES = ['GLEAM'])
libEnv.AppendUnique(CPPDEFINES = ['__i386'])
libEnv.AppendUnique(CPPDEFINES = ['EFC_FILTER'])
libEnv.AppendUnique(CPPPATH = ['/afs/slac.stanford.edu/g/glast/ground/GLAST_EXT/rhel4_gcc34/obf/B1-1-3/PHY/source/EFC/V4-3-0',
				'/afs/slac/g/glast/ground/GLAST_EXT/redhat4-i686-32bit/../rhel4_gcc34/obf/B1-1-3/CDM/source/GGF_DB/V2-0-0'])

if baseEnv['PLATFORM'] == 'win32':
	libEnv.AppendUnique(CPPDEFINES = ['_WIN32'])

OnboardFilter = libEnv.SharedLibrary('OnboardFilter', listFiles(['src/*.cxx',
						'src/Dll/*.cxx']))

progEnv.Tool('OnboardFilterLib')
test_OnboardFilter = progEnv.GaudiProgram('test_OnboardFilter', listFiles(['src/test/*.cxx']), test=1)
progEnv.Tool('registerObjects', package = 'OnboardFilter', libraries = [OnboardFilter], 
	testApps = [test_OnboardFilter], 
	includes = listFiles(['OnboardFilter/*.h']))




