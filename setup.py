
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

setup(
    name='caser',
    description='A simple light grid-searching framework for Python.',
    version='1.0.0',
    url='https://github.com/shaldokin/caser',
    author='Shaldokin',
    author_email='shaldokin@protonmail.com',
    python_requires='>=3.0',
    install_requires=['cython'],
    classifiers=[
        'Programming Language :: Cython',
        'Programming Language :: Python :: 3 :: Only',
        'License :: OSI Approved :: MIT License',
    ],
    ext_modules=[
        Extension('caser', sources=['caser/caser.pyx'], language='c++',)
    ],
    cmdclass={'build_ext': build_ext}
)

