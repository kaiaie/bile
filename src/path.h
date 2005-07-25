/* :tabSize=4:indentSize=4:folding=indent:
 * path - Functions for manipulating directory paths
 */
#ifndef _PATH_H
#define _PATH_H
#include "bool.h"

typedef enum {PATH_HOST, PATH_DRIVE, PATH_DIR, PATH_FILE, PATH_EXT} PathPart;

bool isDosPath(const char *path);
bool isUncPath(const char *path);
char *getPathPart(const char *path, PathPart part);
char *getCanonicalPath(const char *path);
char *getCombinedPath(const char *path1, const char *path2);
char *getCurrentDirectory(void);

#endif /* _PATH_H */
