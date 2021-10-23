/*
 * rfdict.c
 * 
 * Implementation of rfdict.h
 * 
 * See the header for further information.
 */

#include "rfdict.h"
#include <stdlib.h>
#include <string.h>

/*
 * ASCII constants.
 */
#define ASCII_UPPER_A (0x41)    /* A */
#define ASCII_UPPER_Z (0x5a)    /* Z */
#define ASCII_LOWER_A (0x61)    /* a */
#define ASCII_LOWER_Z (0x7a)    /* z */

/* Structure prototypes */
struct RFDICT_NODE_TAG;
typedef struct RFDICT_NODE_TAG RFDICT_NODE;

/*
 * The RFDICT structure.
 * 
 * Structure prototype defined in the header.
 * 
 * Use the rfdict_ functions to manipulate this structure.
 */
struct RFDICT_TAG {
  /*
   * Pointer to the root node of the dictionary.
   * 
   * If the dictionary is empty, this shall be a NULL pointer.
   * 
   * All nodes are dynamically allocated, and must be freed before this
   * structure is freed.
   */
  RFDICT_NODE *pRoot;
  
  /*
   * Case sensitivity flag.
   * 
   * If non-zero, comparisons shall be case-sensitive.  If zero,
   * comparisons shall be case-insensitive.
   */
  int sensitive;
};

/*
 * The RFDICT_NODE structure.
 * 
 * (Structure prototype given earlier.)
 */
struct RFDICT_NODE_TAG {
  
  /*
   * Pointer to the parent of this node.
   * 
   * NULL if this node is the root node.
   */
  RFDICT_NODE *pParent;
  
  /*
   * Pointer to the left child node of this node.
   * 
   * NULL if there is no left child of this node.
   * 
   * The left child node and all nodes in that subtree must have key
   * values that are less than the key value of this node.
   */
  RFDICT_NODE *pLeft;
  
