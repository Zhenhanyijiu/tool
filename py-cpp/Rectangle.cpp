#include "Rectangle.h"
#include <stdio.h>
namespace shapes
{

    Rectangle::Rectangle() {}

    Rectangle::Rectangle(int X0, int Y0, int X1, int Y1)
    {
        this->x0 = X0;
        this->y0 = Y0;
        this->x1 = X1;
        this->y1 = Y1;
    }

    int Rectangle::getArea()
    {
        return (x1 - x0) * (y1 - y0);
    }

    void Rectangle::getSize(int *width, int *height)
    {
        // (*width) = x1 - x0;
        // (*height) = y1 - y0;
        *width = x1 - x0;
        *height = y1 - y0;
    }

    void Rectangle::move(int dx, int dy)
    {
        x0 += dx;
        y0 += dy;
        x1 += dx;
        y1 += dy;
    }
    void print_area(const char *msg)
    {
        printf("this is a print function:%s...\n", msg);
    }

}
