#ifndef BUDDY_ALLOCATOR_H
#define BUDDY_ALLOCATOR_H

#include <cstddef>
#include <vector>
#include <unordered_map>

class BuddyAllocator {
public:
    explicit BuddyAllocator(size_t maxOrder);
    ~BuddyAllocator();

    void* allocate(size_t size);
    void deallocate(void* ptr);
    size_t getTotalAllocated() const;

private:
    size_t maxOrder;                      // Nivel máximo de orden (2^maxOrder = tamaño total del pool)
    size_t poolSize;                      // Tamaño total del bloque de memoria
    size_t totalAllocated;               // Memoria actualmente asignada
    char* memory;                         // Bloque de memoria principal
    std::vector<std::vector<bool>> freeBlocks;  // Estructura para bloques disponibles por orden
    std::unordered_map<void*, size_t> allocations; // Registro de punteros asignados y su orden

    // Métodos auxiliares
    size_t blockSize(size_t order) const;
    size_t calculateOrder(size_t size) const;

    void splitToOrder(size_t fromOrder, size_t index, size_t toOrder);
    void markFree(size_t order, size_t index);
    void markUsed(size_t order, size_t index);
    bool isFree(size_t order, size_t index) const;
};

#endif // BUDDY_ALLOCATOR_H
