#include <FramebufferRambo.h>

void FramebufferRambo::initializeDevice() {
    pinMode(26, OUTPUT); digitalWrite(26, HIGH);
    pinMode(27, OUTPUT); digitalWrite(27, HIGH);
    pinMode(28, OUTPUT); digitalWrite(28, HIGH);
    pinMode(29, OUTPUT); digitalWrite(29, HIGH);
    pinMode(30, OUTPUT); digitalWrite(30, HIGH);
    pinMode(31, OUTPUT); digitalWrite(31, HIGH);
    pinMode(32, OUTPUT); digitalWrite(32, HIGH);
    pinMode(33, OUTPUT); digitalWrite(33, HIGH);
    pinMode(34, OUTPUT); digitalWrite(34, HIGH);
    pinMode(35, OUTPUT); digitalWrite(35, HIGH);
    pinMode(36, OUTPUT); digitalWrite(36, HIGH);
    pinMode(37, OUTPUT); digitalWrite(37, HIGH);
    pinMode(38, OUTPUT); digitalWrite(38, HIGH);
    pinMode(39, OUTPUT); digitalWrite(39, HIGH);
    pinMode(40, OUTPUT); digitalWrite(40, HIGH);
    pinMode(41, OUTPUT); digitalWrite(41, HIGH);

    _spi.begin();
    _spi.setSpeed(15000000);
        
    fillScreen(0);
}

void FramebufferRambo::writePage() {
    int page = _boff << 1;
    int start = page % 131072;
    int chip = page / 131072;

    digitalWrite(26 + chip, LOW);
    _spi.transfer((uint8_t)0x02);
    _spi.transfer((uint8_t)((start >> 16) & 0xFF));
    _spi.transfer((uint8_t)((start >> 8) & 0xFF));
    _spi.transfer((uint8_t)(start & 0xFF));

    for (int x = 0; x < 8192; x++) {
        uint8_t b1 = _buf[x] >> 8;
        uint8_t b2 = _buf[x] & 0xFF;
        _spi.transfer(b1);
        _spi.transfer(b2);
    }
    digitalWrite(26 + chip , HIGH);
}

void FramebufferRambo::readPage() {
    int page = _boff << 1;
    int start = page % 131072;
    int chip = page / 131072;

    digitalWrite(26 + chip, LOW);
    _spi.transfer((uint8_t)0x03);
    _spi.transfer((uint8_t)((start >> 16) & 0xFF));
    _spi.transfer((uint8_t)((start >> 8) & 0xFF));
    _spi.transfer((uint8_t)(start & 0xFF));

    for (int x = 0; x < 8192; x++) {
        uint8_t b1 = _spi.transfer((uint8_t)0x00);
        uint8_t b2 = _spi.transfer((uint8_t)0x00);
        _buf[x] = (b1 << 8) | b2;
    }
    digitalWrite(26 + chip , HIGH);
}

uint16_t FramebufferRambo::read(int address) {
    if (address >= _boff && address < _boff + 8192) {
        return _buf[address - _boff];
    }
    writePage();
    _boff = address % 0xFFFFE000;
    readPage();
    return _buf[address - _boff];
}

void FramebufferRambo::write(int address, uint16_t val) {
    if (address >= _boff && address < _boff + 8192) {
        _buf[address - _boff] = val;
        return;
    }
    writePage();
    _boff = address % 0xFFFFE000;
    readPage();
    _buf[address - _boff] = val;
}

void FramebufferRambo::setPixel(int x, int y, color_t color) {
    translateCoordinates(&x, &y);

    if (x < 0 || x >= _width || y < 0 || y >= _height) {
        return;
    }
    write(x + y * _width, color);
}

void FramebufferRambo::fillScreen(color_t color) {
    for (int x = 0; x < _width * _height; x++) {
        write(x, color);
    }
}

