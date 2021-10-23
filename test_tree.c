/*
 * test_tree.c
 * 
 * Test program for the dictionary of rfdict.
 * 
 * This tests the tree using internal interfaces of the module.
 * 
 * A word list with the same format as for test_dict.c is read from
 * standard input.  The tree is verified and printed to standard output.
 * The dictionary is constructed in case-insensitive mode.
 * 
 * Compilation:
 * 
 *   - The source file of rfdict is included by this file, so just
 *     compile this file by itself.  Do NOT separately specify rfdict.c,
 *     because that's already included here.
 */

#include "rfdict.c"   /* The source file, not the header! */

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_MAXLINE (1024)

/*
 * Verify that a given node and its subtree obey the rules of red-black
 * trees, and that nodes are linked properly together.
 * 
 * This procedure is not designed to detect circular linking, and
 * undefined behavior occurs if there are circular references in the
 * tree.  (Proper trees should never have circular references.)
 * Recursion is used, so this function may fail if the recursive depth
 * is very large.  Overflow may happen if the depth exceeds the range of
 * a signed integer.
 * 
 * pNode is the node to verify.  If it is NULL, then this function call
 * is ignored and true is returned.
 * 
 * pParent is the parent of the node to verify, or NULL if the root node
 * is being verified.  The function will check that the parent point of
 * the given node equals this parameter, to make sure the tree is linked
 * properly.  It will also make sure that the key relation of parent
 * nodes to their children holds.
 * 
 * black_depth is the number of black nodes that have been visited prior
 * to this node.  It must be zero or greater.  Zero should be passed
 * when verifying the root node.
 * 
 * pexit_depth is a pointer to a variable used to keep track of the
 * black depth of exit nodes.  This may not be NULL.  The variable
 * should be initialized to -1 before verifying the root node, meaning
 * that no black depth of exit node has been established yet.  When the
 * first exit node is reached, it writes its black depth to this
 * variable.  All subsequent exit nodes that are visited must have the
 * same black depth.
 * 
 * Parameters:
 * 
 *   pNode - the node to verify, or NULL
 * 
 *   pParent - the parent of the node to verify, or NULL if the node to
 *   verify is the root node
 * 
 *   black_depth - the number of black nodes that have been visited
 *   before this node
 * 
 *   pexit_depth - pointer to the exit node black depth variable
 * 
 * Return:
 * 
 *   non-zero if subtree verified, zero if verification failed
 */
int verify_tree(
    RFDICT_NODE *pNode,
    RFDICT_NODE *pParent,
    int black_depth,
    int *pexit_depth) {
  
  int status = 1;
  
  /* Check parameters */
  if ((black_depth < 0) || (pexit_depth == NULL)) {
    abort();
  }
  if (*pexit_depth < -1) {
    abort();
  }
  
  /* Verify only if passed node is not NULL */
  if (pNode != NULL) {
    
    /* First, verify the node's parent pointer matches the passed parent
     * pointer value (which may be NULL if this is the root node) */
    if (pNode->pParent != pParent) {
      status = 0;
      fprintf(stderr, "Parent check failed!\n");
    }
    
    /* Second, if this is not the root node, verify that this node is
     * one of the parent's children; furthermore, if the left node,
     * verify the key is less than the parent's and if the right node,
     * verify the key is greater than the parent's */
    if (status && (pParent != NULL)) {
      if (pParent->pLeft == pNode) {
        /* Left child of parent -- make sure not also right child */
        if (pParent->pRight == pNode) {
          status = 0;
        }
        
        /* Make sure parent's key is greater than current node's key */
        if (status) {
          if (strcmp(&((pParent->key)[0]), &((pNode->key)[0])) <= 0) {
            status = 0;
          }
        }
        
      } else if (pParent->pRight == pNode) {
        /* Right child of parent -- make sure not also left child */
        if (pParent->pLeft == pNode) {
          status = 0;
        }
        
        /* Make sure parent's key is less than current node's key */
        if (status) {
          if (strcmp(&((pParent->key)[0]), &((pNode->key)[0])) >= 0) {
            status = 0;
          }
        }
        
      } else {
        /* Not a child of the parent -- linking error */
        status = 0;
      }
      
      if (!status) {
        fprintf(stderr, "Relation check failed!\n");
      }
    }
    
    /* Third, if this is the root node, verify that it is black */
    if (status && (pParent == NULL)) {
      if (pNode->red) {
        status = 0;
        fprintf(stderr, "Root black check failed!\n");
      }
    }
    
    /* Fourth, if this is a red node, verify that it has a parent that
     * is not a red node */
    if (status && pNode->red) {
      if (pParent != NULL) {
        if (pParent->red) {
          status = 0;
        }
      } else {
        status = 0;
      }
      if (!status) {
        fprintf(stderr, "Red relation check failed!\n");
      }
    }
    
    /* Fifth, if this is a black node, increase the black depth, 
     * faulting if there is an overflow */
    if (status && (!(pNode->red))) {
      if (black_depth < INT_MAX) {
        black_depth++;
      } else {
        abort();
      }
    }
    
    /* Sixth, if this is the first exit node, record its black depth */
    if (status && ((pNode->pLeft == NULL) || (pNode->pRight == NULL)) &&
          (*pexit_depth == -1)) {
      *pexit_depth = black_depth;
    }
    
    /* Seventh, if this is an exit node, make sure it has the right
     * black depth */
    if (status && ((pNode->pLeft == NULL) || (pNode->pRight == NULL))) {
      if (*pexit_depth != black_depth) {
        status = 0;
        fprintf(stderr, "Exit depth check failed!\n");
      }
    }
    
    /* Eighth, recursively verify the subtrees */
    if (status) {
      status = verify_tree(
        pNode->pLeft, pNode, black_depth, pexit_depth);
    }
    if (status) {
      status = verify_tree(
        pNode->pRight, pNode, black_depth, pexit_depth);
    }
  }
  
  /* Return verification results */
  return status;
}

