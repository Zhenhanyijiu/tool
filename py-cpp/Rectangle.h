namespace shapes
{
    typedef unsigned char u8;
    class Rectangle
    {
    public:
        int x0, y0, x1, y1;
        Rectangle();
        Rectangle(int x0, int y0, int x1, int y1);
        int getArea();
        void getSize(int *width, int *height);
        void move(int dx, int dy);
    };
    void print_area(char *msg);
}
