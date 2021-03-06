#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Publication.h"
#include "astring.h"
#include "memutils.h"
#include "path.h"
#include "stringext.h"
#include "ConfigFile.h"
#include "Dict.h"
#include "FileHandler.h"
#include "HtmlHandler.h"
#include "ImgHandler.h"
#include "Index.h"
#include "List.h"
#include "Logging.h"
#include "Pair.h"
#include "Section.h"
#include "Story.h"
#include "Tags.h"
#include "Type.h"
#include "Vars.h"

static int sectionId = 1;
static int storyId = 1;

void  addDir(Publication *p, Section *s, const char *path);
Index *findIndex(Section *s, const char *name);
void  generateStories(Publication *p, Section *s, const char *path);
void  generateIndexes(Publication *p, Section *s, const char *path);
void  generateTags(Publication *p);
bool  isIgnoredFile(const char *fileName);
bool  isSpecialFile(const char *fileName);
bool  isIncludeFile(const char *fileName);
void  Publication_createScript(Publication *p);
void  Publication_closeScript(Publication *p);
void  Publication_scriptPutFile(Publication *p, const char *fileName);
void  Publication_scriptChdir(Publication *p, const char *directory, bool changeLocal, bool createFirst);
void  Publication_scriptLchdir(Publication *p, const char *directory);
void  Publication_scriptMkdir(Publication *p, const char *directory);
void  Publication_scriptDeleteFile(Publication *p, const char *fileName);
void  onEnterDir(const char *directory, void *userData);
void  onLeaveDir(void *userData);
void  onCopyFile(const char *fileName, void *userData);

/** Creates a new Publication */
Publication *new_Publication(char *inputDirectory, char *outputDirectory, 
	char *templateDirectory, bool forceMode, bool verboseMode, char *scriptFile){
	Publication *p = NULL;
	
	p = (Publication *)mu_malloc(sizeof(Publication));
	p->type = BILE_PUB;
	/* Initialize the Publication root */
	p->root = new_Section(NULL, ".");
	Vars_let(p->root->variables, "input_directory",   inputDirectory, VAR_CONST);
	Vars_let(p->root->variables, "output_directory",  outputDirectory, VAR_CONST);
	Vars_let(p->root->variables, "template_directory",templateDirectory, VAR_CONST);
	p->inputDirectory    = astrcpy(inputDirectory);
	p->outputDirectory   = astrcpy(outputDirectory);
	p->templateDirectory = astrcpy(templateDirectory);
	p->templateCache     = new_Dict();
	p->forceMode         = forceMode;
	p->verboseMode       = verboseMode;
	p->scriptFileName    = scriptFile;
	p->scriptFile        = NULL;
	p->tagList           = new_List();
	return p;
}


/** Recursively adds the input directory tree to the Publication */
void Publication_build(Publication *p){
	addDir(p, p->root, (char *)NULL);
}


/** Renders the Publication to the output directory */
void Publication_generate(Publication *p){
	DIR *d = NULL;
	struct dirent *e = NULL;
	char *srcPath = NULL;
	char *destPath = NULL;
	ReplaceOption option = REPLACE_OLDER;
	
	/* Initialise script file */
	Publication_createScript(p);
	
	generateStories(p, p->root, (char *)NULL);
	generateIndexes(p, p->root, (char *)NULL);
	generateTags(p);
	
	/* Copy static content from the subdirectories in template directory to 
	** corresponding directories in the output directory
	*/
	if (p->forceMode) option = REPLACE_ALWAYS;
	if ((d = opendir(p->templateDirectory)) != NULL){
		while ((e = readdir(d)) != NULL){
			if (!strxequals(e->d_name, ".") && !strxequals(e->d_name, "..") && 
				!strxequals(e->d_name, "CVS")
			) {
				srcPath = buildPath(p->templateDirectory, e->d_name);
				if (directoryExists(srcPath)) {
					Publication_scriptChdir(p, e->d_name, true, true);
					destPath = buildPath(p->outputDirectory, e->d_name);
					if(!directoryExists(destPath)) pu_mkdir(destPath);
					copyDirectory(srcPath, destPath, option, false, onEnterDir, 
						onLeaveDir, onCopyFile, p
					);
					mu_free(destPath);
					Publication_scriptChdir(p, "..", true, false);
				}
				mu_free(srcPath);
			}
		}
		closedir(d);
	}
	
	/* Close script file */
	Publication_closeScript(p);
}


