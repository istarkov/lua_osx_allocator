/*
 * iceutils.cpp

 *
 *  Created on: Apr 17, 2013
 *      Author: ice
 */
#include "lua_osx_allocator.h"

//#include <cstdlib>
#include <stdint.h>

#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>

#include <vector>
#include <stdexcept>

#include <boost/thread/mutex.hpp>


#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS   MAP_ANON
#endif

//#define MIN_BLOCK_SIZE 0x10000000
#define MIN_UNIT_SIZE 1024*1024*16;

/* The following implementation of malloc and free was copied from the book 
   "The C Programming Language" by Brian W. Kernighan and Dennis M. Ritchie. */  
  
typedef long Align;  
  
union header {  
    struct {  
         union header *ptr;  
         unsigned size;  
    } s;  
    Align x;  
};  
  
typedef union header Header;  
  
static Header base;  
static Header *freep = NULL;  


static void *kr_sbrk()  
{  
  
  #ifdef __APPLE__
  static const uintptr_t MMAP_REGION_START = 0x10000;//((uintptr_t)0x10000);
  static const uintptr_t MMAP_REGION_END = 0x80000000;//((uintptr_t)0x80000000)
  #else
  static const uintptr_t MMAP_REGION_START = 0x00000000;
  static const uintptr_t MMAP_REGION_END =   0x70000000;
  #endif
  static const int MMAP_PROT = (PROT_READ | PROT_WRITE);
  static const int MMAP_FLAGS = (MAP_PRIVATE | MAP_ANONYMOUS);
  
  static int region_num = -1;
  static std::vector<void*> allocated_regions_;

  if(region_num==-1){
    
    uintptr_t alloc_hint = MMAP_REGION_START;
    
    size_t size = sizeof(Header) * MIN_UNIT_SIZE;
    
    region_num = 0;

    for(;;){
      #ifdef __APPLE__
      void *p = mmap((void *)alloc_hint, size, MMAP_PROT, MMAP_FLAGS, -1, 0);  
      #else
      void *p = mmap((void *)alloc_hint, size, MMAP_PROT, MAP_32BIT | MMAP_FLAGS, -1, 0);
      #endif

      if (p != NULL && p!=(void *) -1){
        if(((uintptr_t)p >= MMAP_REGION_START) && ((uintptr_t)p + size < MMAP_REGION_END)){
          //printf("kr_sbrk: allocated %lx %lx\n", size, (uintptr_t)p);
          allocated_regions_.push_back(p);        
        }else{
          //printf("kr_sbrk: need be deallocated\n");
          munmap(p, size);
          break;
        }
        #ifdef __APPLE__
        alloc_hint = (uintptr_t)p + size;
        #endif
      } else {
        //printf("kr_sbrk: nmap allocation failed\n");
        return (void *) -1;
      }
    }
  }

  if(region_num < (int)allocated_regions_.size()){
    //printf("kr_sbrk: return mem region %d\n", region_num);
    return allocated_regions_[region_num++];
  }

  return (void *) -1;
}
  
  
  
static void kr_free(void *ap) {  
    Header *bp, *p;  
  
    bp = (Header *)ap - 1;  
  
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)  
         if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))  
             break;  
  
    if (bp + bp->s.size == p->s.ptr) {  
        bp->s.size += p->s.ptr->s.size;  
        bp->s.ptr = p->s.ptr->s.ptr;  
    } else {  
        bp->s.ptr = p->s.ptr;  
    }  
  
    if (p + p->s.size == bp) {  
        p->s.size += bp->s.size;  
        p->s.ptr = bp->s.ptr;  
    } else {  
        p->s.ptr = bp;  
    }  
  
    freep = p;  
}

static Header *kr_morecore() {  
    char *cp;  
    Header *up;  
    cp = static_cast<char*>(kr_sbrk());

    if (cp == (char *) -1)  
        return NULL;  
  
    up = (Header *) cp;  
    up->s.size = MIN_UNIT_SIZE; 
    kr_free((void *)(up + 1));  
  
    return freep;  
}  

static void* kr_malloc(unsigned nbytes) {  
    Header *p, *prevp;  
    unsigned nunits;  
  
    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;  
  
    if ((prevp = freep) == NULL) {  
        base.s.ptr = freep = prevp = &base;  
        base.s.size = 0;  
    }  
  
    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {  
  
        if (p->s.size >= nunits) {  
            if (p->s.size == nunits) {  
                prevp->s.ptr = p->s.ptr;  
            } else {  
                p->s.size -= nunits;  
                p += p->s.size;  
                p->s.size = nunits;  
            }  
            freep = prevp;  
            return (void *)(p+1);  
        }  
  
        if (p == freep)  
            if ((p = kr_morecore()) == NULL)  
                return NULL;  
    }  
}  
  
static boost::mutex mtx_;

void* ice_alloc(unsigned size){
  boost::mutex::scoped_lock lock(mtx_);
  return kr_malloc(size);
}

void ice_free(void *ap) {
  boost::mutex::scoped_lock lock(mtx_); 
  kr_free(ap);
}



