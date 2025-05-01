#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <malloc.h>
#include "buddy_allocator.h"
#include "image.h"

#define VERSION "1.0.0"

struct ProgramOptions {
    std::string inputFile;
    std::string outputFile;
    double rotationAngle = 0.0;
    double scaleFactor = 1.0;
    bool useBuddySystem = false;
    bool showHelp = false;
    bool showVersion = false;
};

void printHelp() {
    std::cout << "=== AYUDA: USO DEL PROGRAMA ===\n"
              << "Uso:\n  ./program_image entrada.jpg salida.jpg -angulo ANGULO -escalar ESCALA -buddy\n\n"
              << "Parámetros:\n"
              << "  entrada.jpg        Archivo de imagen de entrada\n"
              << "  salida.jpg         Archivo donde se guarda la imagen procesada\n"
              << "  -angulo ANGULO     Ángulo de rotación (grados)\n"
              << "  -escalar ESCALA    Factor de escalado\n"
              << "  -buddy             Usa Buddy System\n"
              << "  -h, --help         Muestra esta ayuda\n"
              << "  -v, --version      Muestra la versión\n";
}

void printVersion() {
    std::cout << "program_image v" << VERSION << " - Procesador de imágenes con Buddy System\n";
}

size_t getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;
    return static_cast<size_t>(file.tellg());
}


ProgramOptions parseArgs(int argc, char* argv[]) {
    ProgramOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            opts.showHelp = true;
            printHelp();
            exit(0);
        } else if (arg == "-v" || arg == "--version") {
            opts.showVersion = true;
            printVersion();
            exit(0);
        } else if (arg == "-angulo" && i + 1 < argc) {
            opts.rotationAngle = std::stod(argv[++i]);
        } else if (arg == "-escalar" && i + 1 < argc) {
            opts.scaleFactor = std::stod(argv[++i]);
        } else if (arg == "-buddy") {
            opts.useBuddySystem = true;
        } else if (opts.inputFile.empty()) {
            opts.inputFile = arg;
        } else if (opts.outputFile.empty()) {
            opts.outputFile = arg;
        } else {
            std::cerr << "[ERROR] Argumento no reconocido: " << arg << "\n";
            printHelp();
            exit(1);
        }
    }

    if (opts.inputFile.empty() || opts.outputFile.empty()) {
        std::cerr << "[ERROR] Se requieren archivo de entrada y salida.\n";
        printHelp();
        exit(1);
    }

    return opts;
}

void processImage(const ProgramOptions& opts) {
    BuddyAllocator buddyAllocator(24);
    size_t originalSize = getFileSize(opts.inputFile);

    std::cout << "=== INICIANDO PROCESAMIENTO ===\n";
    std::cout << "Entrada: " << opts.inputFile << "\nSalida: " << opts.outputFile << "\n";

    // Procesamiento sin Buddy System
    auto start1 = std::chrono::high_resolution_clock::now();
    ImageProcessor basicProcessor(false, nullptr);
    if (!basicProcessor.loadImage(opts.inputFile)) {
        std::cerr << "No se pudo cargar la imagen de entrada.\n";
        exit(1);
    }

    int w, h, c;
    basicProcessor.getImageInfo(w, h, c);
    std::cout << "Dimensiones originales: " << w << " x " << h << "\n";
    std::cout << "Tamaño archivo original: " << originalSize / 1024.0 << " KB\n";

    basicProcessor.rotateImage(opts.rotationAngle);
    basicProcessor.scaleImage(opts.scaleFactor);
    basicProcessor.saveImage("temp_conventional.jpg");
    auto end1 = std::chrono::high_resolution_clock::now();
    auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    size_t memConventional = mallinfo2().uordblks;

    // Procesamiento con Buddy System
    auto start2 = std::chrono::high_resolution_clock::now();
    ImageProcessor buddyProcessor(true, &buddyAllocator);
    buddyProcessor.loadImage(opts.inputFile);
    buddyProcessor.rotateImage(opts.rotationAngle);
    buddyProcessor.scaleImage(opts.scaleFactor);
    buddyProcessor.saveImage(opts.outputFile);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto time2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    size_t memBuddy = buddyAllocator.getTotalAllocated();

    int fw, fh, fc;
    buddyProcessor.getImageInfo(fw, fh, fc);
    size_t finalSize = getFileSize(opts.outputFile);

    // Resultados
    std::cout << "\n=== RESULTADOS ===\n";
    std::cout << "Dimensiones finales: " << fw << " x " << fh << "\n";
    std::cout << "Tamaño final: " << finalSize / 1024.0 << " KB\n";
    std::cout << "\nTIEMPOS:\n";
    std::cout << " - Convencional: " << time1.count() << " ms\n";
    std::cout << " - Buddy System: " << time2.count() << " ms\n";

    std::cout << "\nMEMORIA:\n";
    std::cout << " - Convencional: " << memConventional / (1024.0 * 1024.0) << " MB\n";
    std::cout << " - Buddy System: " << memBuddy / (1024.0 * 1024.0) << " MB\n";

    std::cout << "\nImagen procesada guardada en: " << opts.outputFile << "\n";
    std::cout << "Reducción/Incremento de tamaño: " << (finalSize * 100.0 / originalSize) << "% del original\n";
}

int main(int argc, char* argv[]) {
    auto options = parseArgs(argc, argv);
    processImage(options);
    return 0;
}