/** Adds the story to section and publication indexes */
void Publication_addToIndexes(Publication *p, Section *s, Story *st){
	Index *idx;
	size_t ii;
	
	/* Update publication indexes */
	for (ii = 0; ii < List_length(p->root->indexes); ++ii){
		idx = (Index *)List_get(p->root->indexes, ii);
		Index_add(idx, st);
	}
	/* Update section indexes */
	if (s != p->root){
		for(ii = 0; ii < List_length(s->indexes); ++ii){
			idx = (Index *)List_get(s->indexes, ii);
			Index_add(idx, st);
		}
	}
}


/** Adds the Story to the Publication's tag list */
bool Publication_addToTags(Publication *p, Story *st) {
	Tags *t;
	size_t ii;
	for (ii = 0; ii < List_length(p->tagList); ++ii) {
		t = (Tags *)List_get(p->tagList, ii);
		Tags_add(t, st);
	}
	return true;
}


/** Returns the compiled template for the specified template file
 *  If the template has not already been compiled, it is loaded and compiled
 */
Template *Publication_getTemplate(Publication *p, char *fileName){
	Template *result       = NULL;
	char     *templatePath = NULL;
	
	if (!Dict_exists(p->templateCache, fileName)){
		templatePath = buildPath(p->templateDirectory, fileName);
		result = Template_compile(templatePath);
		Dict_put(p->templateCache, fileName, result);
		mu_free(templatePath);
	}
	else
		result = (Template *)Dict_get(p->templateCache, fileName);
	return result;
}


/** Returns the index with the specified name */
Index *Publication_findIndex(Publication *p, const char *name){
	return findIndex(p->root, name);
}


/** Returns the named tag list for the Publication */
Tags *Publication_findTags(Publication *p, const char *name){
	Tags *result = NULL;
	size_t ii;
	
	for (ii = 0; ii < List_length(p->tagList); ++ii){
		if (strxequals(name, ((Tags *)List_get(p->tagList, ii))->name)){
			result = List_get(p->tagList, ii);
			break;
		}
	}
	return result;
}



/** Debug function */
void Publication_dump(Publication *p){
	if (p != NULL){
		Logging_warnf("%s: Not implemented yet", __FUNCTION__);
	}
	else
		Logging_warnf("%s: NULL argument", __FUNCTION__);
}


