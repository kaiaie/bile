/* :tabSize=4:indentSize=4:folding=indent:
** $Id: path.c,v 1.18 2010/08/31 15:11:58 ken Exp $
*/
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "path.h"
#include "astring.h"
#include "bool.h"
#include "Buffer.h"
#include "List.h"
#include "Logging.h"
#include "memutils.h"
#include "stringext.h"

#define minOf(a, b) ((a < b) ? a : b)

/** Returns True if path begins with a drive letter, False otherwise */
bool isDosPath(const char *path) {
	if (path != NULL && strlen(path) > 1 && path[1] == ':') {
		return true;
	}
	else {
		return false;
	}
}


/** Returns True if path is of the form //server/share, False otherwise */
bool isUncPath(const char *path) {
	char *tmp   = NULL;
	bool result = false;
	
	if (path != NULL && strlen(path) > 2) {
		tmp = strxreplace(astrcpy(path), '\\', '/');
		result = ((tmp[0] == '/') && (tmp[1] == '/') ? true : false);
		mu_free(tmp);
	}
	return result;
}


/** Returns a part of a directory path */
char *getPathPart(const char *path, PathPart part) {
	char   *result = NULL;
	char   *tmp1   = NULL;
	char   *tmp2   = NULL;
	size_t pos     = 0;
	
	if (path != NULL && strlen(path) > 0) {
		tmp1 = strxreplace(astrcpy(path), '\\', '/');
		switch(part){
			case PATH_HOST:
				if (isUncPath(tmp1)) {
					tmp2 = &tmp1[2];
					if (strxpos(tmp2, '/', &pos)) {
						result = astrleft(tmp2, pos);
					}
					else {
						result = astrcpy(tmp2);
					}
				}
				else {
					result = astrcpy("");
				}
				break;
			case PATH_DRIVE:
				if (isDosPath(tmp1)) {
					result = astrleft(tmp1, 2);
				}
				else {
					result = astrcpy("");
				}
				break;
			case PATH_DIR:
				if (isUncPath(tmp1) || isDosPath(tmp1)) {
					tmp2 = strchr(&tmp1[2], '/');
				}
				else {
					tmp2 = tmp1;
				}
				if (tmp2 != NULL) {
					if (strxrpos(tmp2, '/', &pos)) {
						if (pos == 0) {
							result = astrcpy(tmp2);
						}
						else {
							result = astrleft(tmp2, pos);
						}
					}
					else {
						result = astrcpy("");
					}
				}
				else {
					result = astrcpy("");
				}
				break;
			case PATH_FILE:
			case PATH_FILEONLY:
				if ((tmp2 = strrchr(tmp1, '/')) != NULL) {
					result = &tmp2[1];
				}
				else {
					result = tmp1;
				}
				if (part == PATH_FILE) {
					result = astrcpy(result);
				}
				else if (part == PATH_FILEONLY) {
					/* Strip off file extension */
					if (strxrpos(result, '.', &pos)) {
						if(pos == 0) {
							result = astrcpy(result);
						}
						else {
							result = astrleft(result, pos);
						}
					}
					else {
						result = astrcpy(result);
					}
				}
				break;
			case PATH_EXT:
				if ((tmp2 = strrchr(tmp1, '.')) != NULL) {
					if (strchr(tmp2, '/') == NULL)  {
						result = astrcpy(&tmp2[1]);
					}
					else {
						result = astrcpy("");
					}
				}
				else {
					result = astrcpy("");
				}
				break;
			default:
				result = astrcpy("");
				break;
		}
		mu_free(tmp1);
	}
	return result;
}


/** Removes "." and ".." from a path */
char *getCanonicalPath(const char *path){
	char   *prefix = NULL;
	char   *rest   = NULL;
	char   *tmp    = NULL;
	size_t offset  = 0;
	char   **src   = NULL;
	List   *dst    = NULL;
	size_t ii = 0;
	Buffer *buf    = NULL;
	char   *result = NULL;
	
	if (path != NULL && strlen(path) > 0) {
		tmp = strxreplace(astrcpy(path), '\\', '/');
		if (isDosPath(tmp) || isUncPath(tmp)) {
			if (isDosPath(tmp)) {
				prefix = getPathPart(tmp, PATH_DRIVE);
				offset = 0;
			}
			else if (isUncPath(tmp)) {
				prefix = getPathPart(tmp, PATH_HOST);
				offset = 2;
			}
			rest = astrcpy(strchr(&tmp[offset], '/'));
		}
		else {
			rest = astrcpy(tmp);
		}
		src = astrtok(rest, "/");
		dst = new_List();
		while (src[ii] != NULL) {
			if (strxequals(src[ii], "..")) {
				List_remove(dst, -1, false);
			}
			else if (!strxequals(src[ii], ".")) {
				List_append(dst, src[ii]);
			}
			ii++;
		}
		buf = new_Buffer(0);
		if (prefix != NULL) {
			Buffer_appendString(buf, prefix);
		}
		for (ii = 0; ii < List_length(dst); ++ii) {
			Buffer_appendString(buf, List_get(dst, ii));
			if (ii != (List_length(dst) - 1)) {
				Buffer_appendChar(buf, '/');
			}
		}
		result = astrcpy(buf->data);
		delete_Buffer(buf);
		delete_List(dst, false);
		astrtokfree(src);
		mu_free(prefix);
		mu_free(rest);
		mu_free(tmp);
	}
	return result;
}