void FramebufferRambo::draw(Cariad *dev, int x, int y) {
    if (_filter != NULL) {
        uint32_t p = 0;
        color_t line[getWidth()];
        for (int py = 0; py < getHeight(); py++) {
            for (int px = 0; px < getWidth(); px++) {
                line[px] = _filter->process(read(p));
                p++;
            }
            dev->openWindow(x, y + py, getWidth(), 1);
            dev->windowData(line, getWidth());
            dev->closeWindow();
        }
    } else {
//        writePage();
        dev->openWindow(x, y, getWidth(), getHeight());
//        int chip = 0;
//        int offset = 0;
//        digitalWrite(26 + chip, LOW);
//        _spi.transfer((uint8_t)0x03);
//        _spi.transfer((uint8_t)0);
//        _spi.transfer((uint8_t)0);
//        _spi.transfer((uint8_t)0);
        for (int i = 0; i < (_width * _height); i++) {
//            uint8_t b1 = _spi.transfer((uint8_t)0x00);
//            uint8_t b2 = _spi.transfer((uint8_t)0x00);
//            offset++;
//            offset++;
//            offset = offset % 131072;
//            if (offset == 0) {
//                digitalWrite(26 + chip , HIGH);
//                chip++;
//                digitalWrite(26 + chip, LOW);
//                _spi.transfer((uint8_t)0x03);
//                _spi.transfer((uint8_t)0);
//                _spi.transfer((uint8_t)0);
//                _spi.transfer((uint8_t)0);
//            } 
//            dev->windowData((b1 << 8) | b2);
            dev->windowData(read(i));
        }
//        digitalWrite(26 + chip , HIGH);
        dev->closeWindow();
    }
}

void FramebufferRambo::draw(Cariad *dev, int x, int y, color_t t) {
    uint32_t p = 0;
    color_t line[getWidth()];

    for (int py = 0; py < getHeight(); py++) {
        bool haveTrans = false;
        for (int px = 0; px < getWidth(); px++) {
            color_t col = read(p);
            if (col == t) {
                haveTrans = true;
                line[px] = col;
            } else {
                if (_filter != NULL) {
                    col = _filter->process(col);
                }
                line[px] = col;
            }
            p++;
        }
        if (!haveTrans) {
            dev->openWindow(x, y + py, getWidth(), 1);
            dev->windowData(line, getWidth());
            dev->closeWindow();
        } else {
            for (int px = 0; px < getWidth(); px++) {
                if (read(py * getWidth() + px) != t) {
                    dev->setPixel(x + px, y + py, line[px]);
                }
            }
        }
    }
}

void FramebufferRambo::drawTransformed(Cariad *dev, int x, int y, int transform) {
    uint32_t p = 0;
    for (int py = 0; py < getHeight(); py++) {
        for (int px = 0; px < getWidth(); px++) {
            switch (transform) {
                default:
                    dev->setPixel(x + px, y + py, read(p));
                    break;
                case MirrorH:
                    dev->setPixel(getWidth() - (x + px) - 1, y + py, read(p));
                    break;
                case MirrorV:
                    dev->setPixel(x + px, getHeight() - (y + py) - 1, read(p));
                    break;
                case Rotate180:
                    dev->setPixel(getWidth() - (x + px) - 1, getHeight() - (y + py) - 1, read(p));
                    break;
            }
            p++;
        }
    }
}

void FramebufferRambo::drawTransformed(Cariad *dev, int x, int y, int transform, color_t t) {
    uint32_t p = 0;
    for (int py = 0; py < getHeight(); py++) {
        for (int px = 0; px < getWidth(); px++) {
            color_t col = read(p);
            if (col != t) {
                switch (transform) {
                    default:
                        dev->setPixel(x + px, y + py, col);
                        break;
                    case MirrorH:
                        dev->setPixel(getWidth() - (x + px) - 1, y + py, col);
                        break;
                    case MirrorV:
                        dev->setPixel(x + px, getHeight() - (y + py) - 1, col);
                        break;
                    case Rotate180:
                        dev->setPixel(getWidth() - (x + px) - 1, getHeight() - (y + py) - 1, col);
                        break;
                }
            }
            p++;
        }
    }
}
