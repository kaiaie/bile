/* :tabSize=4:indentSize=4:folding=indent:
** $Id: path.h,v 1.14 2010/08/31 15:14:39 ken Exp $
*/
/**
*** \file path.h
*** \brief Functions for creating and manipulating directory paths
**/
#ifndef PATH_H
#define PATH_H
#include <time.h>
#include "bool.h"

/* mkdir() on Windows does not take a umask parameter */
#ifdef _WIN32
#define pu_mkdir(a) mkdir(a)
#else
#define pu_mkdir(a) mkdir(a, 0777)
#define O_BINARY 0
#endif

/** \brief Parts of a path returned by getPathPart() */
typedef enum {
	/** \brief If a UNC path, return the name of the server */
	PATH_HOST, 
	/** \brief If a DOS path, return the drive letter */
	PATH_DRIVE, 
	/** \brief Return the file name and directory minus the drive letter or 
	*** server name if a DOS or UNC path
	**/
	PATH_DIR,
	/** \brief Return the file name and its extension */
	PATH_FILE, 
	/** \brief Return the file name with the extension stripped */
	PATH_FILEONLY, 
	/** \brief Return the file extension only */
	PATH_EXT
} PathPart;

/** \brief Indicates how files should be replaced by copyDirectory() */
typedef enum {
	/** \brief Existing files in the destination should never be replaced */
	REPLACE_NEVER, 
	/** \brief Overwrite the destination file if the source file is newer */
	REPLACE_OLDER, 
	/** \brief Always overwrite the destination file */
	REPLACE_ALWAYS
} ReplaceOption;

/** Callback function when copyDirectory() enters a directory */
typedef void (*EnterDirCallback)(const char *, void *);

/** Callback function when copyDirectory() leaves a directory */
typedef void (*LeaveDirCallback)(void *);

/** Callback function when copyDirectory() copies a file */
typedef void (*CopyFileCallback)(const char *, void *);

bool   isDosPath(const char *path);
bool   isUncPath(const char *path);
char   *getPathPart(const char *path, PathPart part);
char   *getCanonicalPath(const char *path);
char   *getCombinedPath(const char *path1, const char *path2);
char   *getCurrentDirectory(void);
char   *buildPath(const char *path1, const char *path2);
bool   fileExists(const char *fileName);
bool   directoryExists(const char *pathname);
time_t getFileModificationTime(const char *fileName);
long   getFileSize(const char *fileName);
bool   mkdirs(const char *pathname);
bool   copyFile(const char *src, const char *dest);
char   *getRelativePath(const char *targetFile, const char *relativeTo);
bool   copyDirectory(const char *srcDir, const char *destDir, ReplaceOption option, 
          bool copyBackupFiles, EnterDirCallback onEnterDir, LeaveDirCallback onLeaveDir,
          CopyFileCallback onCopyFile, void *userData);

#endif /* PATH_H */