/** Joins two paths together, allowing for relative/absolute paths */
char *getCombinedPath(const char *path1, const char *path2) {
	char   *tmp1   = NULL;
	char   *tmp2   = NULL;
	char   *tmp    = NULL;
	Buffer *buf    = NULL;
	char   *result = NULL;
	char   *final  = NULL;
	
	if (!strxnullorempty(path1) && !strxnullorempty(path2)) {
		tmp1 = strxreplace(astrcpy(path1), '\\', '/');
		tmp2 = strxreplace(astrcpy(path2), '\\', '/');
		if (isDosPath(tmp2) || isUncPath(tmp2)) {
			/* Path2 is a fully-qualified DOS/Windows path; return it */
			result = astrcpy(tmp2);
		}
		else {
			buf = new_Buffer(0);
			if (tmp2[0] == '/') {
				/* Path2 is an absolute path.  If Path1 is a DOS/Windows or 
				** UNC path, prepend the drive letter or hostname; otherwise 
				** return it.
				*/
				if (isDosPath(tmp1)) {
					tmp = getPathPart(tmp1, PATH_DRIVE);
					Buffer_appendString(buf, tmp);
					Buffer_appendString(buf, tmp2);
					mu_free(tmp);
				}
				else if (isUncPath(tmp1)) {
					tmp = getPathPart(tmp1, PATH_HOST);
					Buffer_appendString(buf, "//");
					Buffer_appendString(buf, tmp);
					Buffer_appendString(buf, tmp2);
					mu_free(tmp);
				}
				else {
					Buffer_appendString(buf, tmp2);
				}
			}
			else {
				/* Simply concatenate the two paths */
				Buffer_appendString(buf, tmp1);
				if (tmp1[strlen(tmp1) - 1] != '/') {
					Buffer_appendChar(buf, '/');
				}
				Buffer_appendString(buf, tmp2);
			}
			result = astrcpy(buf->data);
			delete_Buffer(buf);
		}
		mu_free(tmp1);
		mu_free(tmp2);
	}
	/* Remove any "." and ".." in the path */
	final = getCanonicalPath(result);
	mu_free(result);
	return final;
}


/** Returns the current directory; the returned string must be freed by the 
*** caller
**/
char *getCurrentDirectory(void) {
	char   *buffer = NULL;
	size_t bufferLength = 64;
	
	buffer = (char *)mu_malloc(bufferLength);
	while(getcwd(buffer, bufferLength) == NULL){
		bufferLength *= 2;
		buffer = (char *)mu_realloc(buffer, bufferLength);
	}
	return buffer;
}


/** Concatenates two paths, adding a separator if necessary.
*** It is the responsibility of the caller to dispose of the returned string.
**/
char *buildPath(const char *path1, const char *path2) {
	if(path1[strlen(path1) - 1] == '/') {
		return astrcat(path1, path2);
	}
	else {
		return asprintf("%s/%s", path1, path2);
	}
}


/** Returns True if the specified file exists, False otherwise */
bool fileExists(const char *fileName) {
	return (access(fileName, F_OK) == 0);
}


/** Returns True if the specified file exists and is a directory, False 
*** otherwise
**/
bool directoryExists(const char *pathname) {
	struct stat st;
	bool result = fileExists(pathname);
	if (result) {
		if (stat(pathname, &st) == 0) {
			result = S_ISDIR(st.st_mode);
		}
		else {
			result = false;
		}
	}
	return result;
}


/** Returns the last modification date of the specified file */
time_t getFileModificationTime(const char *fileName) {
	struct stat st;
	if (fileExists(fileName)) {
		if (stat(fileName, &st) == 0) {
			return st.st_mtime;
		}
	}
	return 0;
}