  /*
   * Pointer to the right child node of this node.
   * 
   * NULL if there is no right child of this node.
   * 
   * The right child node and all nodes in that subtree must have key
   * values that are greater than the key value of this node.
   */
  RFDICT_NODE *pRight;
  
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
 * rf_ctable_init is non-zero.  Otherwise, it has not been initialized
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
static char rf_ctable[256];
static int  rf_ctable_init = 0;

/* Function prototypes */
static int rfdict_keycmp(
    const char * pKey1,
    const char * pKey2,
    int          sensitive);
static RFDICT_NODE *rfdict_find(RFDICT *pDict, const char *pKey);
static int rfdict_isred(RFDICT_NODE *pNode);
static int rfdict_isblack(RFDICT_NODE *pNode);
static void rfdict_rol(RFDICT_NODE *pNode, RFDICT *pDict);
static void rfdict_ror(RFDICT_NODE *pNode, RFDICT *pDict);

/*
 * Perform a case-insensitive or case-sensitive string comparison.
 * 
 * pKey1 and pKey2 point to the null-terminated strings to compare.
 * Neither may be NULL, but both may point to the same string.
 * 
 * If sensitive is non-zero, then this function calls through to the
 * standard library strcmp() function to perform the comparison.  If
 * sensitive is zero, then the strings will be compared such that 
 * lowercase ASCII letters (0x61-0x7a) are mapped to uppercase ASCII
 * letters (0x41-0x5a).
 * 
 * Parameters:
 * 
 *   pKey1 - the first null-terminated key
 * 
 *   pKey2 - the second null-terminated key
 * 
 *   sensitive - non-zero for case-sensitive comparison, zero for
 *   case-insensitive comparison
 * 
 * Return:
 * 
 *   less than zero, equal to zero, or greater than zero, as key1 is
 *   less than, equal to, or greater than key2
 */
static int rfdict_keycmp(
    const char * pKey1,
    const char * pKey2,
    int          sensitive) {
  
  int result = 0;
  int i = 0;
  int c1 = 0;
  int c2 = 0;
  
  /* Check parameters */
  if ((pKey1 == NULL) || (pKey2 == NULL)) {
    abort();
  }
  
  /* Do the comparison */
  if (sensitive) {
    /* Case-sensitive comparison -- call through to strcmp() */
    result = strcmp(pKey1, pKey2);
    
  } else {
    /* Case-insensitive comparison -- search for the first pair of
     * bytes that are not equal (case-insensitive), or for the first
     * terminating null */
    for(i = 0; (pKey1[i] != 0) && (pKey2[i] != 0); i++) {
      
      /* Get the characters */
      c1 = pKey1[i];
      c2 = pKey2[i];
      
      /* Case mapping */
      if ((c1 >= ASCII_LOWER_A) && (c1 <= ASCII_LOWER_Z)) {
        c1 -= (ASCII_LOWER_A - ASCII_UPPER_A);
      }
      if ((c2 >= ASCII_LOWER_A) && (c2 >= ASCII_LOWER_Z)) {
        c2 -= (ASCII_LOWER_A - ASCII_UPPER_A);
      }
      
      /* Comparison */
      if (c1 != c2) {
        break;
      }
    }
    
    /* If we stopped on null termination, fetch the characters */
    if ((pKey1[i] == 0) || (pKey2[i] == 0)) {
      c1 = pKey1[i];
      c2 = pKey2[i];
    }
    
    /* Result is comparison of c1 and c2 */
    if (c1 == c2) {
      result = 0;
    } else if (c1 > c2) {
      result = 1;
    } else if (c1 < c2) {
      result = -1;
    } else {
      abort();  /* shouldn't happen */
    }
  }
  
  /* Return result */
  return result;
}

/*
 * Find a node in the dictionary matching the given key.
 * 
 * NULL is returned if no node in the dictionary matches that key.
 * Comparisons are case-sensitive or case-insensitive depending on the
 * setting of the sensitive flag in the dictionary.
 * 
 * pDict is the dictionary to search.
 * 
 * pKey is the null-terminated key to find.
 * 
 * Parameters:
 * 
 *   pDict - the dictionary
 * 
 *   pKey - the key to search for
 * 
 * Return:
 * 
 *   the dictionary node matching the key, or NULL if no node matches
 *   the key
 */
static RFDICT_NODE *rfdict_find(RFDICT *pDict, const char *pKey) {
  
  RFDICT_NODE *pCurrent = NULL;
  int retval = 0;
  
  /* Check parameters */
  if ((pDict == NULL) || (pKey == NULL)) {
    abort();
  }
  
  /* Start at root (or NULL if dictionary is empty) */
  pCurrent = pDict->pRoot;
  
  /* Search until node found or we've gone through everything */
  while (pCurrent != NULL) {
    
    /* Compare to current node */
    retval = rfdict_keycmp(
              pKey, &((pCurrent->key)[0]), pDict->sensitive);
    
    /* Done if equal, else go down appropriate branch */
    if (retval == 0) {
      /* Equal -- done */
      break;
    
    } else if (retval < 0) {
      /* Key less than current node */
      pCurrent = pCurrent->pLeft;
      
    } else if (retval > 0) {
      /* Key greater than current node */
      pCurrent = pCurrent->pRight;
      
    } else {
      abort();  /* shouldn't happen */
    }
  }
  
  /* Return the matching node or NULL */
  return pCurrent;
}

/*
 * Check whether a given node is red.
 * 
 * NULL may be passed.  This function returns non-zero only if the
 * passed node is not NULL and it is red.
 * 
 * Parameters:
 * 
 *   pNode - the node to check, or NULL
 * 
 * Return:
 * 
 *   non-zero if the node exists and is red, zero otherwise
 */
static int rfdict_isred(RFDICT_NODE *pNode) {
  
  int result = 0;
  
  if (pNode != NULL) {
    if (pNode->red) {
      result = 1;
    } else {
      result = 0;
    }
  
  } else {
    result = 0;
  }
  
  return result;
}

/*
 * Check whether a given node is black or doesn't exist.
 * 
 * NULL may be passed.  This function returns non-zero if the passed
 * node is not NULL and it is black, or if the passed node is NULL.
 * 
 * Parameters:
 * 
 *   pNode - the node to check, or NULL
 * 
 * Return:
 * 
 *   non-zero if the node does not exist or it exists and is black, zero
 *   otherwise
 */
static int rfdict_isblack(RFDICT_NODE *pNode) {
  
  int result = 0;
  
  if (pNode != NULL) {
    if (pNode->red) {
      result = 0;
    } else {
      result = 1;
    }
  
  } else {
    result = 1;
  }
  
  return result;
}

/*
 * Rotate a node left.
 * 
 * The provided node may not be NULL.  The right child of the node must
 * exist or a fault will occur.
 * 
 * Let N be the provided node, and R be its right child.  R's left
 * subtree becomes N's right subtree, and N becomes R's left child.
 * Both nodes swap parents in the process.
 * 
 * If the provided node was originally the root node of the dictionary,
 * the dictionary is updated so that R becomes the new root.
 * 
 * Parameters:
 * 
 *   pNode - the node to rotate
 * 
 *   pDict - the dictionary
 */
static void rfdict_rol(RFDICT_NODE *pNode, RFDICT *pDict) {
  
  RFDICT_NODE *pR = NULL;
  
  /* Check parameter */
  if ((pNode == NULL) || (pDict == NULL)) {
    abort();
  }
  if (pNode->pRight == NULL) {
    abort();
  }
  
  /* Define right child */
  pR = pNode->pRight;
  
  /* Perform rotation */
  pNode->pRight = pR->pLeft;
  pR->pLeft = pNode;
  
  /* Update parent of the swapped subtree, if it isn't empty */
  if (pNode->pRight != NULL) {
    (pNode->pRight)->pParent = pNode;
  }
  
  /* Swap parents of the two nodes */
  pR->pParent = pNode->pParent;
  pNode->pParent = pR;
  
  /* Update new parent, or root node */
  if (pR->pParent != NULL) {
    /* Update parent */
    if ((pR->pParent)->pLeft == pNode) {
      (pR->pParent)->pLeft = pR;
      
    } else if ((pR->pParent)->pRight == pNode) {
      (pR->pParent)->pRight = pR;
      
    } else {
      abort();  /* shouldn't happen */
    }
  
  } else {
    /* Update root */
    pDict->pRoot = pR;
  }
}

/*
 * Rotate a node right.
 * 
 * The provided node may not be NULL.  The left child of the node must
 * exist or a fault will occur.
 * 
 * Let N be the provided node, and L be its left child.  L's right
 * subtree becomes N's left subtree, and N becomes L's right child.
 * Both nodes swap parents in the process.
 * 
 * If the provided node was originally the root node of the dictionary,
 * the dictionary is updated so that L becomes the new root.
 * 
 * Parameters:
 * 
 *   pNode - the node to rotate
 * 
 *   pDict - the dictionary
 */
static void rfdict_ror(RFDICT_NODE *pNode, RFDICT *pDict) {
  
  RFDICT_NODE *pL = NULL;
  
  /* Check parameters */
  if ((pNode == NULL) || (pDict == NULL)) {
    abort();
  }
  if (pNode->pLeft == NULL) {
    abort();
  }
  
  /* Define left child */
  pL = pNode->pLeft;
  
  /* Perform rotation */
  pNode->pLeft = pL->pRight;
  pL->pRight = pNode;
  
  /* Update parent of the swapped subtree, if it isn't empty */
  if (pNode->pLeft != NULL) {
    (pNode->pLeft)->pParent = pNode;
  }
  
  /* Swap parents of the two nodes */
  pL->pParent = pNode->pParent;
  pNode->pParent = pL;
  
  /* Update new parent, or root node */
  if (pL->pParent != NULL) {
    /* Update parent */
    if ((pL->pParent)->pLeft == pNode) {
      (pL->pParent)->pLeft = pL;
      
    } else if ((pL->pParent)->pRight == pNode) {
      (pL->pParent)->pRight = pL;
      
    } else {
      abort();  /* shouldn't happen */
    }
  
  } else {
    /* Update root */
    pDict->pRoot = pL;
  }
}

/* 
 * Public functions
 * ================
 * 
 * See the header for specifications.
 */

/*
 * rf_ctable_prepare function.
 */
void rf_ctable_prepare(void) {
  
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
  if (!rf_ctable_init) {
    
    /* Clear the table to zero */
    memset(&(rf_ctable[0]), 0, 256);
    
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
      if (rf_ctable[source_c] != 0) {
        abort();
      }
      
      /* Set the record in the mapping table */
      rf_ctable[source_c] = (char) ascii_c;
    }
    
    /* Set the initialization flag */
    rf_ctable_init = 1;
  }
}

