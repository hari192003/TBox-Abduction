/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DYNAMIC MEMORY MANAGEMENT MODULE                * */
/* *                                                        * */
/* *  $Module:   MEMORY                                     * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001      * */
/* *  MPI fuer Informatik                                   * */
/* *                                                        * */
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */ 
/* *  for more details.                                     * */
/* *                                                        * */
/* *                                                        * */
/* $Revision: 1.4 $                                     * */
/* $State: Exp $                                            * */
/* $Date: 2013/12/10 09:19:37 $                             * */
/* $Author: weidenb $                                         * */
/* *                                                        * */
/* *             Contact:                                   * */
/* *             Christoph Weidenbach                       * */
/* *             MPI fuer Informatik                        * */
/* *             Stuhlsatzenhausweg 85                      * */
/* *             66123 Saarbruecken                         * */
/* *             Email: spass@mpi-inf.mpg.de                * */
/* *             Germany                                    * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


/* $RCSfile: memory.h,v $ */


#ifndef _MEMORY_
#define _MEMORY_

/**************************************************************/
/* Includes                                                   */
/**************************************************************/

#include "misc.h"

/**************************************************************/
/* Data structures and constants                              */
/**************************************************************/

#ifndef memory__DYNMAXSIZE
#define memory__DYNMAXSIZE   1024 /* At most blocks of size memory__DYNMAXSIZE
				     bytes are administrated by the
				     module, larger requests are
				     directly mapped to system calls */
#endif

#ifndef memory__SHAREDPAGES
#define memory__SHAREDPAGES    1  /* Number of block sizes sharing an allocated 
				     page. By setting memory__SHAREDPAGES to 4, 
				     the module would administrate requests 
				     for 1, 2, 3 and 4 bytes on the same set of 
				     pages, requests for 5, 6, 7 and 8 on another 
				     one, etc. By default every block size has 
				     its own set of pages */
#endif

#ifndef memory__FREESHREDDER
#define memory__FREESHREDDER  'S' /*  The decimal number 83, which can also be 
				      read as 53 hexadecimal, or the character 
				      'S' in ASCII code */
#endif

#define memory__KILOBYTE        1024

#ifndef memory__DEFAULTPAGESIZE
#define memory__DEFAULTPAGESIZE (8 * memory__KILOBYTE)
                                  /* Used to set the default size of a page. 
				     If the default page size is too small to 
				     contain two objects of size memory__DYNMAXSIZE
				     the default page size is automatically 
				     increased by the function memory_Init */
#endif

#ifndef memory__UNLIMITED
#define memory__UNLIMITED       (-1)
                                  /* Used to set the maximal amount of memory
				     available for the memory module to
				     "unlimited" when calling memory_Init. */
#endif

typedef struct MEMORY_RESOURCEHELP {
  POINTER free;                    /* pointer to the next free block in list  */
  POINTER next;                    /* pointer to the next fresh block         */
  POINTER page;                    /* pointer to head of page list            */
  POINTER end_of_page;             /* pointer to the end of current page      */
  int     total_size;              /* total block size inc. debug marks       */
  int     aligned_size;            /* block size without debug marks          */
  int     offset;                  /* offset of last usable block on page     */
} MEMORY_RESOURCE;

extern MEMORY_RESOURCE * memory_ARRAY[];

#if defined(CHECK)
typedef struct MEMORY_INFOHELP {
  const char * mallocInFile;       /* origin of allocation request:   file    */
  const char * freeInFile;         /* origin of deallocation request: file    */
  unsigned short int mallocAtLine; /* origin of allocation request:   line    */
  unsigned short int freeAtLine;   /* origin of deallocation request: line    */
} MEMORY_INFONODE, * MEMORY_INFO;

#endif

typedef struct MEMORY_BIGBLOCKHEADERHELP {
  struct MEMORY_BIGBLOCKHEADERHELP * previous, * next;
} MEMORY_BIGBLOCKHEADERNODE, * MEMORY_BIGBLOCKHEADER;

extern long                  memory_MAXMEM;

extern unsigned long         memory_NEWBYTES;
extern unsigned long         memory_FREEDBYTES;

extern const unsigned int    memory_MAGICMALLOC;
extern const unsigned int    memory_MAGICFREE;

extern const unsigned int    memory_ALIGN;

extern MEMORY_BIGBLOCKHEADER memory_BIGBLOCKS;

/**************************************************************/
/* Debug Information                                          */
/**************************************************************/

extern unsigned int memory_MARKSIZE;
extern unsigned int memory_OFFSET;

/**************************************************************/
/* Check Functions                                            */
/**************************************************************/


#ifdef CHECK
void    memory_CheckFree(POINTER Freepointer, unsigned int Size, unsigned int RealBlockSize, const char * File, unsigned short int Line);
#endif /* CHECK */


/**************************************************************/
/* Inline Functions                                           */
/**************************************************************/

unsigned int memory_CalculateRealBlockSize(unsigned int BlockSize);
unsigned int memory_LookupRealBlockSize(unsigned int BlockSize);


#ifdef CHECK
void         memory_SetBlockStatusAndSize(POINTER Mem, unsigned int Status, unsigned int Size);
unsigned int memory_GetBlockSize(POINTER Mem);
unsigned int memory_GetRealBlockSize(POINTER Mem);
unsigned int memory_GetBlockStatus(POINTER Mem); 
void         memory_SetInfo(MEMORY_INFO Info, const char * MallocInFile,
			    unsigned short int MallocAtLine, const char * FreeInFile,
			    unsigned short int FreeAtLine);
#endif


unsigned long memory_DemandedBytes(void);
unsigned long memory_UsedBytes(void);

/**************************************************************/
/* Functions                                                  */
/**************************************************************/

void    memory_Init(long);
void    memory_Restrict(long);

void    memory_Print(void);
void    memory_FPrint(FILE*);
  
void    memory_PrintLeaks(void);
void    memory_PrintDetailed(void);
void    memory_PrintAllocatedBlocks(unsigned int Size);
void    memory_PrintFreedBlocks(unsigned int Size);
void    memory_PrintAllocatedBigBlocks(void);
  
void    memory_FreeAllMem(void);


#if defined(CHECK) && !defined(NO_MEMORY_MANAGEMENT) /* declare drop-in debug versions memory functions */
POINTER memory_MallocIntern(unsigned int, const char *, unsigned short int);
POINTER memory_ReallocIntern(POINTER, unsigned int, unsigned int, const char *, unsigned short int);
void    memory_FreeIntern(POINTER  Freepointer, 
			  unsigned int Size, 
			  const char * File, 
			  unsigned short int Line);
POINTER memory_CallocIntern(unsigned int,
			    unsigned int, 
			    const char *, 
			    unsigned short int);
#define memory_Malloc(Size) memory_MallocIntern((Size), __FILE__, __LINE__)
#define memory_Realloc(Ptr, OldSize, NewSize) memory_ReallocIntern((Ptr), (OldSize), (NewSize), __FILE__, __LINE__) 
#define memory_Calloc(Elements, Size) memory_CallocIntern((Elements), (Size), __FILE__, __LINE__)
#define memory_Free(Pointer, Size) memory_FreeIntern((Pointer), (Size), __FILE__, __LINE__)
#else
POINTER memory_Malloc(unsigned int);
POINTER memory_Realloc(POINTER, unsigned int, unsigned int);
void    memory_Free(POINTER, unsigned int);
POINTER memory_Calloc(unsigned int, unsigned int);
#endif

#endif

