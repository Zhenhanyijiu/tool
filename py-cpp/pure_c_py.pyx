# cdef extern from "Python.h":
#     ctypedef struct PyObject:
#         pass
#     long Py_REFCNT(PyObject *)
from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
import numpy as np
cdef extern from "pure_c2.h":
    int sumcpp(int,int)

    



def sum_py(a:int,b:int):
    cdef int m=sumcpp(a,b)
    return m
# def py_rect_func():
#     pyR=pyRectangle()
#     area=pyR.getArea()
#     w1,h1=pyR.getSize()
#     return w1,h1,area
