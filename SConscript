# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/OnboardFilter/SConscript,v 1.13.2.1 2010/10/08 16:39:12 heather Exp $
# Authors: Tracy Usher <usher@SLAC.Stanford.edu>
# Version: OnboardFilter-04-14-04-gr01

Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package='OnboardFilter', toBuild='component')

libEnv.AppendUnique(CPPDEFINES = ['GLEAM'])
libEnv.AppendUnique(CPPDEFINES = ['__i386'])
libEnv.AppendUnique(CPPDEFINES = ['EFC_FILTER'])

## should no longer necessary with new re-organized obf
##libEnv.AppendUnique(CPPPATH = ['/afs/slac.stanford.edu/g/glast/ground/GLAST_EXT/rhel4_gcc34/obf/B1-1-3/PHY/source/EFC/V4-3-0',
##				'/afs/slac/g/glast/ground/GLAST_EXT/redhat4-i686-32bit/../rhel4_gcc34/obf/B1-1-3/CDM/source/GGF_DB/V2-0-0'])

if baseEnv['PLATFORM'] == 'win32':
	libEnv.AppendUnique(CPPDEFINES = ['_WIN32'])

OnboardFilter = libEnv.ComponentLibrary('OnboardFilter',
					listFiles(['src/*.cxx']))

progEnv.Tool('OnboardFilterLib')
test_OnboardFilter = progEnv.GaudiProgram('test_OnboardFilter', listFiles(['src/test/*.cxx']), test=1, package='OnboardBilter')

progEnv.Tool('registerTargets', package = 'OnboardFilter',
	     libraryCxts = [[OnboardFilter, libEnv]], 
	     testAppCxts = [[test_OnboardFilter, progEnv]], 
	     includes = listFiles(['OnboardFilter/*.h']),
	     jo = ['src/test/jobOptions.txt'])




