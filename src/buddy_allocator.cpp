#include "buddy_allocator.h"
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>

BuddyAllocator::BuddyAllocator(size_t maxOrder)
    : maxOrder(maxOrder), totalAllocated(0) {
    poolSize = 1ULL << maxOrder;
    memory = new char[poolSize];
    freeBlocks.resize(maxOrder + 1);

    for (size_t level = 0; level <= maxOrder; ++level) {
        size_t count = 1ULL << (maxOrder - level);
        freeBlocks[level].assign(count, false);
    }

    // Start with one large free block
    freeBlocks[maxOrder][0] = true;
}

BuddyAllocator::~BuddyAllocator() {
    delete[] memory;
}

void* BuddyAllocator::allocate(size_t size) {
    size = std::max(size, size_t(16));
    size_t targetOrder = calculateOrder(size);
    if (targetOrder > maxOrder) return nullptr;

    for (size_t current = targetOrder; current <= maxOrder; ++current) {
        size_t blockCount = freeBlocks[current].size();
        for (size_t index = 0; index < blockCount; ++index) {
            if (freeBlocks[current][index]) {
                splitToOrder(current, index, targetOrder);
                void* blockPtr = memory + index * blockSize(targetOrder);
                allocations[blockPtr] = targetOrder;
                totalAllocated += blockSize(targetOrder);
                return blockPtr;
            }
        }
    }

    return nullptr;
}

void BuddyAllocator::deallocate(void* ptr) {
    if (!ptr || allocations.find(ptr) == allocations.end()) return;

    size_t order = allocations[ptr];
    size_t size = blockSize(order);
    size_t offset = static_cast<char*>(ptr) - memory;
    size_t index = offset / size;

    markFree(order, index);
    totalAllocated -= size;

    while (order < maxOrder) {
        size_t buddy = index ^ 1;
        if (isFree(order, buddy)) {
            markUsed(order, index);
            markUsed(order, buddy);
            index /= 2;
            order++;
            markFree(order, index);
        } else {
            break;
        }
    }

    allocations.erase(ptr);
}

size_t BuddyAllocator::getTotalAllocated() const {
    return totalAllocated;
}

// Helper Methods

size_t BuddyAllocator::blockSize(size_t order) const {
    return 1ULL << order;
}

size_t BuddyAllocator::calculateOrder(size_t size) const {
    size_t order = 0;
    size_t bSize = 1;
    while (bSize < size && order < maxOrder) {
        bSize <<= 1;
        ++order;
    }
    return order;
}

void BuddyAllocator::splitToOrder(size_t fromOrder, size_t index, size_t toOrder) {
    for (size_t o = fromOrder; o > toOrder; --o) {
        markUsed(o, index);
        size_t left = index * 2;
        size_t right = left + 1;
        markFree(o - 1, left);
        markFree(o - 1, right);
        index = left;
    }
    markUsed(toOrder, index);
}

void BuddyAllocator::markFree(size_t order, size_t index) {
    if (order <= maxOrder && index < freeBlocks[order].size()) {
        freeBlocks[order][index] = true;
    }
}

void BuddyAllocator::markUsed(size_t order, size_t index) {
    if (order <= maxOrder && index < freeBlocks[order].size()) {
        freeBlocks[order][index] = false;
    }
}

bool BuddyAllocator::isFree(size_t order, size_t index) const {
    return order <= maxOrder && index < freeBlocks[order].size() && freeBlocks[order][index];
}
