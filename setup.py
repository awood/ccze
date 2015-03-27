#!/usr/bin/env python
from distutils.core import setup, Extension

pyccze_module = Extension(
    '_pyccze',
    define_macros=[
        ('__error_t_defined', '1'),
        ('SYSCONFDIR', '"/etc"'),
        ('PATCHLEVEL', '"1"'),
        ('PKGLIBDIR', '"."'),
        ],
    include_dirs=['src', 'swig', '.'],
    libraries=['ncurses', 'pcre'],
    sources=['swig/pyccze_wrap.c',
        'src/ccze.c',
        'src/ccze-builtins.c',
        'src/ccze-compat.c',
        'src/ccze-color.c',
        'src/ccze-plugin.c',
        'src/ccze-wordcolor.c']
)

setup(
    name='pyccze',
    version='0.1',
    author="Alex Wood",
    description="""Bindings for CCZE Python plugins""",
    ext_modules=[pyccze_module],
    py_modules=["pyccze"],
)
