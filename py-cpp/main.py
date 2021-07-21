import json
from rect import *

if __name__ == '__main__':
    print("mmmmm")
    print("mmmmm")
    print_area_py()
    w, h, a = py_rect_func()

    print("w:{},h:{},a:{}".format(w, h, a))
    pyR = pyRectangle()
    area = pyR.getArea()
    w, h = pyR.getSize()

    print('====w,h,a', w, h, area)
    print('======', 20000)
    print(">>>>>>")
    d = dict()
