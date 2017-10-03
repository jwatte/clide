#if !defined(clide_filesystem_h)
#define clide_filesystem_h

#include <memory.h>

/* If a/b fits into the destination, generate that path into dst and return dst. 
 * If they don't fit, set dst to empty string and return NULL.
 */
char *fs_concat(char *dst, size_t dstsize, char const *a, char const *b);

#endif  //  clide_filesystem_h
