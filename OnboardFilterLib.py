# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/OnboardFilter/OnboardFilterLib.py,v 1.5 2012/04/26 21:47:24 jrb Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        if env['PLATFORM']=='win32' and env.get('CONTAINERNAME','')=='GlastRelease':
	    env.Tool('findPkgPath', package = 'OnboardFilter') 

    #    env.Tool('addLibrary', library = ['OnboardFilter'])
    if not env['PLATFORM']  == 'win32':
        env.Tool('addLibrary', library = ['dl'])
        env.Tool('addLibrary', library = ['pthread'])
    env.Tool('addLibrary', library = env['obfLibs'])
    env.Tool('addLibrary', library = env['gaudiLibs'])
    env.Tool('OnboardFilterTdsLib')
    env.Tool('MootSvcLib')
    env.Tool('CalibDataLib')
    env.Tool('LdfEventLib')
    env.Tool('EventLib')
    env.Tool('facilitiesLib')
    env.Tool('addLibrary', library = env['rootLibs'])
    if env['PLATFORM']=='win32' and env.get('CONTAINERNAME','')=='GlastRelease':
        env.Tool('findPkgPath', package = 'EbfWriter') 
        env.Tool('findPkgPath', package = 'GlastSvc') 

def exists(env):
    return 1;
