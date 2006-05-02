/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: path.h,v 1.5 2006/05/02 23:10:07 ken Exp $
 * path - Functions for manipulating directory paths
 */
#ifndef PATH_H
#define PATH_H
#include <time.h>
#include "bool.h"

typedef enum {PATH_HOST, PATH_DRIVE, PATH_DIR, PATH_FILE, PATH_EXT} PathPart;

bool isDosPath(const char *path);
bool isUncPath(const char *path);
char *getPathPart(const char *path, PathPart part);
char *getCanonicalPath(const char *path);
char *getCombinedPath(const char *path1, const char *path2);
char *getCurrentDirectory(void);
char *buildPath(const char *path1, const char *path2);
bool fileExists(const char *fileName);
bool directoryExists(const char *pathname);
time_t getFileModificationTime(const char *fileName);
bool mkdirs(const char *pathname);
bool copyFile(const char *src, const char *dest);

#endif /* PATH_H */
