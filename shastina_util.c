/*
 * shastina_util.c
 * 
 * Utility module for Shastina.  This is completely separate from the
 * main Shastina module, but it has auxiliary functions that are useful.
 */

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Structure prototypes */
struct SNDICT_TAG;
typedef struct SNDICT_TAG SNDICT;

struct SNDICT_NODE_TAG;
typedef struct SNDICT_NODE_TAG SNDICT_NODE;

/*
 * The SNDICT structure.
 * 
 * (Structure prototype given earlier.)
 * @@TODO: change above to header
 * 
 * Use the sndict_ functions to manipulate this structure.
 */
struct SNDICT_TAG {
  /*
   * Pointer to the root node of the dictionary.
   * 
   * If the dictionary is empty, this shall be a NULL pointer.
   * 
   * All nodes are dynamically allocated, and must be freed before this
   * structure is freed.
   */
  SNDICT_NODE *pRoot;
  
  /*
   * Case sensitivity flag.
   * 
   * If non-zero, comparisons shall be case-sensitive.  If zero,
   * comparisons shall be case-insensitive.
   */
  int sensitive;
};

/*
 * The SNDICT_NODE structure.
 * 
 * (Structure prototype given earlier.)
 * @@TODO: change above to header
 */
struct SNDICT_NODE_TAG {
  
  /*
   * Pointer to the parent of this node.
   * 
   * NULL if this node is the root node.
   */
  SNDICT_NODE *pParent;
  
  /*
   * Pointer to the left child node of this node.
   * 
   * NULL if there is no left child of this node.
   * 
   * The left child node and all nodes in that subtree must have key
   * values that are less than the key value of this node.
   */
  SNDICT_NODE *pLeft;
  
  /*
   * Pointer to the right child node of this node.
   * 
   * NULL if there is no right child of this node.
   * 
   * The right child node and all nodes in that subtree must have key
   * values that are greater than the key value of this node.
   */
  SNDICT_NODE *pRight;
  
  /*
   * The value associated with this node.
   * 
   * This is the value that the key maps to.  It may be any long value.
   */
  long val;
  
  /*
   * The red color flag.
   * 
   * If this value is non-zero, the node is a "red" node.  If this value
   * is zero, the node is a "black" node.
   * 
   * The colors of the nodes must follow certain rules to ensure that
   * the binary tree is kept in a proper balance.  The following are the
   * rules:
   * 
   * (1) The root node (the node with a NULL parent pointer) must be
   *     black.
   * 
   * (2) A red node may not have another red node as a parent.
   * 
   * (3) Let the "root path" from a node N be the sequence of nodes
   *     N0, N1, ... Nn, where N0 is the node N, Nn is the root node,
   *     and node N(n+1) is the parent of node N(n) for all n > 0.  Let
   *     the "black depth" of a node be the number of black nodes in the
   *     node's root path.  A node is an "exit node" if its left or
   *     right pointer is NULL, or both are NULL.  All exit nodes in the
   *     tree must have the same black depth.
   */
  char red;
  
  /*
   * The string key of this node.
   * 
   * This is a null-terminated string.  It may be empty.
   * 
   * The actual string data is allocated beyond the end of the
   * structure.  This field must therefore be the last field in the
   * structure.
   * 
   * If case-insensitive comparisons are desired, the key string should
   * already have its letters transformed to make everything uppercase
   * (or everything lowercase).
   */
  char key[1];
  
};

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

/*
 * Allocate a new dictionary object.
 * 
 * The dictionary starts out empty.  If sensitive is non-zero, key
 * comparisons will be case-sensitive.  Otherwise, key comparisons will
 * be case-insensitive.
 * 
 * Dictionaries must be freed with sndict_free().
 * 
 * Parameters:
 * 
 *   sensitive - non-zero for case-sensitive comparisons, zero for
 *   case-insensitive comparisons
 * 
 * Return:
 * 
 *   a new dictionary
 */
SNDICT *sndict_alloc(int sensitive) {
  
  SNDICT *pDict = NULL;
  
  /* Allocate dictionary structure and clear it */
  pDict = (SNDICT *) malloc(sizeof(SNDICT));
  if (pDict == NULL) {
    abort();
  }
  memset(pDict, 0, sizeof(SNDICT));
  
  /* Initialize the dictionary */
  pDict->pRoot = NULL;
  pDict->sensitive = sensitive;
  
  /* Return the dictionary */
  return pDict;
}

/*
 * Free a dictionary object.
 * 
 * The call is ignored if the passed pointer is NULL.
 * 
 * Parameters:
 * 
 *   pDict - the dictionary to free
 */
void sndict_free(SNDICT *pDict) {
  
  SNDICT_NODE *pNode = NULL;
  SNDICT_NODE *pParent = NULL;
  
  /* Only perform operation if point is non-NULL */
  if (pDict != NULL) {
    
    /* Start at the root node (if there is one) */
    pNode = pDict->pRoot;
    
    /* Keep releasing nodes until there are none left */
    while (pNode != NULL) {
      
      /* We can only release nodes that have no child nodes, so keep
       * iterating nodes until we reach a node with no children */
      while((pNode->pLeft != NULL) || (pNode->pRight != NULL)) {
        
        if (pNode->pLeft != NULL) {
          pNode = pNode->pLeft;
        
        } else if (pNode->pRight != NULL) {
          pNode = pNode->pRight;
          
        } else {
          abort();  /* shouldn't happen */
        }
      }
      
      /* We're now at a childless node -- get the parent node, or
       * NULL */
      pParent = pNode->pParent;
      
      /* If there is a parent node, unlink this node from the parent */
      if (pParent != NULL) {
        if (pParent->pLeft == pNode) {
          pParent->pLeft = NULL;
        
        } else if (pParent->pRight == pNode) {
          pParent->pRight = NULL;
          
        } else {
          abort();  /* shouldn't happen */
        }
      }
      
      /* Release current node and set new current node to parent, or
       * NULL if we just released the root node */
      free(pNode);
      pNode = pParent;
    }
    
    /* We've released all nodes; now release the dictionary object */
    free(pDict);
  }
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
