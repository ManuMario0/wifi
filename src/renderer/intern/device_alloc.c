//
//  device_alloc.c
//  AgeOfAnarchy
//
//  Created by Emmanuel Mera on 31/03/2023.
//

// TODO: redo the allocator for more optimization, for now, I'll only do a default one

#include "device_alloc.h"

#include "MEM_alloc.h"

#include "KER_list.h"

//-----------------------------------------------//
//-     Local structures                        -//
//-----------------------------------------------//

typedef struct MemSegment MemSegment;

struct MemBlock {
    int         type;
    
    MemSegment *segment;
    size_t      offset;
    size_t      size;
};

struct MemSegment {
    int             type;
    
    VkDeviceMemory  mem;
    List *          mem_base;
    size_t          size;
    size_t          mem_in_use;
};

typedef struct {
    int     type;
    
    List *  segment_base;
    size_t  size;
    size_t  mem_in_use;
} MemAlloc;

//-----------------------------------------------//
//-     Local variables                         -//
//-----------------------------------------------//

static MemAlloc allocators[64] = {0};
static VkPhysicalDeviceMemoryProperties memProperties = {0};
static int totblock = 0;

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

static MemBlock *add_memory_block(Window *window, MemAlloc *alloc, VkMemoryRequirements *memRequirements);

//-----------------------------------------------//
//-     Implementation                          -//
//-----------------------------------------------//

void initVulkanAllocator(Window *window) {
    vkGetPhysicalDeviceMemoryProperties(window->physicalDevice, &memProperties);
    
    for (int i=0; i<memProperties.memoryTypeCount; i++) {
        MemAlloc *alloc = &allocators[2*i];
        
        alloc->size = DEFAULT_SEGMENT_SIZE;
        alloc->mem_in_use = 0;
        alloc->type = 2*i;
        
        MemSegment *segment = MEM_malloc(sizeof(MemSegment), __func__);
        segment->size = DEFAULT_SEGMENT_SIZE;
        segment->mem_in_use = 0;
        segment->mem_base = NULL;
        segment->type = alloc->type;
        
        VkMemoryAllocateInfo memInfo = {0};
        memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memInfo.allocationSize = DEFAULT_SEGMENT_SIZE;
        memInfo.memoryTypeIndex = i;
        
        vkAllocateMemory(window->device, &memInfo, NULL, &segment->mem);
        alloc->segment_base = listCreateCell(segment);
        
        alloc++;
        alloc->size = DEFAULT_SEGMENT_SIZE;
        alloc->mem_in_use = 0;
        alloc->type = 2*i+1;
        
        segment = MEM_malloc(sizeof(MemSegment), __func__);
        segment->size = DEFAULT_SEGMENT_SIZE;
        segment->mem_in_use = 0;
        segment->mem_base = NULL;
        segment->type = alloc->type;
        
        vkAllocateMemory(window->device, &memInfo, NULL, &segment->mem);
        alloc->segment_base = listCreateCell(segment);
    }
}

void destroyVulkanAllocators(Window *window) {
    for (int i=0; i<2*memProperties.memoryTypeCount; i++) {
        List *segments = allocators[i].segment_base;
        
        while (segments) {
            MemSegment *s = listHead(segments);
            
            listDestroyList(s->mem_base, MEM_free);
            vkFreeMemory(window->device, s->mem, NULL);
            
            segments = listTail(segments);
        }
        
        listDestroyList(allocators[i].segment_base, MEM_free);
        allocators[i].size = 0;
        allocators[i].mem_in_use = 0;
        allocators[i].segment_base = NULL;
    }
}

MemBlock *allocVulkanMemory(Window *window, VkMemoryRequirements *memRequirements, VkMemoryPropertyFlags properties, int type) {
    int memoryTypeIndex = 0;
    for (int i=0; i<memProperties.memoryTypeCount; i++) {
        if (memRequirements->memoryTypeBits & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            memoryTypeIndex = i;
            break;
        }
    }
    
    return add_memory_block(window, &allocators[2*memoryTypeIndex+type], memRequirements);
}

void freeVulkanMemory(MemBlock *block) {
    MemSegment *s = block->segment;
    
    List *b = s->mem_base;
    if (listHead(b) == block) {
        s->mem_base = listTail(s->mem_base);
        listCons(b, NULL); // to completely decouple it from the rest
        listDestroyList(b, NULL);
        
        s->mem_in_use -= block->size;
        allocators[block->type].mem_in_use -= block->size;
    } else {
        while (listHead(listTail(b)) != block) {
            b = listTail(b);
        }
        List *temp = listTail(b);
        listCons(b, listTail(temp));
        listCons(temp, NULL); // to completely decouple it from the rest
        listDestroyList(temp, NULL);
        
        s->mem_in_use -= block->size;
        allocators[block->type].mem_in_use -= block->size;
    }
    totblock--;
    MEM_free(block);
}

