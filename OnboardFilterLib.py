# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/OnboardFilter/OnboardFilterLib.py,v 1.1 2008/08/15 21:42:36 ecephas Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['OnboardFilter'])
 
    env.Tool('addLibrary', library = ['dl'])
    env.Tool('addLibrary', library = ['pthread'])
    env.Tool('addLibrary', library = env['obfLibs'])
    env.Tool('addLibrary', library = env['gaudiLibs'])
    env.Tool('OnboardFilterTdsLib')
    env.Tool('addLibrary', library = env['rootLibs'])
def exists(env):
    return 1;
