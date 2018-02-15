from distutils.core import setup, Extension

extension_mod = Extension('RS_FEC', ['RS_FEC_module.cpp'])

setup(name = 'RS_FEC',
	version = '1.0',
	description = 'This module provides an interface for Reed-Solomon encoding and decoding',
	ext_modules = [extension_mod])