/** Reads a directory and add its contents to the publication */
void addDir(Publication *p, Section *s, const char *path) {
	const char pubConfigFileName[] = "publication.bile";
	const char sectionConfigFileName[] = "section.bile";
	const char *configFileName = NULL;
	DIR *d = NULL;
	struct dirent *e = NULL;
	struct stat st;
	char    *tmp            = NULL;
	char    *fullPath       = NULL;
	char    *newPath        = NULL;
	char    *configFilePath = NULL;
	char    *inputFilePath  = NULL;
	char    *thePath        = NULL;
	char    *theName        = NULL;
	char    *theExt         = NULL;
	char    *outputFileName = NULL;
	Section *newSection     = NULL;
	Story   *newStory       = NULL;
	Index   *defaultIndex   = NULL;
	bool    processIncFiles = false;
	
	/* Check if at top level */
	if (s == p->root) {
		Logging_debug("Loading toplevel directory");
		configFileName = pubConfigFileName;
		fullPath = astrcpy(p->inputDirectory);
		/* Add a few useful variables and constants to the publication */
		Vars_let(s->variables, "pi", "3.141592653589793", VAR_CONST | VAR_NOSHADOW);
		Vars_let(s->variables, "tag_separator", ", ", VAR_CONST);
		Vars_let(s->variables, "tag_by", "keywords", VAR_CONST);
		Vars_let(s->variables, "error", "0", VAR_NOSHADOW);
		Vars_let(s->variables, "path", "", VAR_STD);
		
		/* By default, .inc files are not processed by templates as they 
		** normally contain only fragments of HTML; however, this can be 
		** overridden on a per-Section basis.
		*/
		Vars_let(s->variables, "process_inc_files", "false", VAR_STD);
		
		/* These are the default FTP commands used when generating an FTP script; 
		** they can be overridden in in the publication.bile file if the FTP 
		** client uses different ones.
		*/
		Vars_let(s->variables, "ftp_chdir", "cwd", VAR_STD);
		Vars_let(s->variables, "ftp_delete", "dele", VAR_STD);
		Vars_let(s->variables, "ftp_put", "put", VAR_STD);
		Vars_let(s->variables, "ftp_lcd", "lcd", VAR_STD);
		Vars_let(s->variables, "ftp_mkdir", "mkdir", VAR_STD);
	}
	else {
		Logging_debugf("Loading directory %s", path);
		configFileName = sectionConfigFileName;
		fullPath = buildPath(p->inputDirectory, path);
		Vars_let(s->variables, "path", path, VAR_STD);
		Vars_let(s->variables, "use_template", "false", VAR_STD);
		Vars_let(s->variables, "process_inc_files", "false", VAR_STD);
		/* TODO: Figure out what other section variables shouldn't be inherited 
		** and default them
		*/
	}
	
	tmp = asprintf("%d", sectionId++);
	Vars_let(s->variables, "section_id", tmp, VAR_STD);
	mu_free(tmp);
	configFilePath = buildPath(fullPath, configFileName);
	/* Read the config file if it exists */
	if (access(configFilePath, F_OK | R_OK) == 0) {
		readConfigFile(p, s, configFilePath);
	}
	else {
		/* publication.bile file is mandatory, section.bile files are not */
		if (s == p->root) {
			Logging_fatal("No publication configuration file found!");
		}
		else {
			Logging_warnf("Can't find configuration file %s: %s", configFilePath, strerror(errno));
			/* Create defaults */
			Vars_let(s->variables, "section_title", path, VAR_STD);
			/* Create a default index */
			defaultIndex = new_Index(s, path);
			Vars_let(defaultIndex->variables, "sort_by", "+file_name", VAR_STD);
			List_append(s->indexes, defaultIndex);
		}
	}
	mu_free(configFilePath);
	
	processIncFiles = Type_toBool(Vars_get(s->variables, "process_inc_files"));
	/* Read files */
	if ((d = opendir(fullPath)) == NULL) {
		Logging_fatalf("Error opening directory %s: %s", path, strerror(errno));
	}
	while ((e = readdir(d)) != NULL) {
		/* Skip unnecessary files*/
		if (isIgnoredFile(e->d_name)) continue;
		if (s == p->root) {
			newPath = astrcpy(e->d_name);
		}
		else {
			newPath = buildPath(path, e->d_name);
		}
		inputFilePath = buildPath(p->inputDirectory, newPath);
		if(stat(inputFilePath, &st) != 0) {
			Logging_warnf("Error stat()'ing file %s: %s",
				inputFilePath, strerror(errno)
			);
			mu_free(inputFilePath);
			continue;
		}
		/* Is the current directory entry a subdirectory? */
		if (S_ISDIR(st.st_mode)) {
			/* Create new Section and recurse */
			newSection = new_Section(s, e->d_name);
			List_append(s->sections, newSection);
			addDir(p, newSection, newPath);
		}
		else {
			/* Story file */
			Logging_debugf("Reading metadata from file %s", inputFilePath);
			newStory = new_Story(s);
			tmp = asprintf("%d", storyId++);
			Vars_let(newStory->variables, "story_id", tmp, VAR_STD);
			mu_free(tmp);
			Vars_let(newStory->variables, "path", newPath, VAR_STD);
			
			/* Read metadata from file */
			/* TODO: Have a proper list of file handlers rather than hardcoding */
			if (htmlCanHandle(inputFilePath)) {
				htmlReadMetadata(inputFilePath, newStory->variables);
			}
			else if (imgCanHandle(inputFilePath)) {
				imgReadMetadata(inputFilePath, newStory->variables);
			}
			defaultReadMetadata(inputFilePath, newStory->variables);
			
			/* Special files such as configuration files should not be indexed
			 * or run through the template processor. Include files can be 
			 * optionally processed.
			 */
			if (isSpecialFile(e->d_name) || 
				(isIncludeFile(e->d_name) && !processIncFiles)
			) {
				Vars_let(newStory->variables, "noindex", "true", VAR_STD);
				Vars_let(newStory->variables, "use_template", "false", VAR_STD);
			}
			/* If the "use_template_ext" variable is set, update the "path" 
			** variable so that the outputted story file will have the extension 
			** of the template file rather than its own extension.
			*/
			if (Vars_defined(newStory->variables,"use_template") &&
				Type_toBool(Vars_get(newStory->variables,"use_template")) &&
				Vars_defined(newStory->variables,"use_template_ext") &&
				Type_toBool(Vars_get(newStory->variables,"use_template_ext")))
			{
				thePath = getPathPart(Vars_get(newStory->variables, "path"), PATH_DIR);
				theName = getPathPart(Vars_get(newStory->variables, "path"), PATH_FILEONLY);
				theExt  = getPathPart(Vars_get(newStory->variables, "template_file"), PATH_EXT);
				outputFileName = asprintf("%s.%s", theName, theExt);
				tmp = buildPath(thePath, outputFileName);
				Vars_let(newStory->variables, "path", tmp, VAR_STD);
				mu_free(tmp);
				mu_free(outputFileName);
				mu_free(theExt);
				mu_free(theName);
				mu_free(thePath);
			}
			List_append(s->stories, newStory);
			Logging_debug("Story variables:");
			Vars_dump(newStory->variables);
			Publication_addToIndexes(p, s, newStory);
			Publication_addToTags(p, newStory);
		}
		mu_free(newPath);
		mu_free(inputFilePath);		
	}
	closedir(d);
	mu_free(fullPath);
}


