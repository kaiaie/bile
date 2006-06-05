/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: path.c,v 1.11 2006/06/05 16:47:03 ken Exp $
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

#ifdef _WIN32
#define pu_mkdir(a) mkdir(a)
#else
#define pu_mkdir(a) mkdir(a, 0777)
#endif

bool isDosPath(const char *path){
	if(path != NULL && strlen(path) > 1 && path[1] == ':'){
		return true;
	}
	else{
		return false;
	}
}


bool isUncPath(const char *path){
	char *tmp   = NULL;
	bool result = false;
	
	if(path != NULL && strlen(path) > 2){
		tmp = strreplace(astrcpy(path), '\\', '/');
		result = ((tmp[0] == '/') && (tmp[1] == '/') ? true : false);
		mu_free(tmp);
	}
	return result;
}


char *getPathPart(const char *path, PathPart part){
	char   *result = NULL;
	char   *tmp1   = NULL;
	char   *tmp2   = NULL;
	size_t pos     = 0;
	
	if(path != NULL && strlen(path) > 0){
		tmp1 = strreplace(astrcpy(path), '\\', '/');
		switch(part){
			case PATH_HOST:
				if(isUncPath(tmp1)){
					tmp2 = &tmp1[2];
					if(strpos(tmp2, '/', &pos)){
						result = astrleft(tmp2, pos);
					}
					else{
						result = astrcpy(tmp2);
					}
				}
				else{
					result = astrcpy("");
				}
				break;
			case PATH_DRIVE:
				if(isDosPath(tmp1)){
					result = astrleft(tmp1, 2);
				}
				else{
					result = astrcpy("");
				}
				break;
			case PATH_DIR:
				if(isUncPath(tmp1) || isDosPath(tmp1)){
					tmp2 = strchr(&tmp1[2], '/');
				}
				else{
					tmp2 = tmp1;
				}
				if(tmp2 != NULL){
					if(strrpos(tmp2, '/', &pos)){
						if(pos == 0){
							result = astrcpy(tmp2);
						}
						else{
							result = astrleft(tmp2, pos);
						}
					}
					else{
						result = astrcpy("");
					}
				}
				else{
					result = astrcpy("");
				}
				break;
			case PATH_FILE:
			case PATH_FILEONLY:
				if((tmp2 = strrchr(tmp1, '/')) != NULL){
					result = &tmp2[1];
				}
				else{
					result = tmp1;
				}
				if(part == PATH_FILE){
					result = astrcpy(result);
				}
				else if(part == PATH_FILEONLY){
					/* Strip off file extension */
					if(strrpos(result, '.', &pos)){
						if(pos == 0){
							result = astrcpy(result);
						}
						else{
							result = astrleft(result, pos);
						}
					}
					else{
						result = astrcpy(result);
					}
				}
				break;
			case PATH_EXT:
				if((tmp2 = strrchr(tmp1, '.')) != NULL){
					if(strchr(tmp2, '/') == NULL){
						result = astrcpy(&tmp2[1]);
					}
					else{
						result = astrcpy("");
					}
				}
				else{
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
	
	if(path != NULL && strlen(path) > 0){
		tmp = strreplace(astrcpy(path), '\\', '/');
		if(isDosPath(tmp) || isUncPath(tmp)){
			if(isDosPath(tmp)){
				prefix = getPathPart(tmp, PATH_DRIVE);
				offset = 0;
			}
			else if(isUncPath(tmp)){
				prefix = getPathPart(tmp, PATH_HOST);
				offset = 2;
			}
			rest = astrcpy(strchr(&tmp[offset], '/'));
		}
		else{
			rest = astrcpy(tmp);
		}
		src = astrtok(rest, "/");
		dst = new_List();
		while(src[ii] != NULL){
			if(strequals(src[ii], "..")){
				List_remove(dst, -1, false);
			}
			else if(!strequals(src[ii], ".")){
				List_append(dst, src[ii]);
			}
			ii++;
		}
		buf = new_Buffer(0);
		if(prefix != NULL){
			Buffer_appendString(buf, prefix);
		}
		for(ii = 0; ii < List_length(dst); ++ii){
			Buffer_appendString(buf, List_get(dst, ii));
			if(ii != (List_length(dst) - 1)){
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


char *getCombinedPath(const char *path1, const char *path2){
	char   *tmp1   = NULL;
	char   *tmp2   = NULL;
	char   *tmp    = NULL;
	Buffer *buf    = NULL;
	char   *result = NULL;
	char   *final  = NULL;
	
	if(path1 != NULL && strlen(path1) > 0 && path2 != NULL && strlen(path2) > 0){
		tmp1 = strreplace(astrcpy(path1), '\\', '/');
		tmp2 = strreplace(astrcpy(path2), '\\', '/');
		if(isDosPath(tmp2) || isUncPath(tmp2)){
			/* Path2 is a fully-qualified DOS/Windows path; return it */
			result = astrcpy(tmp2);
		}
		else{
			buf = new_Buffer(0);
			if(tmp2[0] == '/'){
				/* Path2 is an absolute path.  If Path1 is a DOS/Windows or 
				 * UNC path, prepend the drive letter or hostname; otherwise 
				 * return it.
				 */
				if(isDosPath(tmp1)){
					tmp = getPathPart(tmp1, PATH_DRIVE);
					Buffer_appendString(buf, tmp);
					Buffer_appendString(buf, tmp2);
					mu_free(tmp);
				}
				else if(isUncPath(tmp1)){
					tmp = getPathPart(tmp1, PATH_HOST);
					Buffer_appendString(buf, "//");
					Buffer_appendString(buf, tmp);
					Buffer_appendString(buf, tmp2);
					mu_free(tmp);
				}
				else{
					Buffer_appendString(buf, tmp2);
				}
			}
			else{
				/* Simply concatenate the two paths */
				Buffer_appendString(buf, tmp1);
				if(tmp1[strlen(tmp1 - 1)] != '/'){
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


char *getCurrentDirectory(void){
	char   *buffer = NULL;
	size_t bufferLength = 64;
	
	buffer = (char *)mu_malloc(bufferLength);
	while(getcwd(buffer, bufferLength) == NULL){
		bufferLength *= 2;
		buffer = (char *)mu_realloc(buffer, bufferLength);
	}
	return buffer;
}


/*
 * buildPath - concatenate two paths, adding a separator if necessary.
 * It is the responsibility of the caller to dispose of the returned string.
 */
char *buildPath(const char *path1, const char *path2){
	if(path1[strlen(path1) - 1] == '/')
		return astrcat(path1, path2);
	else
		return asprintf("%s/%s", path1, path2);
}


bool fileExists(const char *fileName){
	return (access(fileName, F_OK) == 0);
}


bool directoryExists(const char *pathname){
	struct stat st;
	bool result = fileExists(pathname);
	if(result){
		if(stat(pathname, &st) == 0)
			result = S_ISDIR(st.st_mode);
		else
			result = false;
	}
	return result;
}


time_t getFileModificationTime(const char *fileName){
	struct stat st;
	if(fileExists(fileName)){
		if(stat(fileName, &st) == 0)
			return st.st_mtime;
	}
	return 0;
}


long getFileSize(const char *fileName){
	struct stat st;
	if(fileExists(fileName)){
		if(stat(fileName, &st) == 0)
			return st.st_size;
	}
	return -1;
}


/*
 * mkdirs - make a directory, creating any missing parent directories
 */
bool mkdirs(const char *pathname){
	bool   result = false;
	size_t ii = 0;
	char   *tmp = NULL;
	
	if(pu_mkdir(pathname) == -1){
		/* Did mkdir() fail because a parent directory doesn't exist? */
		if(errno == ENOENT){
			/* Try making the parent */
			if(pathname[strlen(pathname) - 1] == '/')
				ii = strlen(pathname) - 2;
			else
				ii = strlen(pathname) - 1;
			do{
				if(pathname[ii--] == '/') break;
			} while(ii >= 0);
			tmp = astrleft(pathname, ii);
			result = mkdirs(tmp);
			if(result){
				/* Try making the original again */
				result = (pu_mkdir(pathname) == 0);
			}
			mu_free(tmp);
			return result;
		}
		else{
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


bool copyFile(const char *src, const char *dest){
	/* Adapted from snippets */
	bool result = false;
	register char *buffer;
	register int bytesRead;
	int fdsrc, fddest;
	buffer = (char *)mu_malloc(0x10000);
	if((fdsrc = open(src, O_RDONLY | O_BINARY, 0)) >= 0){
		if((fddest = open(dest, O_BINARY | O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE)) >= 0){
			while(true){
				bytesRead = read(fdsrc, buffer, 0x10000);
				if(bytesRead == 0){
					result = true;
					break;
				}
				if(bytesRead == -1 || bytesRead != write(fddest, buffer, (unsigned)bytesRead))
					break;
			}
			close(fddest);
			if(!result) unlink(dest); /* Remove partial file */
		}
		close(fdsrc);
	}
	mu_free(buffer);
	return result;
}


/* getRelativePath: returns the path to targetFile relative to the path 
 * specified in relativeTo.  The returned string must be free()'d by the 
 * caller.
 * NOTE: Needs to be more thoroughly tested, esp. with DOS paths; may 
 * also merge with getCombinedPath
 */
char *getRelativePath(const char *targetFile, const char *relativeTo){
	char *tmpTarget = NULL;
	char *tmpRel    = NULL;
	char *result    = NULL;
	char **targetPath = NULL;
	char **relativePath = NULL;
	Buffer *buffer = NULL;
	size_t joinPoint = 0;
	size_t levels = 0;
	size_t ii;
	
	tmpTarget = strreplace(astrcpy(targetFile), '\\', '/');
	tmpRel    = strreplace(astrcpy(relativeTo), '\\', '/');
	if(strempty(tmpRel))
		result = astrcpy(targetFile);
	else{
		targetPath = astrtok(tmpTarget, "/");
		relativePath = astrtok(tmpRel, "/");
		buffer = new_Buffer(0);
		if(alength(targetPath) == 1){
			/* Relative path to top of the directory */
			levels = alength(relativePath);
			joinPoint = 0;
		}
		else{
			/* Move down the paths until we find the point of divergence */
			while(joinPoint < minOf(alength(targetPath), alength(relativePath))){
				if(!strequals(targetPath[joinPoint], relativePath[joinPoint])) break;
				joinPoint++;
			}
			/* Work out how many levels up from this point we have to come */
			levels = alength(relativePath) - joinPoint;
		}
		/* Walk up the relative path to the join point... */
		for(ii = 0; ii < levels; ++ii){
			if(ii != 0) Buffer_appendChar(buffer, '/');
			Buffer_appendString(buffer, "..");
		}
		/* ... and back down the target path */
		while(targetPath[joinPoint] != NULL){
			if(!strempty(buffer->data)) Buffer_appendChar(buffer, '/');
			Buffer_appendString(buffer, targetPath[joinPoint]);
			joinPoint++;
		}
		result = astrcpy(buffer->data);
		delete_Buffer(buffer);
		astrtokfree(relativePath);
		astrtokfree(targetPath);
	}
	mu_free(tmpTarget);
	mu_free(tmpRel);
	return result;
}


/* copyDirectory: recursively copies the contents of the source directory to 
 * the destination directory
 */
bool copyDirectory(const char *srcDir, const char *destDir, ReplaceOption option){
	char *srcPath = NULL;
	char *destPath = NULL;
	DIR *d = NULL;
	struct dirent *e = NULL;
	struct stat st;
	bool shouldCopy = true;
	bool result = false;
	
	if((d = opendir(srcDir)) != NULL){
		while((e = readdir(d)) != NULL){
			if(!strequals(e->d_name, ".") && !strequals(e->d_name, "..")){
				srcPath  = buildPath(srcDir, e->d_name);
				destPath = buildPath(destDir, e->d_name);
				if(stat(srcPath, &st) == 0){
					if(S_ISDIR(st.st_mode)){
						if(!directoryExists(destPath)) mkdir(destPath);
						if(!(result = copyDirectory(srcPath, destPath, option)))
							break;
					}
					else{
						if(option == REPLACE_NEVER)
							shouldCopy = !fileExists(destPath);
						else if(option == REPLACE_OLDER){
							shouldCopy = !fileExists(destPath) || 
								(getFileModificationTime(srcPath) > getFileModificationTime(destPath));
						}
						else if(option == REPLACE_ALWAYS)
							shouldCopy = true;
						if(shouldCopy){
							if(!(result = copyFile(srcPath, destPath))) break;
						}
					}
				}
				else{
					Logging_warnf("Error trying to stat() file \"%s\": %s",
						srcPath, strerror(errno)
					);
					break;
				}
				mu_free(destPath); destPath = NULL;
				mu_free(srcPath);  srcPath = NULL;
			}
		}
		if(destPath != NULL) mu_free(destPath);
		if(srcPath != NULL) mu_free(srcPath);
		closedir(d);
	}
	return result;
}
