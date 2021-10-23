#ifndef SHASTINA_UTIL_H_INCLUDED
#define SHASTINA_UTIL_H_INCLUDED

/*
 * shastina_util.h
 * 
 * Utility module for Shastina.  This is completely separate from the
 * main Shastina module, but it has auxiliary functions that are useful.
 */

#include <stddef.h>

/* Structure prototypes */
struct SNDICT_TAG;
typedef struct SNDICT_TAG SNDICT;

/*
 * The maximum length of a dictionary key in bytes, not including the
 * terminating null.
 * 
 * This value plus the size of the SNDICT_NODE structure (defined in the
 * implementation) must not exceed the maximum value of a size_t and
 * must not exceed the range of a signed int, or undefined behavior
 * occurs.
 */
#define SNDICT_MAXKEY (16384)

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
void snu_ctable_prepare(void);

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
int snu_ctable_ascii(int source_c);

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
SNDICT *sndict_alloc(int sensitive);

/*
 * Free a dictionary object.
 * 
 * The call is ignored if the passed pointer is NULL.
 * 
 * Parameters:
 * 
 *   pDict - the dictionary to free
 */
void sndict_free(SNDICT *pDict);

/*
 * Insert a new key/value pair into a given dictionary.
 * 
 * pDict is the dictionary to insert the key into.
 * 
 * pKey points to the key to insert.  This must be a null-terminated
 * string, which may be empty.  The key may not be equal to any key that
 * is already in the dictionary, or the function will fail.  If the
 * dictionary was created as case-insensitive, then keys must be exactly
 * the same to match.  If the dictionary was created as case-sensitive,
 * uppercase characters A-Z are treated as equivalent to lowercase
 * letters a-z, but otherwise keys must be exactly the same to match.
 * The key string may contain characters of any value (except for zero,
 * which is used as the terminating null character).
 * 
 * The length of the key may not exceed SNDICT_MAXKEY or a fault occurs.
 * 
 * val is the value to associate with the given key.  This may have any
 * long value.
 * 
 * If the translate flag is zero, then the key string is used as-is, and
 * may contain bytes of any value, as described above.  If the translate
 * flag is non-zero, each character in the key string will be translated
 * through snu_ctable_ascii() before being stored in the dictionary (see
 * that function for further information).  If the translate flag is
 * set, then each character in the string must be translateable by
 * snu_ctable_ascii() or a fault will occur.
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
 *   translate - non-zero for character translation, zero otherwise
 * 
 * Return:
 * 
 *   non-zero if successful, zero if function failed because key was
 *   already present in the dictionary
 */
int sndict_insert(
    SNDICT     * pDict,
    const char * pKey,
    long         val,
    int          translate);

/*
 * Get the value associated with a given key in a dictionary.
 * 
 * pDict is the dictionary to query.
 * 
 * pKey is the null-terminated key to check for.  No character
 * translation is performed on the key.  Comparisons will be
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
long sndict_get(SNDICT *pDict, const char *pKey, long dvalue);

#endif