/** Returns True if a file is to be ignored, False otherwise */
bool isIgnoredFile(const char *fileName) {
	/* Ignore . and .. */
	if (strxequals(fileName, ".") || strxequals(fileName, "..")) {
		return true;
	}
	/* Ignore administrative directories used by version control systems */
	else if (strxequals(fileName, "CVS") || strxequals(fileName, ".svn")) {
		return true;
	}
	/* Ignore special files used by Windows Explorer */
	else if (strxequalsi(fileName, "thumbs.db") || strxequalsi(fileName, "desktop.ini")) {
		return true;
	}
	/* Ignore Bile configuration files */
	else if (strxends(fileName, ".bile")) {
		return true;
	}
	/* Ignore backup files created by various editors */
	else if (strxends(fileName, "~") || strxends(fileName, "#")) {
		return true;
	}
	/* Is there anything left?! */
	else {
		return false;
	}
}


/** Returns True if file should not be passed through a template */
bool isSpecialFile(const char *fileName) {
	/* The following special files should never be indexed or passed 
	** through a template:
	** .	favicon.ico
	** .	robots.txt
	** .	files beginning with "." (e.g. .htaccess)
	*/
	if (strxequalsi(fileName, "robots.txt") || 
		strxequalsi(fileName, "favicon.ico") || 
		fileName[0] == '.'
	) {
		return true;
	}
	else {
		return false;
	}
}


/** Returns True if file extension is .inc */
bool isIncludeFile(const char *fileName) {
	if (strxends(fileName, ".inc") || 
		strxends(fileName, ".INC")
	) {
		return true;
	}
	else {
		return false;
	}
}


