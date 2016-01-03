/**
 * heap209.c
 * =========
 *
 * Implements a small heap space organized into chunks of free and allocated
 * space (maintained through linked lists.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>

#include "heap209.h"

void *heap_region = NULL;
Chunk *free_list = NULL;
Chunk *alloc_list = NULL;

/**
 * Allocate `nbytes` from the heap, and returns a pointer to the start of the
 * freshly allocated memory. Returns NULL if there is insufficient free
 * contiguous space.
 */
void *malloc209(size_t nbytes)
{
   Chunk *checksize = free_list;
   Chunk *behindcheck = NULL;  
    while(checksize != NULL){
         if(checksize->size >= nbytes){
             break;
           }
          behindcheck = checksize;
          checksize = checksize->next;
    }
    if(checksize == NULL){
     return NULL; //no free space available
    }

    Chunk *newchunk = (Chunk *) malloc(sizeof(Chunk)); //chunk to be added into alloc_list that gets mem from free_list
   if(!newchunk){fprintf(stderr, "Not enough memory\n"); exit(1);} 

    if(checksize->size == nbytes){
      if(behindcheck == NULL){ //checksize is the first chunk in free_list
        newchunk->addr = checksize->addr;
        newchunk->size = nbytes;
        newchunk->next = NULL;
        free_list = checksize->next;

        }else if(behindcheck != NULL){
          newchunk->addr = checksize->addr;
          newchunk->size = nbytes;
          newchunk->next = NULL;
          behindcheck->next = checksize->next;

         }
     }else if(checksize->size > nbytes){
               newchunk->addr = checksize->addr;
               newchunk->size = nbytes;
               newchunk->next = NULL;

               checksize->addr = checksize->addr + nbytes;
               checksize->size = checksize->size - nbytes;
            }

     if(alloc_list->addr == NULL){ //if alloc_list is empty 
        free(alloc_list);
        alloc_list = newchunk;
        return alloc_list->addr; // in a sense you are deleting the first chunk that was init to null, 0 , null
       }else if(newchunk->addr < alloc_list->addr){
               Chunk *tempalloc = alloc_list;
                newchunk->next = tempalloc;
                alloc_list = newchunk;
                return alloc_list->addr;
                   }  else{
            Chunk *insertpoint = alloc_list;
            Chunk *behindpoint = NULL;
            while(insertpoint != NULL){
              if((behindpoint != NULL) && (insertpoint != NULL) && (behindpoint->addr < newchunk->addr) && (newchunk->addr < insertpoint->addr)){
                 break;
                     }
              behindpoint = insertpoint;
              insertpoint = insertpoint->next;
                  }
             if(insertpoint == NULL){
               behindpoint->next = newchunk; //inserting at the end
               return newchunk->addr;
                }else{
                   behindpoint->next = newchunk;
                   newchunk->next = insertpoint;
                   return newchunk->addr;
   
                    }

         }

    return NULL;
}

/**
 * Release the previously allocated memory pointed to by `addr` back into the
 * heap. Returns 0 on success, or -1 if `addr` was not found in the allocated
 * chunk list.
 */
int free209(void *addr)
{
 Chunk *tofree = alloc_list;
 Chunk *beforefree = NULL;
   while((tofree->addr != addr) && (tofree != NULL)){
         beforefree = tofree;
         tofree = tofree->next;
            }
   if(tofree == NULL){
   return -1; }
   if(beforefree == NULL){
    alloc_list = tofree->next;
     }else if (beforefree != NULL){
            beforefree->next = tofree->next;
                   }
   
 if(free_list == NULL){
    free_list = tofree;
    return 0;
         }

 Chunk *freeinsert = free_list;
 Chunk *beforeinsert = NULL;
    
   if(addr < free_list->addr){
       tofree->next = freeinsert;
       free_list = tofree;
       return 0;
             }else{

    while(freeinsert != NULL){
        if((beforeinsert != NULL) &&( beforeinsert->addr<addr) && (addr<freeinsert->addr)){
           break; }
         
        beforeinsert = freeinsert;
        freeinsert = freeinsert->next;
          }
  if(freeinsert == NULL){
      beforeinsert->next = tofree;
      return 0;
     }  else {
            beforeinsert->next = tofree;
            tofree->next = freeinsert;
            return 0;
                    }
             }
return -1;
} 

/**
 * Initialize a large piece of contiguous memory to serve as the heap region.
 *
 * NB: mmap(1) is a system call that is used to create new regions within the
 * virtual (logical) address space of the calling process. It can be used to
 * map the contents of files into memory, so that you can directly access the
 * file data through memory pointers instead of requiring explicit read and
 * write function calls. Here we are using it in a slightly different way in
 * order to negotiate with the operating system to give us a large region of
 * memory for our private use (without any files being involved.)
 *
 * The `mmap` call will return a `void *` pointer to the allocated memory. It's
 * arguments are:
 *
 *  - NULL: let the kernel choose where to place the memory in the address
 *      space.
 *  - PROT_READ | PROT_WRITE: bitmask to indicate memory will be used for
 *      reading and writing
 *  - MAP_PRIVATE | MAP_ANON: create an anonymous mapping that is private (only
 *      visible) to this process
 *  - -1: no file descriptor as this mapping is not backed by a file
 *  - 0: no offest, again because no files are involved
 */
void heap209_init(size_t heap_size)
{
    if (heap_region) {
        fprintf(stderr, "ERROR: Cannot re-initialize the 209 heap\n");
        return;
    }

    heap_region = mmap(NULL, heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

    if (heap_region == MAP_FAILED) {
        perror("mmap");
        exit(-1);
    }

    // TODO FIXME: Initialize the heap metadata lists
    free_list = (Chunk *) malloc(sizeof(Chunk));
    if(!free_list){fprintf(stderr, "Not enough memory\n"); exit(1);}
    alloc_list = (Chunk *) malloc(sizeof(Chunk));
     if(!alloc_list){fprintf(stderr, "Not enough memory\n"); exit(1);}
    free_list->addr = heap_region;
    free_list->size = heap_size;
    free_list->next = NULL;
    alloc_list->addr = NULL;
    alloc_list->size = 0;
    alloc_list->next = NULL;
	
}

/**
 * Clean up all chunk metadata associated with the heap.
 */
void heap209_cleanup(void)
{
  Chunk *alloc_temp = alloc_list->next;
  while(alloc_temp !=NULL){
    Chunk *temp = alloc_temp->next;
    free(alloc_temp);
    alloc_temp = temp;
  }
  free(alloc_list);
  free(alloc_temp);

  Chunk *free_temp = free_list->next;
  while(free_temp != NULL){
    Chunk *tempfree = free_temp->next;
    free(free_temp);
    free_temp = tempfree;
    }
  free(free_list);
  free(free_temp);

   
}