/** Returns the size of the file in bytes */
long getFileSize(const char *fileName) {
	struct stat st;
	if (fileExists(fileName)) {
		if (stat(fileName, &st) == 0) {
			return st.st_size;
		}
	}
	return -1;
}


/** Creates a directory including any missing parent directories */
bool mkdirs(const char *pathname) {
	bool   result = false;
	size_t ii = 0;
	char   *tmp = NULL;
	
	if (pu_mkdir(pathname) == -1) {
		/* Did mkdir() fail because a parent directory doesn't exist? */
		if (errno == ENOENT) {
			/* Try making the parent */
			if (pathname[strlen(pathname) - 1] == '/') {
				ii = strlen(pathname) - 2;
			}
			else {
				ii = strlen(pathname) - 1;
			}
			do {
				if (pathname[ii--] == '/') break;
			} while (ii >= 0);
			tmp = astrleft(pathname, ii);
			result = mkdirs(tmp);
			if (result) {
				/* Try making the original again */
				result = (pu_mkdir(pathname) == 0);
			}
			mu_free(tmp);
			return result;
		}
		else {
			/* Some other error */
			Logging_debugf("%s(): Cannot create directory \"%s\": %s",
				__FUNCTION__, 
				pathname, 
				strerror(errno)
			);
			return false;
		}
	}
	else return true;
}


/** Copies the specified file to the specified destination */
bool copyFile(const char *src, const char *dest){
	/* Adapted from snippets */
	bool result = false;
	register char *buffer;
	register int bytesRead;
	int fdsrc, fddest;
	buffer = (char *)mu_malloc(0x10000);
	if ((fdsrc = open(src, O_RDONLY | O_BINARY, 0)) >= 0) {
		if ((fddest = open(dest, O_BINARY | O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE)) >= 0) {
			while (true) {
				bytesRead = read(fdsrc, buffer, 0x10000);
				if (bytesRead == 0) {
					result = true;
					break;
				}
				if(bytesRead == -1 || bytesRead != write(fddest, buffer, (unsigned)bytesRead))
					break;
			}
			close(fddest);
			if (!result) unlink(dest); /* Remove partial file */
		}
		close(fdsrc);
	}
	mu_free(buffer);
	return result;
}


/** Returns the path to targetFile relative to the path 
*** specified in relativeTo.  The returned string must be free()'d by the 
*** caller.
*** \note Needs to be more thoroughly tested, esp. with DOS paths; may 
*** also merge with getCombinedPath
**/
char *getRelativePath(const char *targetFile, const char *relativeTo) {
	char *tmpTarget = NULL;
	char *tmpRel    = NULL;
	char *result    = NULL;
	char **targetPath = NULL;
	char **relativePath = NULL;
	Buffer *buffer = NULL;
	size_t joinPoint = 0;
	size_t levels = 0;
	size_t ii;
	
	
	tmpTarget = strxreplace(astrcpy(targetFile), '\\', '/');
	tmpRel    = strxreplace(astrcpy(relativeTo), '\\', '/');
	
	/* Special case: empty relative path */
	if (strxempty(tmpRel)) {
		result = astrcpy(targetFile);
		goto done;
	}

	/* Special case: file in same directory as relative directory */
	for (ii = strlen(tmpTarget) - 1; ii >= 0; --ii) {
		if (tmpTarget[ii] == '/') {
			if ((strlen(tmpRel) == (ii + 1)) &&
				(strncmp(tmpRel, tmpTarget, (ii + 1)) == 0)
			) {
				result = getPathPart(tmpTarget, PATH_FILE);
				goto done;
			}
			break;
		}
	}
	
	targetPath = astrtok(tmpTarget, "/");
	relativePath = astrtok(tmpRel, "/");
	/* If the root directory, remove the trailing blank token */
	if (alength(relativePath) == 2 && strxempty(relativePath[1])) {
		mu_free(relativePath[1]);
		relativePath[1] = NULL;
	}
	buffer = new_Buffer(0);
	if (alength(targetPath) == 1) {
		/* Relative path to top of the directory */
		levels = alength(relativePath);
		joinPoint = 0;
	}
	else {
		/* Move down the paths until we find the point of divergence */
		while(joinPoint < minOf(alength(targetPath), alength(relativePath))){
			if(!strxequals(targetPath[joinPoint], relativePath[joinPoint])) break;
			joinPoint++;
		}
		/* Work out how many levels up from this point we have to come */
		levels = alength(relativePath) - joinPoint;
	}
	/* Walk up the relative path to the join point... */
	for (ii = 0; ii < levels; ++ii){
		if(ii != 0) Buffer_appendChar(buffer, '/');
		Buffer_appendString(buffer, "..");
	}
	/* ... and back down the target path */
	while (targetPath[joinPoint] != NULL){
		if(!strxempty(buffer->data)) Buffer_appendChar(buffer, '/');
		Buffer_appendString(buffer, targetPath[joinPoint]);
		joinPoint++;
	}
	result = astrcpy(buffer->data);
	delete_Buffer(buffer);
	astrtokfree(relativePath);
	astrtokfree(targetPath);
	
done:
	mu_free(tmpTarget);
	mu_free(tmpRel);
	return result;
}