/** Generates all the Story files in a Section and its subsections */
void generateStories(Publication *p, Section *s, const char *path){
	Story    *currStory   = NULL;
	Section  *subSection  = NULL;
	size_t   ii;
	bool     isRoot                 = false;
	bool     usingTemplate          = false;
	bool     shouldOutputStory      = false;
	bool     shouldOutputTemplate   = false;
	char     *storyFile       = NULL;
	char     *inputPath       = NULL;
	char     *outputDirectory = NULL;
	char     *templateFile    = NULL;
	char     *templateExt     = NULL;
	Template *storyTemplate   = NULL;
	char     *storyOutputPath    = NULL;
	char     *sectionOutputPath  = NULL;
	char     *templateOutputPath = NULL;
	char     *fileNoExt = NULL;
	char     *fileWithExt = NULL;
	enum {OUTPUT_NORMAL, OUTPUT_NONE, OUTPUT_BOTH};
	int      outputMode = OUTPUT_NORMAL;
	char     *currDir = NULL;
	char     *currFile = NULL;
	
	isRoot = (s == p->root);
	/* Construct full output directory */
	if (isRoot) {
		outputDirectory = astrcpy(p->outputDirectory);
		Publication_scriptLchdir(p, outputDirectory);
		Publication_scriptChdir(p, Vars_get(p->root->variables, "ftp_root"), false, false);
	}
	else {
		outputDirectory = buildPath(p->outputDirectory, path);
		if ((p->scriptFile != NULL)) {
			currDir = abasename(path);
			Publication_scriptChdir(p, currDir, true, true);
			mu_free(currDir);
		}
	}
	/* Create directory if it doesn't exist */
	if (!directoryExists(outputDirectory)) mkdirs(outputDirectory);
	
	for (ii = 0; ii < List_length(s->stories); ++ii){
		/* Copy story file to output directory */
		currStory  = (Story *)List_get(s->stories, ii);
		storyFile  = Vars_get(currStory->variables, "file_name");
		
		/* Determine full paths to input and output files */
		if (isRoot) {
			inputPath  = buildPath(p->inputDirectory, storyFile);
		}
		else {
			inputPath = asprintf("%s/%s/%s", p->inputDirectory, path, storyFile);
		}
		currStory->inputPath = astrcpy(inputPath);
		storyOutputPath = buildPath(outputDirectory, storyFile);
		
		/* Determine output mode:
		** NORMAL = create a single file in the output directory from each 
		**          input file, optionally using a template.
		** NONE   = do not produce output; delete any existing copies of the 
		**          input file in the output directory.
		** BOTH   = create up to two files in the output directory for each 
		**          input file; one consisting of a copy of the input file, the 
		**          other using a template (if one is specified).  This is 
		**          useful when the input file is not HTML.  For example, this 
		**          can be used with image files to generate a gallery.
		*/
		if (Vars_defined(currStory->variables, "output_mode") && 
			strxequalsi(Vars_get(currStory->variables, "output_mode"), "none"))
		{
			outputMode = OUTPUT_NONE;
		}
		else if (Vars_defined(currStory->variables, "output_mode") && 
			strxequalsi(Vars_get(currStory->variables, "output_mode"), "both"))
		{
			outputMode = OUTPUT_BOTH;
		}
		else {
			outputMode = OUTPUT_NORMAL;
		}
		
		/* Get template, if using */
		if ((usingTemplate = Type_toBool(Vars_get(currStory->variables, "use_template")))){
			templateFile  = Vars_get(currStory->variables, "template_file");
			storyTemplate = Publication_getTemplate(p, templateFile);
			/* If using a template and the use_template_ext variable is set, 
			 * the file extension of the output file is that of the template 
			 * rather than that of the input file
			 */
			if(Vars_defined(currStory->variables, "use_template_ext") && 
				Type_toBool(Vars_get(currStory->variables, "use_template_ext")))
			{
				templateExt = getPathPart(templateFile, PATH_EXT);
				fileNoExt   = getPathPart(storyFile, PATH_FILEONLY);
				fileWithExt = asprintf("%s.%s", fileNoExt, templateExt);
				templateOutputPath = buildPath(outputDirectory, fileWithExt);
				mu_free(fileWithExt);
				mu_free(fileNoExt);
				mu_free(templateExt);
			}
			else {
				templateOutputPath = astrcpy(storyOutputPath);
			}
		}
		
		/* Determine whether output is to be generated */
		if (outputMode == OUTPUT_NONE){
			/* Generate no output and delete any existing output files */
			shouldOutputStory = false;
			shouldOutputTemplate = false;
			if (fileExists(storyOutputPath)) {
				unlink(storyOutputPath);
				Publication_scriptDeleteFile(p, storyFile);
			}
			if (usingTemplate && fileExists(templateOutputPath)) {
				unlink(templateOutputPath);
				if (p->scriptFile != NULL) {
					currFile = abasename(templateOutputPath);
					Publication_scriptDeleteFile(p, currFile);
					mu_free(currFile);
				}
			}
		}
		else if (outputMode == OUTPUT_NORMAL) {
			shouldOutputStory = !usingTemplate;
			shouldOutputTemplate = usingTemplate;
		}
		else {
			shouldOutputStory = true;
			shouldOutputTemplate = usingTemplate;
		}
		
		if (shouldOutputTemplate) {
			if (p->forceMode ||
				!fileExists(templateOutputPath) || 
				getFileModificationTime(templateOutputPath) < getFileModificationTime(inputPath) ||
				getFileModificationTime(templateOutputPath) < storyTemplate->timestamp)
			{
				/* Set the "new" and "modified" flags */
				if (!fileExists(templateOutputPath)) {
					Vars_let(currStory->variables, "is_new", "true", VAR_STD);
				}
				else if (getFileModificationTime(templateOutputPath) < getFileModificationTime(inputPath)) {
					Vars_let(currStory->variables, "is_modified", "true", VAR_STD);
				}
				Logging_infof("Generating file \"%s\"...", templateOutputPath);
				Template_execute(storyTemplate, currStory, templateOutputPath);
				if (p->scriptFile != NULL) {
					currFile = abasename(templateOutputPath);
					Publication_scriptPutFile(p, currFile);
					mu_free(currFile);
				}
			}
		}
		if (shouldOutputStory) {
			if (p->forceMode ||
				!fileExists(storyOutputPath) || 
				getFileModificationTime(storyOutputPath) < getFileModificationTime(inputPath))
			{
				if (!fileExists(storyOutputPath)) {
					Vars_let(currStory->variables, "is_new", "true", VAR_STD);
				}
				else if (getFileModificationTime(storyOutputPath) < getFileModificationTime(inputPath)) {
					Vars_let(currStory->variables, "is_modified", "true", VAR_STD);
				}
				Logging_infof("Copying file \"%s\"...", storyOutputPath);
				copyFile(inputPath, storyOutputPath);
				if (p->scriptFile != NULL) {
					currFile = abasename(storyOutputPath);
					Publication_scriptPutFile(p, currFile);
					mu_free(currFile);
				}
			}
		}
		
		/* Cleanup */
		mu_free(storyOutputPath);
		mu_free(templateOutputPath);
	}
	
	mu_free(outputDirectory);
	
	/* Generate subsections */
	for (ii = 0; ii < List_length(s->sections); ++ii){
		subSection = (Section *)List_get(s->sections, ii);
		if (isRoot) {
			sectionOutputPath = astrcpy(subSection->dir);
		}
		else {
			sectionOutputPath = buildPath(path, subSection->dir);
		}
		generateStories(p, subSection, sectionOutputPath);
		mu_free(sectionOutputPath);
	}
	
	if (!isRoot) Publication_scriptChdir(p, "..", true, false);
}


