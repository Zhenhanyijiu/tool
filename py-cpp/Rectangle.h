#include "tmp.h"
namespace shapes
{
    typedef uchar u8;
    class Rectangle
    {
    private:
        int x0, y0, x1, y1;

    public:
        Rectangle();
        Rectangle(int x0, int y0, int x1, int y1);
        int getArea();
        void getSize(int *width, int *height);
        void move(int dx, int dy);
    };
    void print_area(const char *msg);
    void change(char** buf);
}
