#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include "buddy_allocator.h"

class ImageProcessor {
public:
    ImageProcessor(bool useBuddy = false, BuddyAllocator* allocator = nullptr);
    ~ImageProcessor();

    // Carga y guarda imágenes
    bool loadImage(const std::string& filename);
    bool saveImage(const std::string& filename);

    // Obtener información de la imagen
    void getImageInfo(int& width, int& height, int& channels) const;

    // Transformaciones
    void rotateImage(double angleDegrees);
    void scaleImage(double factor);

private:
    unsigned char* imageData;
    int width;
    int height;
    int channels;

    bool useBuddySystem;
    BuddyAllocator* allocator;

    // Gestión de memoria
    void allocateImage(int w, int h, int c);
    void deallocateImage();

    // Utilidades internas para manejo de píxeles
    unsigned char* getPixel(unsigned char* data, int x, int y, int c, int w, int h);
    void setPixel(unsigned char* data, int x, int y, int c, unsigned char value, int w, int h);
    unsigned char bilinearInterpolation(unsigned char* data, double x, double y, int c, int w, int h);
};

#endif // IMAGE_PROCESSOR_H
