# from distutils.core import setup
from Cython.Build import cythonize
from distutils.core import setup, Extension
from Cython.Distutils import build_ext
import os


# 获取源文件列表方法
def get_source_files(src_dir, str_fix='.cpp'):
    datanames = os.listdir(src_dir)
    # print("===ls:", datanames)
    flist = []
    for dataname in datanames:
        if os.path.splitext(dataname)[1] == str_fix:  # 目录下包含.cpp 的文件
            # print(dataname)
            flist.append(src_dir + '/' + dataname)
    return flist


# cython编译
def start_setup():
    # 用源文件编译,获取源文件列表
    source_files = []
    source_files += ["psi3.pyx"]
    # cryptotools
    source_files += get_source_files("../cryptoTools/Network")
    source_files += get_source_files("../cryptoTools/Crypto")
    source_files += get_source_files("../cryptoTools/Common")
    # libote
    source_files += get_source_files("../libOTe/TwoChooseOne")
    source_files += get_source_files("../libOTe/NChooseOne")
    source_files += get_source_files("../libOTe/Base")
    source_files += get_source_files("../libOTe/Tools")
    # liboprf
    source_files += get_source_files("../libOPRF/OPPRF")
    source_files += get_source_files("../libOPRF/Hashing")
    # psi3
    source_files += get_source_files("../psi3")
    # libPaXoS
    source_files += ['../libPaXoS/ObliviousDictionary.cpp',
                     '../libPaXoS/gf2e_mat_solve.cpp',
                     ]

    # 获取头文件路径路径，-I
    include_dirs = ["../psi3",
                    '../cryptoTools',
                    '../thirdparty/linux/boost/includes',
                    '../libOPRF',
                    '../libOTe',
                    '../libPaXoS',
                    '../thirdparty/linux/miracl',
                    '../thirdparty/linux/ntl/include',
                    '/tmp/include']

    # 宏定义
    define_macros = [('NDEBUG', None), ('NO_INTEL_ASM_SHA1', 1)]
    # 其他编译条件，-std=gnu++11,-std=c++11
    extra_compile_args = ['-g', '-std=c++11', '-Wall', '-O2', '-pthread',
                          '-msse3', '-msse2', '-msse4.1', '-maes', '-mpclmul']

    # 链接omp
    extra_link_args = ['-llinbox', '-lopenblas',
                       '-lgivaro', '-lgmp']
    # extra_link_args = ['-llinbox', '-lopenblas',
    #                    '-lgivaro',  '-lgomp','-lgmpxx']
    # ../libPaXoS/xxHash/libxxhash.a
    # 依赖库路径，-L
    library_dirs = ['../thirdparty/linux/boost/stage/lib',
                    '../thirdparty/linux/miracl/miracl/source',
                    '../thirdparty/linux/ntl/src',
                    '../libPaXoS/xxHash',
                    '/tmp/lib']
    # 库名，-l
    libraries = ['xxhash', 'miracl', 'ntl', 'boost_system']
    ext1 = Extension("psi3",
                     sources=source_files,
                     include_dirs=include_dirs,
                     define_macros=define_macros,
                     library_dirs=library_dirs,
                     libraries=libraries,
                     extra_compile_args=extra_compile_args,
                     extra_link_args=extra_link_args,
                     language='c++')
    setup(
        cmdclass={'build_ext': build_ext},
        ext_modules=cythonize(ext1),
    )


# python3 setup.py build_ext --inplace
if __name__ == '__main__':
    print('======== 编译 psi3 cython 接口开始 ========')
    start_setup()
    print('======== 删除编译中产生的临时文件 ========')
    os.system("rm -rf build/ psi3.cpp")
