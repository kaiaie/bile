/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: path.h,v 1.3 2006/03/12 01:08:03 ken Exp $
 * path - Functions for manipulating directory paths
 */
#ifndef PATH_H
#define PATH_H
#include "bool.h"

typedef enum {PATH_HOST, PATH_DRIVE, PATH_DIR, PATH_FILE, PATH_EXT} PathPart;

bool isDosPath(const char *path);
bool isUncPath(const char *path);
char *getPathPart(const char *path, PathPart part);
char *getCanonicalPath(const char *path);
char *getCombinedPath(const char *path1, const char *path2);
char *getCurrentDirectory(void);

#endif /* PATH_H */
