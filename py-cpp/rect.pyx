# cdef extern from "Python.h":
#     ctypedef struct PyObject:
#         pass
#     long Py_REFCNT(PyObject *)
cdef extern from "Rectangle.h" namespace "shapes":
    void print_area(char *msg)
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