/*
 * rf_ctable_ascii function.
 */
int rf_ctable_ascii(int source_c) {
  
  int ascii_c = 0;
  
  /* Check range of source_c */
  if ((source_c < -128) || (source_c > 255)) {
    abort();
  }
  
  /* Initialize table if not already initialized */
  rf_ctable_prepare();
  
  /* If source_c is negative, add 256 to make it positive */
  if (source_c < 0) {
    source_c += 256;
  }
  
  /* Look up the equivalent ASCII code */
  ascii_c = rf_ctable[source_c];
  
  /* Fault if not recognized */
  if (ascii_c == 0) {
    abort();
  }
  
  /* Return mapped ASCII character */
  return ascii_c;
}

/*
 * rfdict_alloc function.
 */
RFDICT *rfdict_alloc(int sensitive) {
  
  RFDICT *pDict = NULL;
  
  /* Allocate dictionary structure and clear it */
  pDict = (RFDICT *) malloc(sizeof(RFDICT));
  if (pDict == NULL) {
    abort();
  }
  memset(pDict, 0, sizeof(RFDICT));
  
  /* Initialize the dictionary */
  pDict->pRoot = NULL;
  pDict->sensitive = sensitive;
  
  /* Return the dictionary */
  return pDict;
}

/*
 * rfdict_free function.
 */
