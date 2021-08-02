# cdef extern from "Python.h":
#     ctypedef struct PyObject:
#         pass
#     long Py_REFCNT(PyObject *)
from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
cdef extern from "tmp.h":
    ctypedef unsigned char uchar
cdef extern from "Rectangle.h" namespace "shapes":
    ctypedef uchar u8
    void print_area(const char *msg) 
    void change(char **buf)   
    cdef cppclass Rectangle:
        Rectangle()except+
        Rectangle(int x0, int y0, int x1, int y1)        
        int getArea()
        void getSize(int *width, int *height)
        
        # void move(int dx, int dy)

cdef class pyRectangle:
    # cdef Rectangle p_Rect    
    cdef Rectangle *p_Rect    
    def __init__(self,w=33,h=77):
        self.p_Rect=new Rectangle(0,0,w,h)
    def getAreaTT(self):
        return self.p_Rect.getArea()
    # def getSize(self):
    #     cdef int w,h        
    #     self.p_Rect.getSize(&w,&h)        
    #     return w,h

def print_area_py():
    cdef char* m="wwwwwwwwwww"
    print_area(m)
# def py_rect_func():
#     pyR=pyRectangle()
#     area=pyR.getArea()
#     w1,h1=pyR.getSize()
#     return w1,h1,area
def test_vector(ba:bytearray,n:int):
    cdef vector[vector[char]] vv
    vv.resize(10)
    cdef int num=vv.size()
    # bytes ch=b'1111'
    # ch=bytes()
    # ch=b'1111'
    cdef char ff[1000]
    string.memset(ff,0,10)
    # string.memcpy(ff,ch,4)
    # ba=bytearray(b'aaa')
    
    cdef char* buf=ba
    string.memcpy(ff,buf,n)
    print("num:",num)
    printf("===>>%s\n",ff)
    by=bytearray(b'222')
    # cdef char* bb=by
    cdef uchar* aa
    # cdef char** aaa=<char**>&aa
    change(<char**>&aa)
    # string.memcpy(bb,aa,5)
    print(by,len(aa),aa[:6],aa)
    by=aa


    