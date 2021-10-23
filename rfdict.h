#ifndef RFDICT_H_INCLUDED
#define RFDICT_H_INCLUDED

/*
 * rfdict.h
 */

#include <stddef.h>

/* Structure prototypes */
struct RFDICT_TAG;
typedef struct RFDICT_TAG RFDICT;

/*
 * The maximum length of a dictionary key in bytes, not including the
 * terminating null.
 * 
 * This value plus the size of the RFDICT_NODE structure (defined in the
 * implementation) must not exceed the maximum value of a size_t and
 * must not exceed the range of a signed int, or undefined behavior
 * occurs.
 */
#define RFDICT_MAXKEY (16384)

/*
 * Allocate a new dictionary object.
 * 
 * The dictionary starts out empty.  If sensitive is non-zero, key
 * comparisons will be case-sensitive.  Otherwise, key comparisons will
 * be case-insensitive.
 * 
 * Dictionaries must be freed with rfdict_free().
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
RFDICT *rfdict_alloc(int sensitive);

/*
 * Free a dictionary object.
 * 
 * The call is ignored if the passed pointer is NULL.
 * 
 * Parameters:
 * 
 *   pDict - the dictionary to free
 */
void rfdict_free(RFDICT *pDict);

/*
 * Insert a new key/value pair into a given dictionary.
 * 
 * pDict is the dictionary to insert the key into.
 * 
 * pKey points to the key to insert.  This must be a null-terminated
 * string, which may be empty.  The key may not be equal to any key that
 * is already in the dictionary, or the function will fail.  If the
 * dictionary was created as case-sensitive, then keys must be exactly
 * the same to match.  If the dictionary was created as
 * case-insensitive, uppercase characters A-Z are treated as equivalent
 * to lowercase letters a-z, but otherwise keys must be exactly the same
 * to match.  The key string may contain characters of any value (except
 * for zero, which is used as the terminating null character).
 * 
 * The length of the key may not exceed RFDICT_MAXKEY or a fault occurs.
 * 
 * val is the value to associate with the given key.  This may have any
 * long value.
 * 
 * This function returns whether it succeeded.  If the function fails,
 * then the dictionary is unmodified.  Otherwise, the key/value pair is
 * inserted successfully.
 * 
 * Parameters:
 * 
 *   pDict - the dictionary object
 * 
 *   pKey - the key string
 * 
 *   val - the value to associate with the key
 * 
 * Return:
 * 
 *   non-zero if successful, zero if function failed because key was
 *   already present in the dictionary
 */
int rfdict_insert(
    RFDICT     * pDict,
    const char * pKey,
    long         val);

/*
 * Get the value associated with a given key in a dictionary.
 * 
 * pDict is the dictionary to query.
 * 
 * pKey is the null-terminated key to check for.  Comparisons will be
 * case-sensitive or case-insensitive depending on the dictionary
 * setting.
 * 
 * dvalue is the value to return if the key is not present within the
 * dictionary.
 * 
 * Parameters:
 * 
 *   pDict - the dictionary
 * 
 *   pKey - the key string
 * 
 *   dvalue - the default value to return if key not found
 * 
 * Return:
 * 
 *   the value associated with the key, or dvalue if the key is not
 *   present in the dictionary
 */
long rfdict_get(RFDICT *pDict, const char *pKey, long dvalue);

#endif