/** Recursively copies the contents of the source directory to the destination 
*** directory
*** \param srcDir  The directory to copy
*** \param destDir The destination directory
*** \param option  Determines under what circumstances a file will be 
*** overwritten if it exists in the destination
*** \param copyBackupFiles If True, backup files and version-control 
*** directories will be copies; if False, these will be skipped
*** \param onEnterDir A callback function that is to be called when a directory 
*** if first entered; this parameter may be left NULL
*** \param onLeaveDir A callback function that is to be called when the entire 
*** contents of a directory have been copied; this parameter may be left NULL
*** \param onCopyFile A callback function that is to be called each time a file 
*** is copied from the source to the destination; this parameter may bee left 
*** NULL
*** \param userData A user-defined value that is to be passed to the above 
*** callback functions when they are called.
*** \return True if copy was successful, False otherwise
**/
bool copyDirectory(
	const char *srcDir, 
	const char *destDir, 
	ReplaceOption option, 
	bool copyBackupFiles, 
	EnterDirCallback onEnterDir, 
	LeaveDirCallback onLeaveDir, 
	CopyFileCallback onCopyFile, 
	void *userData
) {
	char *srcPath = NULL;
	char *destPath = NULL;
	DIR *d = NULL;
	struct dirent *e = NULL;
	struct stat st;
	bool shouldCopy = true;
	bool result = false;
	
	if ((d = opendir(srcDir)) != NULL) {
		while ((e = readdir(d)) != NULL) {
			if (!strxequals(e->d_name, ".") && 
				!strxequals(e->d_name, "..") &&
				!(!copyBackupFiles && strxequals(e->d_name, "CVS")) &&
				!(!copyBackupFiles && strxequals(e->d_name, ".svn"))
			) {
				srcPath  = buildPath(srcDir, e->d_name);
				destPath = buildPath(destDir, e->d_name);
				if (stat(srcPath, &st) == 0){
					/* If directory entry is a directory, recurse into it */
					if (S_ISDIR(st.st_mode)){
						if (!directoryExists(destPath)) pu_mkdir(destPath);
						/* Execute callback function to notify directory change */
						if (onEnterDir != NULL) onEnterDir(e->d_name, userData);
						if (!(result = copyDirectory(srcPath, destPath, option, 
							copyBackupFiles, onEnterDir, onLeaveDir, onCopyFile, 
							userData))
						) {
							break;
						}
						/* Execute callback function to notify return */
						if (onLeaveDir != NULL) onLeaveDir(userData);
					}
					/* If directory entry is a file, copy it if replacement 
					** options allow
					*/
					else {
						if (option == REPLACE_NEVER)
							shouldCopy = !fileExists(destPath);
						else if (option == REPLACE_OLDER) {
							shouldCopy = !fileExists(destPath) || 
								(getFileModificationTime(srcPath) > getFileModificationTime(destPath));
						}
						else if (option == REPLACE_ALWAYS)
							shouldCopy = true;
						
						/* Check for backup files */
						if (!copyBackupFiles) {
							if (strxends(srcPath, "~") || strxends(srcPath, "#")) {
								shouldCopy = false;
							}
						}
						
						if (shouldCopy) {
							if (!(result = copyFile(srcPath, destPath))) break;
							/* Execute callback function to notify file copy */
							if (onCopyFile != NULL) onCopyFile(e->d_name, userData);
						}
					}
				}
				else {
					Logging_warnf("Error trying to stat() file \"%s\": %s",
						srcPath, strerror(errno)
					);
					break;
				}
				mu_free(destPath); destPath = NULL;
				mu_free(srcPath);  srcPath = NULL;
			}
		}
		if (destPath != NULL) mu_free(destPath);
		if (srcPath != NULL) mu_free(srcPath);
		closedir(d);
	}
	return result;
}

