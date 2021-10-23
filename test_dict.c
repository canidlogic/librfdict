/*
 * test_dict.c
 * 
 * Test program for the dictionary of shastina_util.
 * 
 * This tests the dictionary using the external interface of the module.
 * 
 * Syntax:
 * 
 *   test_dict [flag] [key]
 * 
 * Parameters:
 * 
 *   [flag] is either "s" or "i" to select a case-Sensitive or
 *   case-Insensitive dictionary mode.
 * 
 *   [key] is the key to search for.
 * 
 * Operation:
 * 
 *   A list of string keys is read from standard input, one key per
 *   line, US-ASCII encoding, start and end trimming, and blank lines
 *   ignored.  These are used to build a dictionary, with the values
 *   equal to the line number.  The dictionary is then queried for the
 *   key passed as a command-line parameter, with the line number
 *   returned, or a report that the requested key is not in the
 *   dictionary.
 * 
 * Compilation:
 * 
 *   - Compile with shastina_util.c
 */

#include "shastina_util.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_MAXLINE (1024)

/*
 * @@TODO:
 */
int main(int argc, char *argv[]) {
  
  SNDICT *pDict = NULL;
  char buf[INPUT_MAXLINE];
  int status = 1;
  int x = 0;
  int line = 0;
  int result = 0;
  int sensitive = 0;
  char *pc = NULL;
  
  /* Initialize buffer */
  memset(&(buf[0]), 0, INPUT_MAXLINE);
  
  /* Check parameters */
  if (argc < 0) {
    abort();
  }
  if (argc > 0) {
    if (argv == NULL) {
      abort();
    }
    for(x = 0; x < argc; x++) {
      if (argv[x] == NULL) {
        abort();
      }
    }
  }
  
  /* Make sure exactly two command-line parameters */
  if (argc != 3) {
    fprintf(stderr, "Expecting two parameters!\n");
    status = 0;
  }
  
  /* Use the first parameter to determine sensitive flag */
  if (status) {
    if (strcmp(argv[1], "s") == 0) {
      sensitive = 1;
    
    } else if (strcmp(argv[1], "i") == 0) {
      sensitive = 0;
      
    } else {
      fprintf(stderr, "Unrecognized sensitivity flag!\n");
      status = 0;
    }
  }
  
  /* Allocate dictionary */
  if (status) {
    pDict = sndict_alloc(sensitive);
  }
  
  /* Read each line of input */
  while (status && (fgets(&(buf[0]), INPUT_MAXLINE, stdin) != NULL)) {
    
    /* Fail if length is up to full buffer, because the line might be
     * too long in that case */
    if (strlen(&(buf[0])) >= (INPUT_MAXLINE - 1)) {
      fprintf(stderr, "Input line is too long!\n");
      status = 0;
    }
    
    /* Fail if line count about to overflow -- else, increment line
     * count */
    if (status) {
      if (line >= INT_MAX) {
        fprintf(stderr, "Too many lines in input!\n");
        status = 0;
      } else {
        line++;
      }
    }
    
    /* End-trim characters not in visible ASCII range */
    if (status) {
      for(x = ((int) strlen(&(buf[0]))) - 1;
          x >= 0;
          x--) {
        if ((buf[x] >= 0x20) && (buf[x] <= 0x7e)) {
          break;
        } else {
          buf[x] = (char) 0;
        }
      }
    }
    
    /* Lead-trim characters not in visible ASCII range */
    if (status) {
      for(x = 0;
          ((buf[x] < 0x20) || (buf[x] > 0x7e)) &&
            (buf[x] != 0);
          x++);
    }
    
    /* Insert trimmed string as key, with line number as value, unless
     * trimmed length is zero, in which case line is blank */
    if (status && (strlen(&(buf[x])) > 0)) {
      if (!sndict_insert(pDict, &(buf[x]), line, 0)) {
        fprintf(stderr, "Duplicate key!  Line %d\n", line);
        status = 0;
      }
    }
    
    /* Leave loop if error */
    if (!status) {
      break;
    }
  }
  
  /* Check status of input */
  if (status) {
    if (!feof(stdin)) {
      fprintf(stderr, "I/O error!\n");
      status = 0;
    }
  }
  
  /* Copy passed key into buffer */
  if (status) {
    if (strlen(argv[2]) >= (INPUT_MAXLINE - 1)) {
      fprintf(stderr, "Provided key is too long!\n");
      status = 0;
    }
  }
  if (status) {
    strcpy(&(buf[0]), argv[2]);
  }
  
  /* Convert key to ASCII */
  if (status) {
    for(x = 0; buf[x] != 0; x++) {
      buf[x] = snu_ctable_ascii(buf[x]);
    }
  }
  
  /* End-trim characters not in visible ASCII range */
  if (status) {
    for(x = ((int) strlen(&(buf[0]))) - 1;
        x >= 0;
        x--) {
      if ((buf[x] >= 0x20) && (buf[x] <= 0x7e)) {
        break;
      } else {
        buf[x] = (char) 0;
      }
    }
  }
  
  /* Lead-trim characters not in visible ASCII range */
  if (status) {
    for(x = 0;
        ((buf[x] < 0x20) || (buf[x] > 0x7e)) &&
          (buf[x] != 0);
        x++);
  }
  
  /* Find the key in the dictionary, returning -1 if not found */
  if (status) {
    result = sndict_get(pDict, &(buf[x]), -1);
  }
  
  /* Report result */
  if (status) {
    if (result != -1) {
      printf("Key is on line %d\n", result);
    } else {
      printf("Key not found!\n");
    }
  }
  
  /* Free dictionary if allocated */
  sndict_free(pDict);
  pDict = NULL;
  
  /* Return inverted status */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
