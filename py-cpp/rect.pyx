# cdef extern from "Python.h":
#     ctypedef struct PyObject:
#         pass
#     long Py_REFCNT(PyObject *)
from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
cdef extern from "Rectangle.h" namespace "shapes":
    void print_area(char *msg)
    # ctypedef u8 unsigned char
    cdef cppclass Rectangle:
        Rectangle()
        Rectangle(int x0, int y0, int x1, int y1)        
        int getArea()
        void getSize(int *width, int *height)
        void move(int dx, int dy)

cdef class pyRectangle:
    cdef Rectangle *p_Rect    
    def __init__(self):
        self.p_Rect=new Rectangle(0,0,3,7)
    def getArea(self):
        return self.p_Rect.getArea()
    def getSize(self):
        cdef int w,h
        self.p_Rect.getSize(&w,&h)
        return w,h

def print_area_py():
    cdef char* m="wwwwwwwwwww"
    print_area(m)
def py_rect_func():
    pyR=pyRectangle()
    area=pyR.getArea()
    w,h=pyR.getSize()
    return w,h,area
def test_vector():
    cdef vector[vector[char]] vv
    vv.resize(10)
    cdef int num=vv.size()
    # bytes ch=b'1111'
    # ch=bytes()
    # ch=b'1111'
    cdef char ff[10]
    string.memset(ff,0,10)
    # string.memcpy(ff,ch,4)
    ba=bytearray(b'aaa')
    
    cdef char* buf=ba
    string.memcpy(ff,buf,3)
    print("num:",num)
    printf("===>>%s\n",ff)
    