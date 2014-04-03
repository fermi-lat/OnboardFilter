# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/SConscript,v 1.36 2014/03/05 21:49:04 heather Exp $
# Authors: Tracy Usher <usher@SLAC.Stanford.edu>
# Version: OnboardFilter-04-18-04

Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package='OnboardFilter', toBuild='component')

libEnv.AppendUnique(CPPDEFINES = ['GLEAM'])
libEnv.AppendUnique(CPPDEFINES = ['__i386'])
libEnv.AppendUnique(CPPDEFINES = ['EFC_FILTER'])
if 'obfdynlddict' in libEnv:
    for k in libEnv['obfdynlddict']:
        defstring = k +'=' + '\\"' + libEnv['obfdynlddict'][k] + '\\"'
        #print 'adding a DEFINE: ', defstring
	libEnv.AppendUnique(CPPDEFINES = defstring)

# CPPDEFINE of obf version has been moved to containerSettings/package.scons
#vstring = 'OBF_' + (str(baseEnv['obfversion'])).replace('-', '_')
#while vstring.count('_') > 3:
#    ix = vstring.rfind('_')
#    vstring = vstring[:ix]
    
#libEnv.AppendUnique(CPPDEFINES = vstring)

if baseEnv['PLATFORM'] == 'win32':
	libEnv.AppendUnique(CPPDEFINES = ['_WIN32'])

cxx = listFiles(['src/*.cxx'])

toRemove = listFiles(['src/*B1-0*.cxx', 'src/*B1-1-0*.cxx',
                      'src/*B1-1-2*.cxx'])

for r in toRemove :
    cxx.remove(r)

if baseEnv['obfversion'][:6] == 'B1-1-3' :
    for r in listFiles(['src/*B3-*.cxx']) :
        cxx.remove(r)

OnboardFilter = libEnv.ComponentLibrary('OnboardFilter', cxx  )

progEnv.Tool('OnboardFilterLib')
test_OnboardFilter = progEnv.GaudiProgram('test_OnboardFilter', listFiles(['src/test/*.cxx']), test=1, package='OnboardFilter')

progEnv.Tool('registerTargets', package = 'OnboardFilter',
	     libraryCxts = [[OnboardFilter, libEnv]], 
	     testAppCxts = [[test_OnboardFilter, progEnv]], 
	     includes = listFiles(['OnboardFilter/*.h']),
	     jo = ['src/test/jobOptions.txt'])




