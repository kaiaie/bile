/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: path.h,v 1.11 2010/07/08 21:04:25 ken Exp $
 */
/**
 * \file path.h
 * Functions for creating and manipulating directory paths
 */
#ifndef PATH_H
#define PATH_H
#include <time.h>
#include "bool.h"

#ifdef _WIN32
#define pu_mkdir(a) mkdir(a)
#else
#define pu_mkdir(a) mkdir(a, 0777)
#define O_BINARY 0
#endif

typedef enum {PATH_HOST, PATH_DRIVE, PATH_DIR, PATH_FILE, PATH_FILEONLY, PATH_EXT} PathPart;
typedef enum {REPLACE_NEVER, REPLACE_OLDER, REPLACE_ALWAYS} ReplaceOption;

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
long getFileSize(const char *fileName);
bool mkdirs(const char *pathname);
bool copyFile(const char *src, const char *dest);
char *getRelativePath(const char *targetFile, const char *relativeTo);
bool copyDirectory(const char *srcDir, const char *destDir, ReplaceOption option, bool copyBackupFiles);

#endif /* PATH_H */
