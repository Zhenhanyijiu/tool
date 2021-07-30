# from distutils.core import setup
from Cython.Build import cythonize
from distutils.core import setup, Extension
from Cython.Distutils import build_ext
# import os
# train_dir = '/mnt/d/ubuntu/pyproject'
# train_dir = '../../cproject/tool/libOTe/cryptoTools/cryptoTools/Common/'
# datanames = os.listdir(train_dir)
# print("===ls:", datanames)
# for dataname in datanames:
#     if os.path.splitext(dataname)[1] == '.cpp':  # 目录下包含.cpp 的文件
#         print(dataname)


# setup(ext_modules=cythonize(
#     "rect.pyx",  # our Cython source
#     sources=["Rectangle.cpp"],  # additional source file(s)
#     language="c++",  # generate C++ code
# ))
# 用动态库编译
# ext1 = Extension("rectpy", sources=["rect.pyx"], library_dirs=[
#                  "."], libraries=["rect"], language='c++')
# 用源文件编译
ext1 = Extension("rect", sources=["rect.pyx",
                                  "Rectangle.cpp"], language='c++')
setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=[ext1],
)