/** Creates the index files */
void generateIndexes(Publication *p, Section *s, const char *path){
	Index    *currIndex = NULL;
	size_t   ii, jj;
	bool     isRoot = false;
	Story    *currStory = NULL;
	char     *outputDirectory = NULL;
	char     *sectionOutputPath = NULL;
	char     *indexFile = NULL;
	char     *oldIndexFile = NULL;
	char     *indexOutputPath = NULL;
	Section  *subSection = NULL;
	bool     keepGoing = true;
	Template *indexTemplate   = NULL;
	Vars     *storyVars       = NULL;
	char     *templateFile    = NULL;
	char     *currDir  = NULL;
	char     *currFile = NULL;

	isRoot = (s == p->root); 
	/* Construct full output directory */
	if (isRoot) {
		outputDirectory = astrcpy(p->outputDirectory);
	}
	else {
		outputDirectory = buildPath(p->outputDirectory, path);
		if ((p->scriptFile != NULL)) {
			currDir = abasename(path);
			Publication_scriptChdir(p, currDir, true, true);
			mu_free(currDir);
		}
	}
	
	for (ii = 0; ii < List_length(s->indexes); ++ii) {
		currIndex = (Index *)List_get(s->indexes, ii);
		Index_dump(currIndex);
		List_moveFirst(currIndex->stories);
		keepGoing = true;
		if(Vars_defined(currIndex->variables, "index_file") && 
			strlen(Vars_get(currIndex->variables, "index_file")) > 0 &&
			Vars_defined(currIndex->variables, "index_template"))
		{
			templateFile = Vars_get(currIndex->variables, "index_template");
			indexTemplate = Publication_getTemplate(p, templateFile);
			/* While generating an index file, story files should search the
			** index scope before the section scope, so we swap change the 
			** parent in the variables structure
			*/
			for (jj = 0; jj < List_length(currIndex->stories); ++jj){
				currStory = (Story *)List_get(currIndex->stories, jj);
				storyVars = currStory->variables;
				storyVars->parent = currIndex->variables;
			}
			
			/* Generate index page(s) */
			indexFile = Vars_get(currIndex->variables, "index_file");
			while (keepGoing) {
				oldIndexFile = astrcpy(indexFile);
				indexOutputPath = buildPath(outputDirectory, indexFile);
				Template_execute(indexTemplate, currIndex, indexOutputPath);
				if ((p->scriptFile != NULL)) {
					currFile = abasename(indexOutputPath);
					Publication_scriptPutFile(p, currFile);
					mu_free(currFile);
				}
				mu_free(indexOutputPath);
				if (List_atEnd(currIndex->stories)) {
					keepGoing = false; /* No more stories in index */
				}
				else {
					/* If the index_file variable has changed, generate another 
					** index page with that name. This allows multi-page indexes
					** to be generated
					*/
					indexFile = Vars_get(currIndex->variables, "index_file");
					keepGoing = !strxequals(indexFile, oldIndexFile);
				}
				mu_free(oldIndexFile);
			}
			
			/* Restore normal scope */
			for (jj = 0; jj < List_length(currIndex->stories); ++jj){
				currStory = (Story *)List_get(currIndex->stories, jj);
				storyVars = currStory->variables;
				storyVars->parent = currStory->parent->variables;
			}
		}
	}
	mu_free(outputDirectory);
	
	/* Generate subsections */
	for (ii = 0; ii < List_length(s->sections); ++ii){
		subSection = (Section *)List_get(s->sections, ii);
		if (isRoot) {
			sectionOutputPath = astrcpy(subSection->dir);
		}
		else {
			sectionOutputPath = buildPath(path, subSection->dir);
		}
		generateIndexes(p, subSection, sectionOutputPath);
		mu_free(sectionOutputPath);
	}

	if (!isRoot) Publication_scriptChdir(p, "..", true, false);
}


