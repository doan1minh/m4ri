/******************************************************************************
*
*                 M4RI: Linear Algebra over GF(2)
*
*    Copyright (C) 2007 Gregory Bard <gregory.bard@ieee.org> 
*
*  Distributed under the terms of the GNU General Public License (GPL)
*  version 2 or higher.
*
*    This code is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*  The full text of the GPL is available at:
*
*                  http://www.gnu.org/licenses/
******************************************************************************/

#ifdef _MSC_VER
#include <windows.h>
#endif

#ifndef HAVE_SSE2
#undef HAVE_MM_MALLOC
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "misc.h"
#ifdef HAVE_MM_MALLOC
#include <mm_malloc.h>
#endif

#include "grayflex.h"


/* blocks of memory we like to keep around for later re-use */
mm_block m4ri_mmc_cache[M4RI_MMC_NBLOCKS];

void m4ri_die(const char *errormessage, ...) {
  va_list lst;
  va_start(lst, errormessage);
  vfprintf(stderr, errormessage, lst);
  va_end(lst);
  abort();
}

/* Warning: I assume *destination has RADIX+1 bytes available */
void m4ri_word_to_str( char *destination, word data, int colon) {
  int i;
  int j = 0;

  if (colon == 0) {

    for (i=0; i<RADIX; i++) {
      if (GET_BIT(data,i))
	destination[i]='1';
      else 
	destination[i]=' ';
    }
    destination[RADIX]='\0';

  } else {

    for (i=0; i<RADIX; i++) {
      if (GET_BIT(data,i))
	destination[j]='1';
      else 
	destination[j]=' ';
      j++;
      if (((i % 4)==3) && (i!=RADIX-1)) {
	destination[j]=':';
	j++;
      }
    }

    destination[j]='\0';
  }
}


void *m4ri_mm_calloc( int count, int size ) {
#ifdef HAVE_MM_MALLOC
  void *newthing = _mm_malloc(count*size, 16);
#else
  void *newthing = calloc(count, size);
#endif
  if (newthing==NULL) {
    m4ri_die("m4ri_mm_calloc: calloc returned NULL\n");
    return NULL; /* unreachable. */
  }
#ifdef HAVE_MM_MALLOC
  char *b = (char*)newthing;
  memset(b, 0, count*size);
#endif
  return newthing;
}

void *m4ri_mm_malloc( int size ) {
#ifdef HAVE_MM_MALLOC
  void *newthing = _mm_malloc(size, 16);
#else
  void *newthing=malloc( size );
#endif  
  if (newthing==NULL && (size>0)) {
    m4ri_die("m4ri_mm_malloc: malloc returned NULL\n");
    return NULL; /* unreachable */
  }
  else return newthing;
}

void m4ri_mm_free(void *condemned, ...) { 
#ifdef HAVE_MM_MALLOC
  _mm_free(condemned); 
#else
  free(condemned);
#endif  
}

#define RAND_SHORT ((word)(rand()&((1<<16)-1)))

word m4ri_random_word() {
  return RAND_SHORT ^ RAND_SHORT<<16 ^ RAND_SHORT<<32 ^ RAND_SHORT<<48;
}

BIT m4ri_coin_flip() {
  if (rand() < RAND_MAX/2) {
    return 0;
  }  else {
    return 1;
  }
}


#ifdef __GNUC__
void __attribute__ ((constructor)) m4ri_init()
#else
void m4ri_init()
#endif
{
  m4ri_build_all_codes();
}
#ifdef __GNUC__
void __attribute__ ((destructor)) m4ri_fini()
#else
void m4ri_fini()
#endif
{
  m4ri_mmc_cleanup();
  m4ri_destroy_all_codes();
}

#ifdef _MSC_VER
BOOL WINAPI DllMain(
                    HINSTANCE hinstDLL,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
  switch( fdwReason ) 
    { 
    case DLL_PROCESS_ATTACH:
      m4ri_build_all_codes();
       break;
      
    case DLL_THREAD_ATTACH:
      // Do thread-specific initialization.
      break;
      
    case DLL_THREAD_DETACH:
      // Do thread-specific cleanup.
      break;
      
    case DLL_PROCESS_DETACH:
      m4ri_mmc_cleanup();
      m4ri_destroy_all_codes();
      break;
    }
  return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
#endif