/*
 * Recursively print the tree.
 * 
 * Parameters:
 * 
 *   pNode - the root node to print
 * 
 *   depth - the recursive depth; pass zero at the root note
 * 
 *   pOut - the file to print the tree to (fully sequential)
 */
void print_tree(RFDICT_NODE *pNode, int depth, FILE *pOut) {
  
  int i = 0;
  
  /* Check parameter */
  if ((pNode == NULL) || (depth < 0) || (depth >= INT_MAX)) {
    abort();
  }
  
  /* If there is a left subtree, recursively print it */
  if (pNode->pLeft != NULL) {
    print_tree(pNode->pLeft, depth + 1, pOut);
  }
  
  /* Print a number of spaces equal to the depth */
  for(i = 0; i < depth; i++) {
    putc(0x20, pOut);
  }
  
  /* Print the current key, along with a color prefix */
  if (pNode->red) {
    fprintf(pOut, "r:%s\n", &((pNode->key)[0]));
  } else {
    fprintf(pOut, "b:%s\n", &((pNode->key)[0]));
  }
  
  /* If there is a right subtree, recursively print it */
  if (pNode->pRight != NULL) {
    print_tree(pNode->pRight, depth + 1, pOut);
  }
}

/*
 * Program entrypoint.
 */
int main(int argc, char *argv[]) {
  
  RFDICT *pDict = NULL;
  char buf[INPUT_MAXLINE];
  int status = 1;
  int x = 0;
  int line = 0;
  int exit_depth = -1;
  char *pc = NULL;
  
  /* Initialize buffer */
  memset(&(buf[0]), 0, INPUT_MAXLINE);
  
  /* Allocate case-insensitive dictionary */
  pDict = rfdict_alloc(0);
  
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
      if (!rfdict_insert(pDict, &(buf[x]), line)) {
        fprintf(stderr, "Duplicate key!  Line %d\n", line);
        status = 0;
      }
    }
    
    /* Verify new tree is valid */
    if (status) {
      exit_depth = -1;
      if (!verify_tree(pDict->pRoot, NULL, 0, &exit_depth)) {
        status = 0;
        fprintf(stderr, "Line %d: Tree verification failed!\n", line);
        fprintf(stderr, "Erroneous tree:\n");
        print_tree(pDict->pRoot, 0, stderr);
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
  
  /* Verify the tree and print exit depth */
  if (status) {
    exit_depth = -1;
    if (verify_tree(pDict->pRoot, NULL, 0, &exit_depth)) {
      printf("\nTree verified, black depth %d.\n", exit_depth);
      
    } else {
      status = 0;
      fprintf(stderr, "Tree verification failed!\n");
    }
  }
  
  /* Print the tree */
  if (status) {
    if (pDict->pRoot != NULL) {
      print_tree(pDict->pRoot, 0, stdout);
    }
  }
  
  /* Free dictionary */
  rfdict_free(pDict);
  pDict = NULL;
  
  /* Return inverted status */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}
