/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include "include/m2mapplicationmemory.h"
#include "mbed-trace/mbed_trace.h"

#define TRACE_GROUP "mClt"

#ifdef M2M_APPLICATION_TRACE_PRINTS
int M2MApplicationMemory::memTotal = 0;
int M2MApplicationMemory::memCount = 0;
#endif
#ifdef M2M_APPLICATION_DYNMEM_LIB
void * M2MApplicationMemory::heapPtr = 0;
#endif

#ifdef M2M_APPLICATION_PASSTHROUGH
#ifdef M2M_APPLICATION_TRACE_PRINTS
#include <malloc.h>
#endif
#endif

#ifdef M2M_APPLICATION_DYNMEM_LIB
#ifdef M2M_APPLICATION_TRACE_PRINTS
m2m_mem_stat_t M2MApplicationMemory::memInfo;
void M2MApplicationMemory::memory_fail_callback(m2m_heap_fail_t fail) {
    tr_error("\nM2M memory failure: %u\n", fail);
}
#endif
#endif

#ifdef M2M_APPLICATION_DYNMEM_LIB

/* nanoservices dynmemlib based implementation */
void * M2MApplicationMemory::operator new (size_t size) {
    void *tmp;
    tmp=m2m_dyn_mem_alloc((uint8_t *)heapPtr, size);
#ifdef M2M_APPLICATION_TRACE_PRINTS
    memTotal+=size; memCount++;
    tr_debug("mn"); /* M2M new */
    print_heap_running_statistics();
#endif
    return tmp;
}

void M2MApplicationMemory::operator delete (void * ptr) {
#ifdef M2M_APPLICATION_TRACE_PRINTS
    memCount--; /* still update allocation counter */
#endif
    m2m_dyn_mem_free((uint8_t *)heapPtr, ptr);
#ifdef M2M_APPLICATION_TRACE_PRINTS
    tr_debug("md"); /* M2M delete */
    print_heap_overall_statistics();
#endif
}
void * M2MApplicationMemory::operator new[] (size_t size) {
    void *tmp;
    tmp=m2m_dyn_mem_alloc((uint8_t *)heapPtr, size);
#ifdef M2M_APPLICATION_TRACE_PRINTS
    memTotal+=size; memCount++;
    tr_debug("mn[]"); /* M2M new array */
#endif
    return tmp;
}
void M2MApplicationMemory::operator delete[] (void * ptr) {
#ifdef M2M_APPLICATION_TRACE_PRINTS
    memCount--;
#endif
    m2m_dyn_mem_free((uint8_t *)heapPtr, ptr);
#ifdef M2M_APPLICATION_TRACE_PRINTS
    tr_debug("md[]"); /* M2M delete array */
#endif
}
#endif

#ifdef M2M_APPLICATION_PASSTHROUGH
/* system malloc based implementation */
void * M2MApplicationMemory::operator new (size_t size) {
    void *tmp;
#ifdef M2M_APPLICATION_TRACE_PRINTS
    long int allocatedSize;
#endif
    tmp=malloc(size);
#ifdef M2M_APPLICATION_TRACE_PRINTS
    allocatedSize=malloc_usable_size(tmp);
    memTotal+=allocatedSize; memCount++;
    tr_debug("mn:%lu:%lu:%d:%d:", size, allocatedSize, memTotal, memCount);
#endif
    return tmp;
}

void M2MApplicationMemory::operator delete (void * ptr) {
#ifdef M2M_APPLICATION_TRACE_PRINTS
    long int allocatedSize;
    allocatedSize=malloc_usable_size(ptr);
    memCount--;
    memTotal-=allocatedSize;
    tr_debug("md:%lu:%d:%d:", allocatedSize, memTotal, memCount);
#endif
    free(ptr);
}

void * M2MApplicationMemory::operator new[] (size_t size) {
    void *tmp;
#ifdef M2M_APPLICATION_TRACE_PRINTS
    long int allocatedSize;
#endif
    tmp=malloc(size);
#ifdef M2M_APPLICATION_TRACE_PRINTS
    allocatedSize=malloc_usable_size(tmp);
    memTotal+=allocatedSize; memCount++;
    tr_debug("mn[]:%lu:%lu:%d:%d:", size, allocatedSize, memTotal, memCount);
#endif
    return tmp;
}
void M2MApplicationMemory::operator delete[] (void * ptr) {
#ifdef M2M_APPLICATION_TRACE_PRINTS
    long int allocatedSize;
    allocatedSize=malloc_usable_size(ptr);
    memCount--;
    memTotal-=allocatedSize;
    tr_debug("md[]:%lu:%d:%d:", allocatedSize, memTotal, memCount);
#endif
    free(ptr);
}
#endif

#ifdef M2M_APPLICATION_DYNMEM_LIB
void M2MApplicationMemory::init(void) {
    init(M2M_APPLICATION_DYNAMIC_MEMORY_HEAP_SIZE);
}

void M2MApplicationMemory::init(size_t heapSize) {
    void *heap=malloc( heapSize );
#ifdef M2M_APPLICATION_TRACE_PRINTS
    tr_debug("Init allocated %lu bytes for cloud client heap at %p\n", heapSize, heap);
#endif
    init(heap, heapSize);
}

void M2MApplicationMemory::init(void *heapAllocation, size_t heapSize) {
    heapSize=heapSize;
    heapPtr=heapAllocation;
#ifdef M2M_APPLICATION_TRACE_PRINTS
    m2m_dyn_mem_init((uint8_t *)heapAllocation, (uint16_t)heapSize, &memory_fail_callback, &memInfo);
#else
    m2m_dyn_mem_init((uint8_t *)heapAllocation, (uint16_t)heapSize, NULL, NULL);
#endif
}
#endif

#ifdef M2M_APPLICATION_TRACE_PRINTS
void M2MApplicationMemory::print_heap_running_statistics() {
#ifdef M2M_APPLICATION_DYNMEM_LIB
    tr_debug(":%d:%d:", memInfo.heap_sector_allocated_bytes, memInfo.heap_sector_alloc_cnt);
#else
    tr_debug(":%d:%d:", memTotal, memCount);
#endif
}
void M2MApplicationMemory::print_heap_overall_statistics() {
#ifdef M2M_APPLICATION_DYNMEM_LIB
    tr_debug(":%d:%d:%u:%u:", memInfo.heap_sector_size,
        memInfo.heap_sector_allocated_bytes_max,
        memInfo.heap_alloc_total_bytes, memInfo.heap_alloc_fail_cnt);
#endif
}
#endif

void* M2MApplicationMemory::memory_alloc(size_t size){
    void *tmp;
    #ifdef M2M_APPLICATION_DYNMEM_LIB
    tmp = m2m_dyn_mem_alloc((uint8_t *)heapPtr, size);
    #else
    if (size) {
        tmp = malloc(size);
    } else {
        tmp = 0;
    }
    #endif
    #ifdef M2M_APPLICATION_TRACE_PRINTS
    #ifdef M2M_APPLICATION_PASSTHROUGH
    memTotal+=size; memCount++;
    #endif
    tr_debug(":ma:%p", tmp);
    print_heap_running_statistics();
    #endif
    return tmp;
}

void M2MApplicationMemory::memory_free(void *ptr) {
    #ifdef M2M_APPLICATION_DYNMEM_LIB
    m2m_dyn_mem_free((uint8_t *)heapPtr, ptr);
    #else
    free(ptr);
    #endif
    #ifdef M2M_APPLICATION_TRACE_PRINTS
    tr_debug(":mf:%p", ptr);
    print_heap_overall_statistics();
    #endif
 }