/** Generates the tag indexes */
void generateTags(Publication *pub){
	Template *tpl    = NULL;
	char *outputFile = NULL;
	char *outputPath = NULL;
	char *outputExt  = NULL;
	char *tag = NULL;
	Tags *t = NULL;
	Pair *p = NULL;
	size_t ii, jj;
	
	for (ii = 0; ii < List_length(pub->tagList); ++ii) {
		t = (Tags *)List_get(pub->tagList, ii);
		/* Rewind all the iterators */
		List_moveFirst((List *)t->tags);
		for (jj = 0; jj < List_length((List *)t->tags); ++jj) {
			p = (Pair *)List_get((List *)t->tags, jj);
			List_moveFirst((List *)p->value);
		}
		if (Vars_defined(t->variables, "tag_template")) {
			tpl = Publication_getTemplate(pub, Vars_get(t->variables, "tag_template"));
			if (Vars_defined(t->variables, "tag_file")) {
				/* Single-file mode */
				outputFile = Vars_get(t->variables, "tag_file");
				outputPath = buildPath(pub->outputDirectory, outputFile);
				Template_execute(tpl, t, outputPath);
				Publication_scriptPutFile(pub, outputFile);
				mu_free(outputPath);
			}
			else {
				/* Multi-file mode */
				outputExt = getPathPart(Vars_get(t->variables, "tag_template"), PATH_EXT);
				while (true) {
					tag = ((Pair *)List_current((List *)t->tags))->key;
					Vars_let(t->variables, "current_tag", tag, VAR_STD);
					outputFile = asprintf("tag_%s_%s.%s", t->name, tag, outputExt);
					outputPath = buildPath(pub->outputDirectory, outputFile);
					Template_execute(tpl, t, outputPath);
					Publication_scriptPutFile(pub, outputFile);
					mu_free(outputFile);
					mu_free(outputPath);
					if(!List_moveNext((List *)t->tags)) break;
				}
				mu_free(outputExt);
			}
		}
	}
}


