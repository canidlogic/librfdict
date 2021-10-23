/*
 * test_tree.c
 * 
 * Test program for the dictionary of shastina_util.
 * 
 * This tests the tree using internal interfaces of the module.
 * 
 * Compilation:
 * 
 *   - The source file of shastina_util is included by this file, so
 *     just compile this file by itself.  Do NOT separately specify
 *     shastina_util.c, because that's already included here.
 */

#include "shastina_util.c"  /* The source file, not the header! */

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_MAXLINE (1024)

/* @@TODO: red/black verification procedure */

/*
 * @@TODO:
 */
void print_tree(SNDICT_NODE *pNode, int depth) {
  
  int i = 0;
  
  /* Check parameter */
  if ((pNode == NULL) || (depth < 0) || (depth >= INT_MAX)) {
    abort();
  }
  
  /* If there is a left subtree, recursively print it */
  if (pNode->pLeft != NULL) {
    print_tree(pNode->pLeft, depth + 1);
  }
  
  /* Print a number of spaces equal to the depth */
  for(i = 0; i < depth; i++) {
    putchar(0x20);
  }
  
  /* Print the current key */
  printf("%s\n", &((pNode->key)[0]));
  
  /* If there is a right subtree, recursively print it */
  if (pNode->pRight != NULL) {
    print_tree(pNode->pRight, depth + 1);
  }
}

/*
 * @@TODO:
 */
int main(int argc, char *argv[]) {
  
  SNDICT *pDict = NULL;
  char buf[INPUT_MAXLINE];
  int status = 1;
  int x = 0;
  int line = 0;
  char *pc = NULL;
  
  /* Initialize buffer */
  memset(&(buf[0]), 0, INPUT_MAXLINE);
  
  /* Allocate case-insensitive dictionary */
  pDict = sndict_alloc(0);
  
  /* Read each line of input */
  while (fgets(&(buf[0]), INPUT_MAXLINE, stdin) != NULL) {
    
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
  
  /* Print the tree */
  if (status) {
    if (pDict->pRoot != NULL) {
      print_tree(pDict->pRoot, 0);
    }
  }
  
  /* Free dictionary */
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
