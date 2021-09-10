# from distutils.core import setup
from Cython.Build import cythonize
from distutils.core import setup, Extension
from Cython.Distutils import build_ext
import os

# 路径
THIRD_INCLUDE = "../../libOTe/cryptoTools/thirdparty/linux"
CRYPTO_INCLUDE = '../../libOTe/cryptoTools'
BLAKE2_DIR = '../../libOTe/cryptoTools/cryptoTools/Crypto/blake2'


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


# setup(ext_modules=cythonize(
#     "rect.pyx",  # our Cython source
#     sources=["Rectangle.cpp"],  # additional source file(s)
#     language="c++",  # generate C++ code
# ))
# 用动态库编译
# ext1 = Extension("rectpy", sources=["rect.pyx"], library_dirs=[
#                  "."], libraries=["rect"], language='c++')

# 编译静态库libmiracl
def compile_miracl():
    miracl_path = THIRD_INCLUDE + "/miracl/miracl/source/"
    cmd = 'cd ' + miracl_path + " && g++ -fPIC -shared -c -m64 -O3 ./*.c -I../include " + \
          " && ar rc libmiracl.a ./*.o" + " && rm -rf ./*.o" + " &&cd -"
    os.system(cmd)


# cython编译
def start_setup():
    # 用源文件编译,获取源文件列表
    source_files = []
    source_files += ["oprf_psi.pyx"]
    src_ot = get_source_files("../ot")
    source_files += src_ot
    # src_miracl = get_source_files(THIRD_INCLUDE + "/miracl/miracl/source", '.c')
    # source_files += src_miracl
    src_common = get_source_files(CRYPTO_INCLUDE + '/cryptoTools/Common')
    source_files += src_common
    src_blake2 = get_source_files(BLAKE2_DIR, '.c')
    source_files += src_blake2
    src_cryto = get_source_files(CRYPTO_INCLUDE + '/cryptoTools/Crypto')
    source_files += src_cryto
    # source_files.append('/mnt/d/ubuntu/cproject/tool/libOTe/cryptoTools/thirdparty/linux/miracl/miracl/source/libmiracl.a')
    # print("===>>source_files:{}\n".format(source_files))
    # 获取头文件路径路径，-I
    include_dirs = ["../ot", CRYPTO_INCLUDE, BLAKE2_DIR]
    include_dirs.append(THIRD_INCLUDE + "/miracl")
    include_dirs.append(THIRD_INCLUDE + "/miracl/miracl/include")
    # 宏定义
    define_macros = [('ENABLE_MIRACL', None), ('OMP_POOL', None), ('NDEBUG', None)]
    # 其他编译条件，-std=gnu++11,-std=c++11
    extra_compile_args = ['-std=c++11', '-Wall', '-O2', '-fopenmp',
                          '-msse3', '-msse2', '-msse4.1', '-maes', '-mpclmul']
    # 链接omp
    extra_link_args = ['-lgomp']
    # 依赖库路径，-L
    library_dirs = [THIRD_INCLUDE + "/miracl/miracl/source/"]
    # 库名，-l
    libraries = ['miracl', 'pthread']
    ext1 = Extension("oprf_psi",
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


if __name__ == '__main__':
    print('======== 编译miracl库开始 ========')
    compile_miracl()
    print('======== 编译cython 接口开始 ========')
    start_setup()
    print('======== 删除编译中产生的临时文件 ========')
    os.system("rm -rf build/ libOTe/ oprf_psi.cpp")