Index *findIndex(Section *s, const char *name){
	Index *idx = NULL;
	Section *subSection = NULL;
	size_t ii;
	
	for (ii = 0; ii < List_length(s->indexes); ++ii){
		idx = (Index *)List_get(s->indexes, ii);
		if (strxequals(idx->name, name)) return idx;
	}
	for (ii = 0; ii < List_length(s->sections); ++ii){
		subSection = (Section *)List_get(s->sections, ii);
		if ((idx = findIndex(subSection, name)) != NULL) return idx;
	}
	return NULL;
}


/** If the option to generate an FTP script has been selected, create the script 
*** file and write the commands necessary to connect and log into the host
***/
void Publication_createScript(Publication *p) {
	Vars *v = p->root->variables;
	if (strxnullorempty(p->scriptFileName)) return;
	
	if ((p->scriptFile = fopen(p->scriptFileName, "w")) == NULL) {
		Logging_warnf("Error opening script file %s: %s", 
			p->scriptFileName, strerror(errno)
		);
	}
	else {
		/* Append login information to script file */
		if (Vars_defined(v, "ftp_host")) {
			fprintf(p->scriptFile, "open %s\n", Vars_get(v, "ftp_host"));
		}
		if (Vars_defined(v, "ftp_user")) {
			fprintf(p->scriptFile, "user %s\n", Vars_get(v, "ftp_user"));
		}
		if (Vars_defined(v, "ftp_pass")) {
			fprintf(p->scriptFile, "%s\n", Vars_get(v, "ftp_pass"));
		}
	}
}


/** Close the FTP script file if one is being generated */
void Publication_closeScript(Publication *p) {
	if (p->scriptFile != NULL) {
		fprintf(p->scriptFile, "bye\n\n");
		fclose(p->scriptFile);
		p->scriptFile = NULL;
	}
}


/** Write a command to the FTP script file */
void Publication_scriptCommand(Publication *p, const char *command, const char *arg) {
	Vars *v = p->root->variables;

	if (p->scriptFile == NULL || strxnullorempty(arg)) return;
	fprintf(p->scriptFile, "%s \"%s\"\n", Vars_get(v, command), arg);
}


/** Write the command to upload a file to the FTP server */
void Publication_scriptPutFile(Publication *p, const char *fileName) {
	if (p->scriptFile == NULL || strxnullorempty(fileName)) return;
	/* Try to delete file first in case it already exists */
	Publication_scriptDeleteFile(p, fileName);
	Publication_scriptCommand(p, "ftp_put", fileName);
}


/** Write the command to change directory on the FTP server */
void Publication_scriptChdir(Publication *p, const char *directory, bool changeLocal, bool createFirst) {
	if (p->scriptFile == NULL || strxnullorempty(directory)) return;
	
	/* Change local directory too? */
	if (changeLocal) {
		Publication_scriptLchdir(p, directory);
	}
	/* Create directory first in case it doesn't exist? */
	if (createFirst) {
		Publication_scriptMkdir(p, directory);
	}
	Publication_scriptCommand(p, "ftp_chdir", directory);
}


/** Write the command to delete a file on the FTP server */
void Publication_scriptDeleteFile(Publication *p, const char *fileName) {
	if (p->scriptFile == NULL || strxnullorempty(fileName)) return;
	Publication_scriptCommand(p, "ftp_delete", fileName);
}


/** Write the command to create a directory on the FTP server */
void Publication_scriptMkdir(Publication *p, const char *directory) {
	if (p->scriptFile == NULL || strxnullorempty(directory)) return;
	Publication_scriptCommand(p, "ftp_mkdir", directory);
}


/** Write the command to change the local directory in the FTP client */
void Publication_scriptLchdir(Publication *p, const char *directory) {
	if (p->scriptFile == NULL || strxnullorempty(directory)) return;
	Publication_scriptCommand(p, "ftp_lcd", directory);
}

/** Callback function when static content is being copied from the template directory */
void onEnterDir(const char *directory, void *userData) {
	Publication *p = (Publication *)userData;
	Publication_scriptChdir(p, directory, true, true);
}


/** Callback function when static content is being copied from the template directory */
void onLeaveDir(void *userData) {
	Publication *p = (Publication *)userData;
	Publication_scriptChdir(p, "..", true, false);
}


/** Callback function when static content is being copied from the template directory */
void onCopyFile(const char *fileName, void *userData) {
	Publication *p = (Publication *)userData;
	Publication_scriptPutFile(p, fileName);
}