void rfdict_free(RFDICT *pDict) {
  
  RFDICT_NODE *pNode = NULL;
  RFDICT_NODE *pParent = NULL;
  
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

/*
 * rfdict_insert function.
 */
int rfdict_insert(
    RFDICT     * pDict,
    const char * pKey,
    long         val,
    int          translate) {
  
  size_t slen = 0;
  RFDICT_NODE *pNode = NULL;
  RFDICT_NODE *pCurrent = NULL;
  RFDICT_NODE *pParent = NULL;
  RFDICT_NODE *pGrand = NULL;
  char *pc = NULL;
  int status = 1;
  int retval = 0;
  
  /* Check parameters */
  if ((pDict == NULL) || (pKey == NULL)) {
    abort();
  }
  
  /* Get size of string, not including terminating null */
  slen = strlen(pKey);
  
  /* Make sure key size isn't too large */
  if (slen > RFDICT_MAXKEY) {
    abort();
  }
  
  /* Allocate a new node */
  pNode = (RFDICT_NODE *) malloc(sizeof(RFDICT_NODE) + slen);
  if (pNode == NULL) {
    abort();
  }
  memset(pNode, 0, sizeof(RFDICT_NODE) + slen);
  
  /* Initialize node */
  pNode->pParent = NULL;
  pNode->pLeft = NULL;
  pNode->pRight = NULL;
  pNode->val = val;
  pNode->red = 0;
  strcpy(&((pNode->key)[0]), pKey);
  
  /* If translation was requested, perform translation */
  if (translate) {
    for(pc = &((pNode->key)[0]); *pc != 0; pc++) {
      *pc = (char) rf_ctable_ascii(*pc);
    }
  }
  
  /* If dictionary is case-insensitive, map lowercase letters to
   * uppercase */
  if (pDict->sensitive == 0) {
    for(pc = &((pNode->key)[0]); *pc != 0; pc++) {
      if ((*pc >= ASCII_LOWER_A) && (*pc <= ASCII_LOWER_Z)) {
        *pc = *pc - (ASCII_LOWER_A - ASCII_UPPER_A);
      }
    }
  }
  
  /* We now have to insert the new node into the search tree */
  if (pDict->pRoot == NULL) {
    
    /* Search tree is currently empty, so set the color of the node to
     * black and set it as the root node */
    pDict->pRoot = pNode;
    pNode->pParent = NULL;
    pNode->red = 0;
    
  } else {
    /* Search tree is not empty, so we need to find the appropriate
     * position */
    pCurrent = pDict->pRoot;
    for(retval = strcmp(&((pCurrent->key)[0]), &((pNode->key)[0]));
        retval != 0;
        retval = strcmp(&((pCurrent->key)[0]), &((pNode->key)[0]))) {
      
      /* We are done if current node is greater than new node and the
       * left branch of current node is empty, or if current node is
       * less than new node and the right branch of current node is
       * empty; otherwise, proceed down the appropriate branch */
      if (((pCurrent->pLeft == NULL) && (retval > 0)) ||
          ((pCurrent->pRight == NULL) && (retval < 0))) {
        /* Done */
        break;
      
      } else {
        /* Not done yet -- proceed down appropriate branch */
        if (retval > 0) {
          pCurrent = pCurrent->pLeft;
        
        } else if (retval < 0) {
          pCurrent = pCurrent->pRight;
          
        } else {
          abort();  /* shouldn't happen */
        }
      }
    }
    
    /* Insert the new node into the search tree, or free the new node
     * and set error status if a duplicate key already exists in the
     * tree */
    if (retval > 0) {
      /* Set the node in the left branch of current, and set color to
       * red */
      pCurrent->pLeft = pNode;
      pNode->pParent = pCurrent;
      pNode->red = 1;
    
    } else if (retval < 0) {
      /* Set the node in the right branch of current, and set color to
       * red */
      pCurrent->pRight = pNode;
      pNode->pParent = pCurrent;
      pNode->red = 1;
    
    } else {
      /* Duplicate key error -- set error status and free node new */
      status = 0;
      free(pNode);
      pNode = NULL;
    }
  }
  
  /* 
   * If new node is red, it is not the root of the tree.  In this case,
   * check whether the parent is red.  If the parent is red, then the
   * tree needs to be rebalanced because red nodes are not allowed to
   * have red parents.
   * 
   * If the parent is red, then the parent of the parent (the
   * grandparent) must also exist, because root nodes are black, and so
   * the parent couldn't be a root node.  The grandparent must
   * furthermore be black, because red nodes can't have red parents.
   * 
   * The first case we're going to handle is the (black) grandparent
   * with two red children.  In this case, change both of the
   * grandparent's children to black and change the grandparent to red.
   * Set the grandparent as the new current node and apply the
   * rebalancing procedures again, unless the new current nde is the
   * root node, in which case color it black and no further rebalancing.
   */
  if (status) {
    if (rfdict_isred(pNode) && rfdict_isred(pNode->pParent)) {
      
      /* Get the grandparent node, which much exist */
      pGrand = (pNode->pParent)->pParent;
      if (pGrand == NULL) {
        abort();
      }
      
      /* Rebalance while both of grandparent's children are red */
      while(rfdict_isred(pGrand->pLeft) &&
            rfdict_isred(pGrand->pRight)) {
        
        /* Set grandparent's children to black color */
        (pGrand->pLeft)->red = 0;
        (pGrand->pRight)->red = 0;
        
        /* If grandparent is the root node, set it to black; else, set
         * it to red */
        if (pGrand->pParent == NULL) {
          pGrand->red = 0;
        } else {
          pGrand->red = 1;
        }
        
        /* Set new node to grandparent node to continue process */
        pNode = pGrand;
        
        /* If new node is red and its parent is red, update grandparent
         * and continue rebalancing loop; else, done with this
         * rebalancing step */
        if (rfdict_isred(pNode) && rfdict_isred(pNode->pParent)) {
          pGrand = (pNode->pParent)->pParent;
          if (pGrand == NULL) {
            abort();
          }
        } else {
          break;
        }
      }
    }
  }
    
  /* 
   * We've now handled all rebalancing cases where the grandparent has
   * two red children.
   * 
   * We now must handle the cases where the grandparent has one black
   * child, or a child that is missing.  We'll handle these cases with
   * rotations, and unlike the former rebalancing case, we don't need to
   * process this in a loop going back up to the root.
   * 
   * We only need to rebalance if the parent of the current node is red.
   * If the parent of the current node is black, then there is no
   * violation of the red node rules to fix.
   */
  if (status) {
    if (rfdict_isred(pNode) && rfdict_isred(pNode->pParent)) {
      
      /* Get the grandparent node, which much exist */
      pGrand = (pNode->pParent)->pParent;
      if (pGrand == NULL) {
        abort();
      }
      
      /* Rebalance if one of the grandparent's children is black or
       * missing */
      if (rfdict_isblack(pGrand->pLeft) ||
          rfdict_isblack(pGrand->pRight)) {
        
        /* Get parent node */
        pParent = pNode->pParent;
        
        /* Handle rebalancing cases */
        if ((pNode == pParent->pRight) && (pParent == pGrand->pLeft)) {
          /* New node is right child of parent and parent is left child
           * of grandparent */
          pNode->red = 0;
          pGrand->red = 1;
          rfdict_rol(pParent, pDict);
          rfdict_ror(pGrand, pDict);
          
        } else if ((pNode == pParent->pLeft) &&
                    (pParent == pGrand->pRight)) {
          /* New node is left child of parent and parent is right child
           * of grandparent */
          pNode->red = 0;
          pGrand->red = 1;
          rfdict_ror(pParent, pDict);
          rfdict_rol(pGrand, pDict);
          
        } else if ((pNode == pParent->pLeft) &&
                    (pParent == pGrand->pLeft)) {
            /* New node is left child of parent and parent is left child
             * of grandparent */
            pParent->red = 0;
            pGrand->red = 1;
            rfdict_ror(pGrand, pDict);
            
        } else if ((pNode == pParent->pRight) &&
                    (pParent == pGrand->pRight)) {
            /* New node is right child of parent and parent is right
             * child of grandparent */
            pParent->red = 0;
            pGrand->red = 1;
            rfdict_rol(pGrand, pDict);
            
        } else {
          abort();  /* shouldn't happen */
        }
        
      }
    }
  }
  
  /* Return status */
  return status;
}

/*
 * rfdict_get function.
 */
long rfdict_get(RFDICT *pDict, const char *pKey, long dvalue) {

  RFDICT_NODE *pNode = NULL;
  long result = 0;
  
  /* Check parameters */
  if ((pDict == NULL) || (pKey == NULL)) {
    abort();
  }
  
  /* Search for the node */
  pNode = rfdict_find(pDict, pKey);
  
  /* If node found, take value from that; else, use dvalue */
  if (pNode != NULL) {
    result = pNode->val;
  } else {
    result = dvalue;
  }
  
  /* Return result */
  return result;
}
