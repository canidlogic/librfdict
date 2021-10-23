/*
 * shastina_util.c
 * 
 * Utility module for Shastina.  This is completely separate from the
 * main Shastina module, but it has auxiliary functions that are useful.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*
 * The character mapping table.
 * 
 * The character table has 256 character elements.  It is only valid if
 * snu_ctable_init is non-zero.  Otherwise, it has not been initialized
 * yet.
 * 
 * When it is initialized, it maps characters from the character set
 * used in C source files into US-ASCII.  All visible, printing US-ASCII
 * characters are supported, as well as the space character.  However,
 * line feed, tabs, carriage returns, and other controls are NOT
 * supported.
 * 
 * To do this, first of all convert the char value into an integer.  If
 * the char value is less than zero (in the case of a signed char type),
 * add 256 to it, such that -1 maps to 255, -2 maps to 254, and so forth
 * down to -128 mapping to 128.
 * 
 * Use the adjusted character value (range 0-255) as an index into the
 * table.  If the value at the table is zero, the character has no ASCII
 * mapping.  Otherwise, it the character maps to the given ASCII value.
 */
static char snu_ctable[256];
static int  snu_ctable_init = 0;

/* Function prototypes */

/* 
 * Public functions
 * ================
 */

/*
 * Initialize the character mapping table if not already initialized.
 * 
 * This call is ignored if the character mapping table is already
 * initialized.
 * 
 * This function will be called automatically when character mapping is
 * used.  However, if there is any multithreading going on, this
 * function should be explicitly called at the start of the program, to
 * avoid the situation of two threads trying to initialize the character
 * mapping table at the same time.
 * 
 * This function is not thread safe, unless the table has already been
 * initialized.
 */
void snu_ctable_prepare(void) {
  
  /* ASCII characters from 0x20 to 0x7E, represented in the character
   * set used in C source files */
  static char char_ref[] =
    " !\"#$%&'()*+,-./"
    "0123456789:;<=>?"
    "@ABCDEFGHIJKLMNO"
    "PQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmno"
    "pqrstuvwxyz{|}~";
  int ascii_c = 0;
  int source_c = 0;
  
  /* Only do something if not yet initialized */
  if (!snu_ctable_init) {
    
    /* Clear the table to zero */
    memset(&(snu_ctable[0]), 0, 256);
    
    /* Map all characters in char_ref */
    for(ascii_c = 0x20; ascii_c <= 0x7E; ascii_c++) {
      
      /* Determine the source character code for this ASCII char */
      source_c = char_ref[ascii_c - 0x20];
      
      /* Source character code can't be zero, because we need that for
       * terminating null characters */
      if (source_c == 0) {
        abort();
      }
      
      /* If source character code negative, add 256 to it to turn it
       * into an unsigned value */
      if (source_c < 0) {
        source_c += 256;
      }
      
      /* Table entry for the source value shouldn't be set yet */
      if (snu_ctable[source_c] != 0) {
        abort();
      }
      
      /* Set the record in the mapping table */
      snu_ctable[source_c] = (char) ascii_c;
    }
    
    /* Set the initialization flag */
    snu_ctable_init = 1;
  }
}

/*
 * Map a character from the character set used in C source files to the
 * US-ASCII character set.
 * 
 * For example, snu_ctable_ascii('a') will return 0x61 (the US-ASCII
 * code for a lowercase a), even if ((int) 'a') actually is something
 * else according to the character set used in C source files.
 * 
 * The only characters supported by this mapping function are the
 * visible US-ASCII characters and the space character (US-ASCII range
 * 0x20-0x7e).  A fault occurs if any other character is passed.
 * 
 * This function is not thread safe, unless the character mapping table
 * has already been initialized with snu_ctable_prepare().  If thread
 * safe operation is required, call snu_ctable_prepare() explicitly at
 * the start of the program.  Otherwise, it will be called implicitly on
 * the first call to this function.
 * 
 * Parameters:
 * 
 *   source_c - a character in the character set used in C source files
 * 
 * Return:
 * 
 *   the US-ASCII equivalent of this character
 */
int snu_ctable_ascii(int source_c) {
  
  int ascii_c = 0;
  
  /* Check range of source_c */
  if ((source_c < -128) || (source_c > 255)) {
    abort();
  }
  
  /* Initialize table if not already initialized */
  snu_ctable_prepare();
  
  /* If source_c is negative, add 256 to make it positive */
  if (source_c < 0) {
    source_c += 256;
  }
  
  /* Look up the equivalent ASCII code */
  ascii_c = snu_ctable[source_c];
  
  /* Fault if not recognized */
  if (ascii_c == 0) {
    abort();
  }
  
  /* Return mapped ASCII character */
  return ascii_c;
}

/* @@TODO: */
#include <stdio.h>

/*
 * @@TODO:
 */
int main(int argc, char *argv[]) {
  
  static char msg[] = "Hello, world!";
  char *pc = NULL;
  
  printf("\"Hello, world!\" is US-ASCII is:\n");
  for(pc = &(msg[0]); *pc != 0; pc++) {
    if (pc != &(msg[0])) {
      printf(" ");
    }
    printf("%02x", snu_ctable_ascii(*pc));
  }
  printf("\n");
  
  return 0;
}