inline VkDeviceMemory getBindingPoint(MemBlock *b) {
    MemSegment *s = b->segment;
    return s->mem;
}

inline size_t getOffset(MemBlock *b) {
    return b->offset;
}

// TODO: print overall stat (fragmentation, total memory allocated, total memory in use) and per segment data
void printVulkanMemStat(void) {
    size_t mem_in_use = 0;
    size_t allocated_mem = 0;
    
    for (int i=0; i<2*memProperties.memoryTypeCount; i++) {
        mem_in_use += allocators[i].mem_in_use;
        allocated_mem += allocators[i].size;
    }
    
    printf("Device memory stats :\n");
    printf("Total memory used : %.3f Mio\n", (double) mem_in_use / (double) (1024*1024));
    printf("Total memory allocated : %.3f Mio\n", (double) allocated_mem / (double) (1024*1024));
    printf("Number of allocated block : %d\n", totblock);
    printf("  ITEMS TOTAL-MiB AVERAGE-KiB NAME :\n");
}

//-----------------------------------------------//
//-     Local functions                         -//
//-----------------------------------------------//

MemBlock *find_memory_spot(MemAlloc *alloc, MemSegment *segment, size_t size, uint64_t alignement) {
    MemBlock startingBlock;
    startingBlock.size = 0;
    startingBlock.offset = 0;
    
    MemBlock endingBlock;
    endingBlock.offset = segment->size;
    
    List *blocksList = segment->mem_base;
    List *blocksListPrev = NULL;
    MemBlock *current_block;
    if (!blocksList) {
        current_block = &endingBlock;
    } else {
        current_block = listHead(blocksList);
    }
    MemBlock *prev_block = &startingBlock;
    size_t padding = (alignement - (prev_block->size + prev_block->offset) % alignement) % alignement;
    while ((long)current_block->offset - ((prev_block->size + prev_block->offset) + padding) < size && blocksList) {
        blocksListPrev = blocksList;
        blocksList = listTail(blocksList);
        prev_block = current_block;
        if (blocksList) {
            current_block = listHead(blocksList);
        } else {
            current_block = &endingBlock;
        }
        padding = (alignement - (prev_block->size + prev_block->offset) % alignement) % alignement;
    }
    
    if (current_block->offset - (prev_block->size + prev_block->offset + padding) >= size) {
        MemBlock *block = MEM_calloc(sizeof(MemBlock), __func__);
        block->size = size;
        block->type = alloc->type;
        prev_block->size += padding;
        block->offset = prev_block->size + prev_block->offset;
        block->segment = segment;
        
        if (prev_block == &startingBlock) {
            segment->mem_base = listCreateCell(block);
        } else {
            List *temp = listCreateCell(block);
            listCons(blocksListPrev, temp);
            if (current_block != &endingBlock) {
                listCons(temp, blocksList);
            }
        }
        
        segment->mem_in_use += size + padding;
        alloc->mem_in_use += size + padding;
        
        totblock ++;
        return block;
    }
    
    return NULL;
}

MemBlock *add_memory_block(Window *window, MemAlloc *alloc, VkMemoryRequirements *memRequirements) {
    List *segmentsList = alloc->segment_base;
    List *prevSegmentsList = NULL;
    size_t size = memRequirements->size;
    uint64_t alignment = memRequirements->alignment;
    
    MemSegment *segment = listHead(segmentsList);
    while (segmentsList) {
        if (segment->size - segment->mem_in_use > size) {
            MemBlock *b = find_memory_spot(alloc, segment, size, alignment);
            if (b) {
                return b;
            }
        }
        
        prevSegmentsList = segmentsList;
        segmentsList = listTail(segmentsList);
        if (segmentsList) {
            segment = listHead(segmentsList);
        }
    }
    
    if (!segmentsList) {
        segment = MEM_calloc(sizeof(MemSegment), __func__);
        segment->size = DEFAULT_SEGMENT_SIZE;
        segment->type = alloc->type;
        segment->mem_in_use = 0;
        segment->mem_base = NULL;
        
        VkMemoryAllocateInfo memInfo = {0};
        memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memInfo.allocationSize = DEFAULT_SEGMENT_SIZE;
        memInfo.memoryTypeIndex = alloc->type;
        
        vkAllocateMemory(window->device, &memInfo, NULL, &segment->mem);
        
        List *cell = listCreateCell(segment);
        listCons(prevSegmentsList, cell);
    }
    
    return find_memory_spot(alloc, segment, size, alignment);
}
