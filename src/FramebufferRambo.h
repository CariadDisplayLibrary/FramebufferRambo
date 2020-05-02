#ifndef _FRAMEBUFFERRAMBO_H
#define _FRAMEBUFFERRAMBO_H

#include <Cariad.h>
#include <DSPI.h>

class FramebufferRambo : public Image {
    private:
        int _offset;

        uint16_t read(int address);
        void write(int address, uint16_t value);

        DSPI1 _spi;

        color_t _buf[8192]; // 16kb buffer
        int _boff;
        void writePage();
        void readPage();

    public:
        FramebufferRambo(int w, int h, int offset = 0) {
            _width = w;
            _height = h;
            _offset = offset;
            _boff = 0;
        }

        void initializeDevice();

        void setPixel(int x, int y, color_t c);
        void fillScreen(color_t c);

        void draw(Cariad *dev, int x, int y);
        void draw(Cariad *dev, int x, int y, color_t t);
        void drawTransformed(Cariad *dev, int x, int y, int transform);
        void drawTransformed(Cariad *dev, int x, int y, int transform, color_t t);

        void draw(Cariad &dev, int x, int y) { draw(&dev, x, y); }
        void draw(Cariad &dev, int x, int y, color_t t) { draw(&dev, x, y, t); }
        void drawTransformed(Cariad &dev, int x, int y, int transform) { drawTransformed(&dev, x, y, transform); }
        void drawTransformed(Cariad &dev, int x, int y, int __attribute__((unused)) transform, color_t t) { drawTransformed(&dev, x, y, t); }


};

#endif
