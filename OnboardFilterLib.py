# $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/OnboardFilterLib.py,v 1.4 2010/10/01 23:35:33 jrb Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['OnboardFilter'])
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
def exists(env):
    return 1;
