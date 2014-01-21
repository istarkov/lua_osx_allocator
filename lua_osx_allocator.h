/*
 * iceutils.h
 *
 *  Created on: Apr 17, 2013
 *      Author: ice
 * u need to patch lua src/lj_alloc.c
 * see readme.md
 * to use this allocator
 * make LDFLAGS="-losx_allocator" all
 * 
 *
 *
 *
 *
 *
 *
 *
 */

#ifndef LUA_OSX_ALLOCATOR_H_
#define LUA_OSX_ALLOCATOR_H_



extern "C" {
  void* ice_alloc(unsigned size);
  void ice_free(void *ap);
}

#endif /* LUA_OSX_ALLOCATOR_H_ */
