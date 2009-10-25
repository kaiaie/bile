/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.c,v 1.32 2009/10/25 13:53:43 ken Exp $
 */
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "BileObj.h"
#include "bool.h"
#include "astring.h"
#include "Dict.h"
#include "Expr.h"
#include "FileHandler.h"
#include "HtmlHandler.h"
#include "ImgHandler.h"
#include "List.h"
#include "Logging.h"
#include "memutils.h"
#include "Pair.h"
#include "path.h"
#include "stringext.h"
#include "TextFile.h"
#include "tokenize.h"
#include "Type.h"


void addDir(Publication *p, Section *s, const char *path);
void readConfig(Publication *p, Section *s, const char *fileName);
Index *findIndex(Section *s, const char *name);
void generateStories(Publication *p, Section *s, const char *path);
void generateIndexes(Publication *p, Section *s, const char *path);
void generateTags(Publication *p);

static int sectionId = 1;
static int storyId = 1;


/*
 * addDir -- read a directory and add its contents to the publication
 */
void addDir(Publication *p, Section *s, const char *path){
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
	
	/* Check if at top level */
	if(s == p->root){
		Logging_debug("Loading toplevel directory");
		configFileName = pubConfigFileName;
		fullPath = astrcpy(p->inputDirectory);
		/* Add a few useful variables to the publication */
		Vars_let(s->variables, "pi", "3.141592653589793", VAR_CONST | VAR_NOSHADOW);
		Vars_let(s->variables, "tag_separator", ", ", VAR_CONST);
		Vars_let(s->variables, "tag_by", "keywords", VAR_CONST);
		Vars_let(s->variables, "error", "0", VAR_NOSHADOW);
	}
	else{
		Logging_debugf("Loading directory %s", path);
		configFileName = sectionConfigFileName;
		fullPath = buildPath(p->inputDirectory, path);
		Vars_let(s->variables, "path", path, VAR_STD);
		Vars_let(s->variables, "use_template", "false", VAR_STD);
		/* TODO: Figure out what section variables shouldn't be inherited and default them */
	}
	tmp = asprintf("%d", sectionId++);
	Vars_let(s->variables, "section_id", tmp, VAR_STD);
	mu_free(tmp);
	configFilePath = buildPath(fullPath, configFileName);
	/* Read the config file if it exists */
	if(access(configFilePath, F_OK | R_OK) == 0){
		readConfig(p, s, configFilePath);
	}
	else{
		Logging_warnf("Can't find configuration file %s: %s", configFilePath, strerror(errno));
	}
	mu_free(configFilePath);
	
	/* Read files */
	if((d = opendir(fullPath)) == NULL)
		Logging_fatalf("Error opening directory %s: %s", path, strerror(errno));
	while((e = readdir(d)) != NULL){
		/* Skip:
		 * - "." and ".."
		 * - CVS
		 * - Windows thumbs.db images file
		 * - Configuration files (i.e. files with ".bile" extension)
		 * - Files ending "~" or "#" (assumed to be backup files)
		 * (more exceptions might be necessary: move to its own function?)
		 */
		if(!strequals(e->d_name, ".") && !strequals(e->d_name, "..") && 
			!strequals(e->d_name, "CVS") &&
			!strequalsi(e->d_name, "thumbs.db") &&
			!strends(e->d_name, ".bile") && !strends(e->d_name, "~") && !strends(e->d_name, "#")){
			if(s == p->root)
				newPath = astrcpy(e->d_name);
			else
				newPath = buildPath(path, e->d_name);
			inputFilePath = buildPath(p->inputDirectory, newPath);
			if(stat(inputFilePath, &st) != 0){
				Logging_warnf("Error stat()'ing file %s: %s",
					inputFilePath, strerror(errno)
				);
				mu_free(inputFilePath);
				continue;
			}
			/* Is the current directory entry a subdirectory? */
			if(S_ISDIR(st.st_mode)){
				/* Create new section */
				newSection = new_Section(s, e->d_name);
				List_append(s->sections, newSection);
				addDir(p, newSection, newPath);
			}
			else {
				Logging_debugf("Reading metadata from file %s", inputFilePath);
				newStory = new_Story(s);
				tmp = asprintf("%d", storyId++);
				Vars_let(newStory->variables, "story_id", tmp, VAR_STD);
				mu_free(tmp);
				Vars_let(newStory->variables, "path", newPath, VAR_STD);
				/* Read metadata from file */
				/* TODO: Set up filehandlers properly */
				if(htmlCanHandle(inputFilePath))
					htmlReadMetadata(inputFilePath, newStory->variables);
				else if(imgCanHandle(inputFilePath))
					imgReadMetadata(inputFilePath, newStory->variables);
				defaultReadMetadata(inputFilePath, newStory->variables);
				/* The following special files should never be indexed or passed 
				 * through a template:
				 * .	favicon.ico
				 * .	robots.txt
				 * .	*.inc files
				 * .	files beginning with "." (e.g. .htaccess)
				 */
				if(
					strequalsi(e->d_name, "robots.txt") || 
					strequalsi(e->d_name, "favicon.ico") || 
					strends(e->d_name, ".inc") || 
					strends(e->d_name, ".INC") ||
					(e->d_name[0] == '.')
				){
					Vars_let(newStory->variables, "noindex", "true", VAR_STD);
					Vars_let(newStory->variables, "use_template", "false", VAR_STD);
				}
				/* Update "path" variable if the "use_template_ext" variable is
				 * set.
				 */
				if(Vars_defined(newStory->variables,"use_template") &&
					Type_toBool(Vars_get(newStory->variables,"use_template")) &&
					Vars_defined(newStory->variables,"use_template_ext") &&
					Type_toBool(Vars_get(newStory->variables,"use_template_ext"))){
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
	}
	closedir(d);
	mu_free(fullPath);
} /* addDir */


/*
 * generate - recursively generate a section
 */
void generateStories(Publication *p, Section *s, const char *path){
	Story    *currStory   = NULL;
	Section  *subSection  = NULL;
	size_t   ii;
	bool     usingTemplate    = false;
	bool     shouldOutputStory    = false;
	bool     shouldOutputTemplate = false;
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
	
	/* Construct full output directory */
	if(s == p->root)
		outputDirectory = astrcpy(p->outputDirectory);
	else
		outputDirectory = buildPath(p->outputDirectory, path);
	/* Create directory if it doesn't exist */
	if(!directoryExists(outputDirectory)) mkdirs(outputDirectory);
	
	for(ii = 0; ii < List_length(s->stories); ++ii){
		/* Copy story file to output directory */
		currStory  = (Story *)List_get(s->stories, ii);
		storyFile  = Vars_get(currStory->variables, "file_name");
		
		/* Determine full paths to input and output files */
		if(s == p->root)
			inputPath  = buildPath(p->inputDirectory, storyFile);
		else
			inputPath = asprintf("%s/%s/%s", p->inputDirectory, path, storyFile);
		currStory->inputPath = astrcpy(inputPath);
		storyOutputPath = buildPath(outputDirectory, storyFile);
		
		/* Determine output mode:
		 * NORMAL = create a single file in the output directory from each 
		 *          input file, optionally using a template.
		 * NONE   = do not produce output; delete any existing copies of the 
		 *          input file in the output directory.
		 * BOTH   = create up to two files in the output directory for each 
		 *          input file; one consisting of a copy of the input file, the 
		 *          other using a template (if one is specified).  This is 
		 *          useful when the input file is not HTML.  For example, this 
		 *          can be used with image files to generate a gallery.
		 */
		if(Vars_defined(currStory->variables, "output_mode") && 
			strequalsi(Vars_get(currStory->variables, "output_mode"), "none")){
			outputMode = OUTPUT_NONE;
		}
		else if(Vars_defined(currStory->variables, "output_mode") && 
			strequalsi(Vars_get(currStory->variables, "output_mode"), "both")){
			outputMode = OUTPUT_BOTH;
		}
		else
			outputMode = OUTPUT_NORMAL;
		
		/* Get template, if using */
		if((usingTemplate = Type_toBool(Vars_get(currStory->variables, "use_template")))){
			templateFile  = Vars_get(currStory->variables, "template_file");
			storyTemplate = Publication_getTemplate(p, templateFile);
			/* If using a template and the use_template_ext variable is set, 
			 * the file extension of the output file is that of the template 
			 * rather than that of the input file
			 */
			if(Vars_defined(currStory->variables, "use_template_ext") && 
				Type_toBool(Vars_get(currStory->variables, "use_template_ext"))){
				templateExt = getPathPart(templateFile, PATH_EXT);
				fileNoExt   = getPathPart(storyFile, PATH_FILEONLY);
				fileWithExt = asprintf("%s.%s", fileNoExt, templateExt);
				templateOutputPath = buildPath(outputDirectory, fileWithExt);
				mu_free(fileWithExt);
				mu_free(fileNoExt);
				mu_free(templateExt);
			}
			else
				templateOutputPath = astrcpy(storyOutputPath);
		}
		
		
		/* Determine whether output is to be generated */
		if(outputMode == OUTPUT_NONE){
			/* Generate no output and delete any existing output files */
			shouldOutputStory = false;
			shouldOutputTemplate = false;
			if(fileExists(storyOutputPath)) unlink(storyOutputPath);
			if(usingTemplate && fileExists(templateOutputPath))
				unlink(templateOutputPath);
		}
		else if(outputMode == OUTPUT_NORMAL){
			shouldOutputStory = !usingTemplate;
			shouldOutputTemplate = usingTemplate;
		}
		else{
			shouldOutputStory = true;
			shouldOutputTemplate = usingTemplate;
		}
		
		if(shouldOutputTemplate){
			if(p->forceMode ||
				!fileExists(templateOutputPath) || 
				getFileModificationTime(templateOutputPath) < getFileModificationTime(inputPath) ||
				getFileModificationTime(templateOutputPath) < storyTemplate->timestamp){
				if(!fileExists(templateOutputPath))
					Vars_let(currStory->variables, "is_new", "true", VAR_STD);
				else if(getFileModificationTime(templateOutputPath) < getFileModificationTime(inputPath))
					Vars_let(currStory->variables, "is_modified", "true", VAR_STD);
				Logging_infof("Generating file \"%s\"...", templateOutputPath);
				Template_execute(storyTemplate, currStory, templateOutputPath);
			}
		}
		if(shouldOutputStory){
			if(p->forceMode ||
				!fileExists(storyOutputPath) || 
				getFileModificationTime(storyOutputPath) < getFileModificationTime(inputPath)){
				if(!fileExists(storyOutputPath))
					Vars_let(currStory->variables, "is_new", "true", VAR_STD);
				else if(getFileModificationTime(storyOutputPath) < getFileModificationTime(inputPath))
					Vars_let(currStory->variables, "is_modified", "true", VAR_STD);
				Logging_infof("Copying file \"%s\"...", storyOutputPath);
				copyFile(inputPath, storyOutputPath);
			}
		}
		
		/* Cleanup */
		mu_free(storyOutputPath);
		mu_free(templateOutputPath);
	}
	
	mu_free(outputDirectory);
	
	/* Generate subsections */
	for(ii = 0; ii < List_length(s->sections); ++ii){
		subSection = (Section *)List_get(s->sections, ii);
		if(s == p->root)
			sectionOutputPath = astrcpy(subSection->dir);
		else
			sectionOutputPath = buildPath(path, subSection->dir);
		generateStories(p, subSection, sectionOutputPath);
		mu_free(sectionOutputPath);
	}
} /* generateStories */


void generateIndexes(Publication *p, Section *s, const char *path){
	Index    *currIndex = NULL;
	size_t   ii, jj;
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

	/* Construct full output directory */
	if(s == p->root)
		outputDirectory = astrcpy(p->outputDirectory);
	else
		outputDirectory = buildPath(p->outputDirectory, path);
	
	for(ii = 0; ii < List_length(s->indexes); ++ii){
		currIndex = (Index *)List_get(s->indexes, ii);
		Index_dump(currIndex);
		List_moveFirst(currIndex->stories);
		if(Vars_defined(currIndex->variables, "index_file") && 
			strlen(Vars_get(currIndex->variables, "index_file")) > 0 &&
			Vars_defined(currIndex->variables, "index_template")){
			templateFile = Vars_get(currIndex->variables, "index_template");
			indexTemplate = Publication_getTemplate(p, templateFile);
			/* While generating an index file, story files should search the
			 * index scope before the section scope.
			 */
			for(jj = 0; jj < List_length(currIndex->stories); ++jj){
				currStory = (Story *)List_get(currIndex->stories, jj);
				storyVars = currStory->variables;
				storyVars->parent = currIndex->variables;
			}
			
			/* Generate index page(s) */
			indexFile = Vars_get(currIndex->variables, "index_file");
			while(keepGoing){
				oldIndexFile = astrcpy(indexFile);
				indexOutputPath = buildPath(outputDirectory, indexFile);
				Template_execute(indexTemplate, currIndex, indexOutputPath);
				mu_free(indexOutputPath);
				if(List_atEnd(currIndex->stories))
					keepGoing = false; /* No more stories in index */
				else{
					/* If the index_file variable has changed, generate another 
					 * index page with that name.
					 */
					indexFile = Vars_get(currIndex->variables, "index_file");
					keepGoing = !strequals(indexFile, oldIndexFile);
				}
				mu_free(oldIndexFile);
			}
			
			/* Restore normal scope */
			for(jj = 0; jj < List_length(currIndex->stories); ++jj){
				currStory = (Story *)List_get(currIndex->stories, jj);
				storyVars = currStory->variables;
				storyVars->parent = currStory->parent->variables;
			}
		}
	}
	mu_free(outputDirectory);
	
	/* Generate subsections */
	for(ii = 0; ii < List_length(s->sections); ++ii){
		subSection = (Section *)List_get(s->sections, ii);
		if(s == p->root)
			sectionOutputPath = astrcpy(subSection->dir);
		else
			sectionOutputPath = buildPath(path, subSection->dir);
		generateIndexes(p, subSection, sectionOutputPath);
		mu_free(sectionOutputPath);
	}
} /* generateIndexes */


/*
 * readConfig - read a configuration file and define variables and indexes
 */
void readConfig(Publication *p, Section *s, const char *fileName){
	const char *aLine = NULL;
	TextFile *t = new_TextFile(fileName);
	List     *l = NULL;
	bool     gotIndex = false;
	bool     gotTags  = false;
	Index    *currIndex   = NULL;
	Tags     *currTags    = NULL;
	Vars     *currVars    = NULL;
	size_t   lineNo = 0;
	char     *varName = NULL;
	char     *varValue = NULL;
	size_t   ii;
	
	Logging_debugf("Reading configuration file %s", fileName);
	currVars = s->variables;
	while((aLine = TextFile_readLine(t)) != NULL){
		lineNo++;
		if(strempty(aLine) || aLine[0] == '#') continue; /* Skip blank lines and comments */
		l = tokenize(aLine);
		if(List_length(l) == 0){
			Logging_warnf("File %s, line %u: Parse error", fileName, lineNo);
			continue;
		}
		if(strequals((char *)List_get(l, 0), "index")){
			if(gotIndex || gotTags){
				Logging_warnf("File %s, line %u: Duplicate index/tag declaration", 
					fileName, lineNo
				);
			}
			else{
				gotIndex = true;
				currIndex = new_Index(s, (char *)List_get(l, 1));
				List_append(s->indexes, currIndex);
				currVars = currIndex->variables;
			}
		}
		else if(strequals((char *)List_get(l, 0), "endindex")){
			if(gotIndex){
				gotIndex = false;
				Logging_debugf("Index variables:");
				Vars_dump(currVars);
				currVars = s->variables;
			}
			else{
				Logging_warnf("File %s, line %u: Unexpected 'end index' encountered", 
					fileName, lineNo
				);
			}
		}
		/* TODO: Tag files only allowed in publication.bile; warn if in section.bile */
		else if(strequals((char *)List_get(l, 0), "tags")){
			if(gotIndex || gotTags){
				Logging_warnf("File %s, line %u: Duplicate index declaration", 
					fileName, lineNo
				);
			}
			else{
				gotTags = true;
				currTags = new_Tags(p, (char *)List_get(l, 1));
				List_append(p->tagList, currTags);
				currVars = currTags->variables;
			}
		}
		else if(strequals((char *)List_get(l, 0), "endtags")){
			if(gotTags){
				gotIndex = false;
				Logging_debugf("Tag variables:");
				Vars_dump(currVars);
				currVars = s->variables;
			}
			else{
				Logging_warnf("File %s, line %u: Unexpected 'end tags' encountered", 
					fileName, lineNo
				);
			}
		}
		else{
			varName = (char *)List_get(l, 0);
			/* Looking for lines of the form:
			 *     $varname = expression
			 */
			if(List_length(l) >= 3 && varName[0] == '$' && strequals((char *)List_get(l, 1), "=")){
				varName = astrcpy(&varName[1]);
				/* Remove the variable name and equals sign */
				List_remove(l, 0, true);
				List_remove(l, 0, true);
				/* Evaluate and store */
				varValue = evaluateTokens(l, currVars);
				Vars_let(currVars, varName, varValue, VAR_STD);
				mu_free(varName);
				mu_free(varValue);
			}
			else{
				Logging_warnf("File %s, line %u: Syntax error: expected variable declaration", 
					fileName, lineNo
				);
			}
		}
		delete_List(l, true);
	}
	delete_TextFile(t);
	Logging_debug("Section variables:");
	Vars_dump(currVars);
	Logging_debug("Section indexes:");
	for(ii = 0; ii < List_length(s->indexes); ++ii){
		Index_dump((Index *)List_get(s->indexes, ii));
	}
} /* readConfig */


/*
 * Publication_addToIndexes - add the story to section and publication indexes
 */
void Publication_addToIndexes(Publication *p, Section *s, Story *st){
	Index *idx;
	size_t ii;
	
	/* Update publication indexes */
	for(ii = 0; ii < List_length(p->root->indexes); ++ii){
		idx = (Index *)List_get(p->root->indexes, ii);
		Index_add(idx, st);
	}
	/* Update section indexes */
	if(s != p->root){
		for(ii = 0; ii < List_length(s->indexes); ++ii){
			idx = (Index *)List_get(s->indexes, ii);
			Index_add(idx, st);
		}
	}
}


Publication *new_Publication(char *inputDirectory, char *outputDirectory, 
	char *templateDirectory, bool forceMode, bool verboseMode){
	Publication *p = NULL;
	
	p = (Publication *)mu_malloc(sizeof(Publication));
	p->type = BILE_PUB;
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
	p->tagList           = new_List();
	return p;
}


void Publication_build(Publication *p){
	addDir(p, p->root, (char *)NULL);
}


void Publication_generate(Publication *p){
	DIR *d = NULL;
	struct dirent *e = NULL;
	char *srcPath = NULL;
	char *destPath = NULL;
	ReplaceOption option = REPLACE_OLDER;
	
	generateStories(p, p->root, (char *)NULL);
	generateIndexes(p, p->root, (char *)NULL);
	generateTags(p);
	
	/* Copy content from subdirectories in template directory to corresponding 
	 * directories in the output directory
	 */
	if(p->forceMode) option = REPLACE_ALWAYS;
	if((d = opendir(p->templateDirectory)) != NULL){
		while((e = readdir(d)) != NULL){
			if(!strequals(e->d_name, ".") && !strequals(e->d_name, "..") && !strequals(e->d_name, "CVS")){
				srcPath = buildPath(p->templateDirectory, e->d_name);
				if(directoryExists(srcPath)){
					destPath = buildPath(p->outputDirectory, e->d_name);
					if(!directoryExists(destPath)) pu_mkdir(destPath);
					copyDirectory(srcPath, destPath, option, false);
					mu_free(destPath);
				}
				mu_free(srcPath);
			}
		}
		closedir(d);
	}
}


Template *Publication_getTemplate(Publication *p, char *fileName){
	Template *result       = NULL;
	char     *templatePath = NULL;
	
	if(!Dict_exists(p->templateCache, fileName)){
		templatePath = buildPath(p->templateDirectory, fileName);
		result = Template_compile(templatePath);
		Dict_put(p->templateCache, fileName, result);
		mu_free(templatePath);
	}
	else
		result = (Template *)Dict_get(p->templateCache, fileName);
	return result;
}


void Publication_dump(Publication *p){
	if(p != NULL){
		Logging_warnf("%s: Not implemented yet", __FUNCTION__);
	}
	else
		Logging_warnf("%s: NULL argument", __FUNCTION__);
}


Section *new_Section(Section *parent, char *dir){
	Section *s = NULL;
	s = (Section *)mu_malloc(sizeof(Section));
	s->type = BILE_SECTION;
	s->parent = parent;
	if(parent == NULL)
		s->variables = new_Vars(NULL);
	else
		s->variables = new_Vars(parent->variables);
	s->dir = astrcpy(dir);
	s->sections  = new_List();
	s->indexes   = new_List();
	s->stories   = new_List();
	return s;
}


Story *new_Story(Section *parent){
	Story *s = NULL;
	s = (Story *)mu_malloc(sizeof(Story));
	s->type = BILE_STORY;
	s->variables = new_Vars(parent->variables);
	s->parent = parent;
	s->inputPath = NULL;
	s->tags = new_Dict();
	/* Add default variables */
	Vars_let(s->variables, "is_new", "false", VAR_STD);
	Vars_let(s->variables, "is_modified", "false", VAR_STD);
	return s;
}


Index *new_Index(Section *parent, const char *name){
	Index *i = NULL;
	i = (Index *)mu_malloc(sizeof(Index));
	i->type   = BILE_INDEX;
	i->parent = parent;
	i->name   = astrcpy(name);
	i->variables = new_Vars(parent->variables);
	i->stories = new_List();
	return i;
}


bool Index_add(Index *idx, Story *st){
	size_t ii;
	Story *ss = NULL;
	char *sortVar = NULL;
	bool added = false;
	int  cmpVal = 1;
	
	/* Skip if noindex variable is defined and true
	 * ( noindex = TRUE is a bit ass-backwards but I think it's better to index 
	 * by default)
	 */
	if(Vars_defined(st->variables, "noindex") && Type_toBool(Vars_get(st->variables, "noindex")))
		return true;
	if(List_length(idx->stories) == 0)
		List_append(idx->stories, st);
	else{
		sortVar = Vars_get(idx->variables, "sort_by");
		/* Simple insertion sort; should be OK for small indexes */
		/* Note: uses strcmp; could be a problem with numeric values */
		cmpVal = (sortVar[0] == '+') ? 1 : -1;
		for(ii = 0; ii < List_length(idx->stories); ++ii){
			ss = (Story *)List_get(idx->stories, ii);
			if(strcmp(Vars_get(ss->variables, &sortVar[1]), 
				Vars_get(st->variables, &sortVar[1])) == cmpVal){
				List_insert(idx->stories, ii, st);
				Logging_debugf("Added story %s at position %u of index %s",
					Vars_get(st->variables, "file_name"),
					ii,
					idx->name
				);
				added = true;
				break;
			}
		}
		if(!added){
			List_append(idx->stories, st);
			Logging_debugf("Added story %s at end of index %s",
				Vars_get(st->variables, "file_name"),
				idx->name
			);
		}
	}
	return true;
}


Index *findIndex(Section *s, const char *name){
	Index *idx = NULL;
	Section *subSection = NULL;
	size_t ii;
	
	for(ii = 0; ii < List_length(s->indexes); ++ii){
		idx = (Index *)List_get(s->indexes, ii);
		if(strequals(idx->name, name)) return idx;
	}
	for(ii = 0; ii < List_length(s->sections); ++ii){
		subSection = (Section *)List_get(s->sections, ii);
		if((idx = findIndex(subSection, name)) != NULL) return idx;
	}
	return NULL;
}


/* Publication_findIndex: find an index with the specified name
 */
Index *Publication_findIndex(Publication *p, const char *name){
	return findIndex(p->root, name);
}


void Index_dump(Index *idx){
	size_t ii;
	Story *st = NULL;
	
	Logging_debugf("Dumping index: %s", idx->name);
	Logging_debug("Index variables:");
	Vars_dump(idx->variables);
	Logging_debug("Indexed files:");
	for(ii = 0; ii < List_length(idx->stories); ++ii){
		st = (Story *)List_get(idx->stories, ii);
		Logging_debugf("\t%s", Vars_get(st->variables, "file_name"));
	}
}


Tags *new_Tags(Publication *parent, const char *name){
	Tags *t      = mu_malloc(sizeof(Tags));
	t->type      = BILE_TAGS;
	t->name      = astrcpy(name);
	t->variables = new_Vars(parent->root->variables);
	t->tags      = new_List();
	return t;
} /* new_Tags */


bool Tags_add(Tags *t, Story *st){
	char *tagVar    = NULL;
	char *tagSep    = NULL;
	char *tags      = NULL;
	char **tagArray = NULL;
	char *tag = NULL;
	List *l = NULL;
	size_t ii = 0, jj;
	bool found = false;
	/* Check story is not excluded from tags */
	if(!(Vars_defined(st->variables, "notags") && Type_toBool(Vars_get(st->variables, "notags")))){
		/* What variable is used to hold the tags? */
		tagVar = Vars_get(t->variables, "tag_by");
		/* Separator characters */
		tagSep = Vars_get(t->variables, "tag_separator");
		if(Vars_defined(st->variables, tagVar)){
			tags = Vars_get(st->variables, tagVar);
			tagArray = astrtok(tags, tagSep);
			while((tag = tagArray[ii]) != NULL){
				if(!strempty(tag)){
					strlower(tag);
					/* Add tags to publication's tags */
					if(Dict_exists(t->tags, tag))
						l = (List *)Dict_get(t->tags, tag);
					else{
						l = new_List();
						Dict_putSorted(t->tags, tag, l);
					}
					List_append(l, st);
					/* Add tags to story's tags */
					if(Dict_exists(st->tags, t->name))
						l = (List *)Dict_get(st->tags, t->name);
					else{
						l = new_List();
						Dict_put(st->tags, t->name, l);
					}
					found = false;
					if(List_length(l) > 0){
						for(jj = 0; jj < List_length(l); ++jj){
							if(strcmp(tag, (char *)List_get(l, jj)) < 0){
								List_insert(l, jj, astrcpy(tag));
								found = true;
								break;
							}
							else if(strequals(tag, (char *)List_get(l, jj))){
								found = true;
								break;
							}
						}
					}
					if(!found) List_append(l, astrcpy(tag));
				}
				ii++;
			}
			astrtokfree(tagArray);
		}
	}
	return true;
} /* Tags_add */


bool Publication_addToTags(Publication *p, Story *st){
	Tags *t;
	size_t ii;
	for(ii = 0; ii < List_length(p->tagList); ++ii){
		t = (Tags *)List_get(p->tagList, ii);
		Tags_add(t, st);
	}
	return true;
} /* Publication_addToTags */


void generateTags(Publication *pub){
	Template *tpl    = NULL;
	char *outputFile = NULL;
	char *outputPath = NULL;
	char *outputExt  = NULL;
	char *tag = NULL;
	Tags *t = NULL;
	Pair *p = NULL;
	size_t ii, jj;
	
	for(ii = 0; ii < List_length(pub->tagList); ++ii){
		t = (Tags *)List_get(pub->tagList, ii);
		/* Rewind all the iterators */
		List_moveFirst((List *)t->tags);
		for(jj = 0; jj < List_length((List *)t->tags); ++jj){
			p = (Pair *)List_get((List *)t->tags, jj);
			List_moveFirst((List *)p->value);
		}
		if(Vars_defined(t->variables, "tag_template")){
			tpl = Publication_getTemplate(pub, Vars_get(t->variables, "tag_template"));
			if(Vars_defined(t->variables, "tag_file")){
				/* Single-file mode */
				outputPath = buildPath(pub->outputDirectory, Vars_get(t->variables, "tag_file"));
				Template_execute(tpl, t, outputPath);
				mu_free(outputPath);
			}
			else{
				/* Multi-file mode */
				outputExt = getPathPart(Vars_get(t->variables, "tag_template"), PATH_EXT);
				while(true){
					tag = ((Pair *)List_current((List *)t->tags))->key;
					outputFile = asprintf("tag_%s_%s.%s", t->name, tag, outputExt);
					outputPath = buildPath(pub->outputDirectory, outputFile);
					Template_execute(tpl, t, outputPath);
					mu_free(outputFile);
					mu_free(outputPath);
					if(!List_moveNext((List *)t->tags)) break;
				}
				mu_free(outputExt);
			}
		}
	}
} /* generateTags */


Tags *Publication_findTags(Publication *p, const char *name){
	Tags *result = NULL;
	size_t ii;
	
	for(ii = 0; ii < List_length(p->tagList); ++ii){
		if(strequals(name, ((Tags *)List_get(p->tagList, ii))->name)){
			result = List_get(p->tagList, ii);
			break;
		}
	}
	return result;
} /* Publication_findTags */


