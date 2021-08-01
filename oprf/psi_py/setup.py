# from distutils.core import setup
from Cython.Build import cythonize
from distutils.core import setup, Extension
from Cython.Distutils import build_ext
import os
# train_dir = '/mnt/d/ubuntu/pyproject'
# train_dir = '../../cproject/tool/libOTe/cryptoTools/cryptoTools/Common/'
# datanames = os.listdir(train_dir)
# print("===ls:", datanames)
# for dataname in datanames:
#     if os.path.splitext(dataname)[1] == '.cpp':  # 目录下包含.cpp 的文件
#         print(dataname)


def get_source_files(src_dir, str_fix='.cpp'):
    datanames = os.listdir(src_dir)
    print("===ls:", datanames)
    flist = []
    for dataname in datanames:
        if os.path.splitext(dataname)[1] == str_fix:  # 目录下包含.cpp 的文件
            # print(dataname)
            flist.append(src_dir+'/'+dataname)
    return flist


# setup(ext_modules=cythonize(
#     "rect.pyx",  # our Cython source
#     sources=["Rectangle.cpp"],  # additional source file(s)
#     language="c++",  # generate C++ code
# ))
# 用动态库编译
# ext1 = Extension("rectpy", sources=["rect.pyx"], library_dirs=[
#                  "."], libraries=["rect"], language='c++')
THIRD_INCLUDE = "../../libOTe/cryptoTools/thirdparty/linux"
CRYPTO_INCLUDE = '../../libOTe/cryptoTools'
BLAKE2_DIR = '../../libOTe/cryptoTools/cryptoTools/Crypto/blake2'
# 用源文件编译
src_ot = get_source_files("../ot")
source_files = ["psi.pyx"]
source_files += src_ot

src_miracl = get_source_files(THIRD_INCLUDE+"/miracl/miracl/source", '.c')
source_files += src_miracl
src_common = get_source_files(CRYPTO_INCLUDE+'/cryptoTools/Common')
source_files += src_common

src_blake2 = get_source_files(BLAKE2_DIR, '.c')
source_files += src_blake2

src_cryto = get_source_files(CRYPTO_INCLUDE+'/cryptoTools/Crypto')
source_files += src_cryto


print("===>>source_files:{}\n".format(source_files))
# include
include_dirs = ["../ot", CRYPTO_INCLUDE, BLAKE2_DIR]
include_dirs.append(THIRD_INCLUDE+"/miracl")
include_dirs.append(THIRD_INCLUDE+"/miracl/miracl/include")
# 宏
define_macros = [('ENABLE_MIRACL', None)]
extra_compile_args = ['-std=c++11', '-Wall', '-O2',
                      '-msse3', '-msse2', '-msse4.1', '-maes', '-mpclmul']
ext1 = Extension("oprf_psi",
                 sources=source_files,
                 include_dirs=include_dirs,
                 define_macros=define_macros,
                 extra_compile_args=extra_compile_args,
                 language='c++')
setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=[ext1],
)
