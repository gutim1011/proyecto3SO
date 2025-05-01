#include "image.h"
#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

constexpr double PI = 3.14159265358979323846;

ImageProcessor::ImageProcessor(bool useBuddy, BuddyAllocator* allocator)
    : imageData(nullptr), width(0), height(0), channels(0),
      useBuddySystem(useBuddy), allocator(allocator) {}

ImageProcessor::~ImageProcessor() {
    deallocateImage();
}

void ImageProcessor::allocateImage(int w, int h, int c) {
    deallocateImage();
    width = w;
    height = h;
    channels = c;

    size_t imageSize = static_cast<size_t>(width * height * channels);
    if (useBuddySystem && allocator) {
        imageData = static_cast<unsigned char*>(allocator->allocate(imageSize));
    } else {
        imageData = new unsigned char[imageSize];
    }
}

void ImageProcessor::deallocateImage() {
    if (!imageData) return;

    if (useBuddySystem && allocator) {
        allocator->deallocate(imageData);
    } else {
        delete[] imageData;
    }
    imageData = nullptr;
}

bool ImageProcessor::loadImage(const std::string& filename) {
    deallocateImage();
    int w, h, c;
    unsigned char* loaded = stbi_load(filename.c_str(), &w, &h, &c, 0);

    if (!loaded) {
        std::cerr << "Error: Failed to load image: " << filename << std::endl;
        return false;
    }

    allocateImage(w, h, c);
    std::memcpy(imageData, loaded, static_cast<size_t>(w * h * c));
    stbi_image_free(loaded);
    return true;
}

bool ImageProcessor::saveImage(const std::string& filename) {
    if (!imageData) {
        std::cerr << "Error: No image data to save." << std::endl;
        return false;
    }

    std::string ext = filename.substr(filename.find_last_of('.') + 1);
    bool result = false;

    if (ext == "jpg" || ext == "jpeg") {
        result = stbi_write_jpg(filename.c_str(), width, height, channels, imageData, 95);
    } else if (ext == "png") {
        result = stbi_write_png(filename.c_str(), width, height, channels, imageData, width * channels);
    } else if (ext == "bmp") {
        result = stbi_write_bmp(filename.c_str(), width, height, channels, imageData);
    } else {
        std::cerr << "Error: Unsupported file format: " << ext << std::endl;
    }

    return result;
}

void ImageProcessor::getImageInfo(int& w, int& h, int& c) const {
    w = width;
    h = height;
    c = channels;
}

// ----- Métodos auxiliares -----

unsigned char* ImageProcessor::getPixel(unsigned char* data, int x, int y, int c, int w, int h) {
    x = std::clamp(x, 0, w - 1);
    y = std::clamp(y, 0, h - 1);
    return &data[(y * w + x) * channels + c];
}

void ImageProcessor::setPixel(unsigned char* data, int x, int y, int c, unsigned char value, int w, int h) {
    if (x >= 0 && x < w && y >= 0 && y < h) {
        data[(y * w + x) * channels + c] = value;
    }
}

unsigned char ImageProcessor::bilinearInterpolation(unsigned char* data, double x, double y, int c, int w, int h) {
    int x1 = static_cast<int>(x), y1 = static_cast<int>(y);
    int x2 = x1 + 1, y2 = y1 + 1;
    double xFrac = x - x1, yFrac = y - y1;

    unsigned char p1 = *getPixel(data, x1, y1, c, w, h);
    unsigned char p2 = *getPixel(data, x2, y1, c, w, h);
    unsigned char p3 = *getPixel(data, x1, y2, c, w, h);
    unsigned char p4 = *getPixel(data, x2, y2, c, w, h);

    double interpTop = p1 * (1 - xFrac) + p2 * xFrac;
    double interpBottom = p3 * (1 - xFrac) + p4 * xFrac;
    return static_cast<unsigned char>(interpTop * (1 - yFrac) + interpBottom * yFrac);
}

// ----- Transformaciones -----

void ImageProcessor::rotateImage(double angleDegrees) {
    if (!imageData) return;

    double radians = angleDegrees * PI / 180.0;
    double cosA = std::cos(radians), sinA = std::sin(radians);
    int newW = static_cast<int>(width * std::abs(cosA) + height * std::abs(sinA));
    int newH = static_cast<int>(width * std::abs(sinA) + height * std::abs(cosA));
    size_t newSize = static_cast<size_t>(newW * newH * channels);

    unsigned char* newImage;
    if (useBuddySystem && allocator) {
        newImage = static_cast<unsigned char*>(allocator->allocate(newSize));
    } else {
        newImage = new unsigned char[newSize];
    }

    std::memset(newImage, 0, newSize);

    double oldCX = width / 2.0, oldCY = height / 2.0;
    double newCX = newW / 2.0, newCY = newH / 2.0;

    for (int y = 0; y < newH; ++y) {
        for (int x = 0; x < newW; ++x) {
            double dx = x - newCX;
            double dy = y - newCY;
            double origX = dx * cosA + dy * sinA + oldCX;
            double origY = -dx * sinA + dy * cosA + oldCY;

            if (origX >= 0 && origX < width && origY >= 0 && origY < height) {
                for (int ch = 0; ch < channels; ++ch) {
                    unsigned char val = bilinearInterpolation(imageData, origX, origY, ch, width, height);
                    setPixel(newImage, x, y, ch, val, newW, newH);
                }
            }
        }
    }

    deallocateImage();
    imageData = newImage;
    width = newW;
    height = newH;
}

void ImageProcessor::scaleImage(double factor) {
    if (!imageData || factor <= 0) return;

    int newW = static_cast<int>(std::round(width * factor));
    int newH = static_cast<int>(std::round(height * factor));
    size_t newSize = static_cast<size_t>(newW * newH * channels);

    unsigned char* resized;
    if (useBuddySystem && allocator) {
        resized = static_cast<unsigned char*>(allocator->allocate(newSize));
    } else {
        resized = new unsigned char[newSize];
    }

    std::memset(resized, 0, newSize);

    double xRatio = width / static_cast<double>(newW);
    double yRatio = height / static_cast<double>(newH);

    for (int y = 0; y < newH; ++y) {
        for (int x = 0; x < newW; ++x) {
            double srcX = x * xRatio;
            double srcY = y * yRatio;
            for (int ch = 0; ch < channels; ++ch) {
                unsigned char val = bilinearInterpolation(imageData, srcX, srcY, ch, width, height);
                setPixel(resized, x, y, ch, val, newW, newH);
            }
        }
    }

    deallocateImage();
    imageData = resized;
    width = newW;
    height = newH;
}
