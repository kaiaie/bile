/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.c,v 1.14 2006/05/05 14:10:42 ken Exp $
 */
#include <dirent.h>
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
#include "path.h"
#include "stringext.h"
#include "TextFile.h"
#include "tokenize.h"
#include "Type.h"


void addDir(Publication *p, Section *s, const char *path);
void readConfig(Publication *p, Section *s, const char *fileName);
void updateIndexes(Publication *p, Section *s, Story *st);
void generate(Publication *p, Section *s, const char *path);

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
	char    *fullPath    = NULL;
	char    *newPath     = NULL;
	char    *fullName    = NULL;
	Section *newSection  = NULL;
	Story   *newStory    = NULL;
	
	/* Check if at top level */
	if(s == p->root){
		Logging_debug("Loading toplevel directory");
		configFileName = pubConfigFileName;
		fullPath = astrcpy(p->inputDirectory);
		/* Add a few useful variables to the publication */
		Vars_let(s->variables, "pi", astrcpy("3.141592653589793"));
	}
	else{
		Logging_debugf("Loading directory %s", path);
		configFileName = sectionConfigFileName;
		fullPath = buildPath(p->inputDirectory, path);
		Vars_let(s->variables, "path", astrcpy(path));
		Vars_let(s->variables, "use_template", astrcpy("false"));
		/* TODO: Figure out what section variables shouldn't be inherited and default them */
	}
	Vars_let(s->variables, "section_id", asprintf("%d", sectionId++));
	fullName = buildPath(fullPath, configFileName);
	/* Read the config file if it exists */
	if(access(fullName, F_OK | R_OK) == 0){
		readConfig(p, s, fullName);
	}
	else{
		Logging_warnf("Can't find configuration file %s: %s", fullName, strerror(errno));
	}
	mu_free(fullName);
	
	/* Read files */
	if((d = opendir(fullPath)) == NULL)
		Logging_fatalf("Error opening directory %s: %s", path, strerror(errno));
	while((e = readdir(d)) != NULL){
		/* Skip:
		 * - "." and ".."
		 * - CVS
		 * - Configuration files (i.e. files with ".bile" extension)
		 * - Files ending "~" (assumed to be backup files)
		 * (more exceptions might be necessary: move to its own function?)
		 */
		if(!strequals(e->d_name, ".") && !strequals(e->d_name, "..") && 
			!strequals(e->d_name, "CVS") &&
			!strends(e->d_name, ".bile") && !strends(e->d_name, "~")){
			if(s == p->root)
				newPath = astrcpy(e->d_name);
			else
				newPath = buildPath(path, e->d_name);
			fullName = buildPath(p->inputDirectory, newPath);
			if(stat(fullName, &st) != 0){
				Logging_warnf("Error stat()'ing file %s: %s",
					fullName, strerror(errno)
				);
				mu_free(fullName);
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
				Logging_debugf("Reading metadata from file %s", fullName);
				newStory = new_Story(s);
				Vars_let(newStory->variables, "story_id", asprintf("%d", storyId++));
				Vars_let(newStory->variables, "path", astrcpy(newPath));
				/* Read metadata from file */
				/* TODO: Set up filehandlers properly */
				if(htmlCanHandle(fullName))
					htmlReadMetadata(fullName, newStory->variables);
				else if(imgCanHandle(fullName))
					imgReadMetadata(fullName, newStory->variables);
				defaultReadMetadata(fullName, newStory->variables);
				List_append(s->stories, newStory);
				Logging_debug("Story variables:");
				Vars_dump(newStory->variables);
				updateIndexes(p, s, newStory);
			}
			mu_free(newPath);
			mu_free(fullName);
		}
	}
	closedir(d);
	mu_free(fullPath);
}


/*
 * generate - recursively generate a section
 */
void generate(Publication *p, Section *s, const char *path){
	Story    *currStory   = NULL;
	Index    *currIndex   = NULL;
	Section  *subSection  = NULL;
	size_t   ii, jj;
	bool     usingTemplate    = false;
	bool     shouldOutput     = false;
	bool     keepGoing        = true;
	char     *storyFile       = NULL;
	char     *indexFile       = NULL;
	char     *oldIndexFile    = NULL;
	char     *inputPath       = NULL;
	char     *outputDirectory = NULL;
	char     *outputPath      = NULL;
	char     *templateFile    = NULL;
	Template *storyTemplate  = NULL;
	Template *indexTemplate  = NULL;
	FILE     *outputFile      = NULL;
	Vars     *storyVars       = NULL;
	
	/* Construct full output directory */
	if(s == p->root)
		outputDirectory = astrcpy(p->outputDirectory);
	else
		outputDirectory = buildPath(p->outputDirectory, path);
	
	for(ii = 0; ii < List_length(s->stories); ++ii){
		/* Copy story file to output directory */
		/* TODO: Use template's file extension? */
		currStory  = (Story *)List_get(s->stories, ii);
		storyFile  = Vars_get(currStory->variables, "file_name");
		
		/* Create directory if it doesn't exist */
		if(!directoryExists(outputDirectory)) mkdirs(outputDirectory);
		
		/* Determine full paths to input and output files */
		if(s == p->root)
			inputPath  = buildPath(p->inputDirectory, storyFile);
		else
			inputPath = asprintf("%s/%s/%s", p->inputDirectory, path, storyFile);
		currStory->inputPath = astrcpy(inputPath);
		outputPath = buildPath(outputDirectory, storyFile);
		
		/* Get template, if using */
		if((usingTemplate = Type_toBool(Vars_get(currStory->variables, "use_template")))){
			templateFile = Vars_get(currStory->variables, "template_file");
			storyTemplate = Publication_getTemplate(p, templateFile);			
		}
		
		/* Determine whether output is to be generated */
		if(!fileExists(outputPath)){
			/* If the output file doesn't exist yet, generate it and set is_new 
			 * flag.
			 */
			Vars_let(currStory->variables, "is_new", astrcpy("true"));
			shouldOutput = true;
		}
		else{
			if(usingTemplate && (getFileModificationTime(outputPath) < storyTemplate->timestamp)){
				/* Update output if template has been altered (but don't set 
				 * the modification flag)
				 */
				shouldOutput = true;
			}
			if(getFileModificationTime(outputPath) < getFileModificationTime(inputPath)){
				/* Update output if input has been altered */
				Vars_let(currStory->variables, "is_modified", astrcpy("true"));
				shouldOutput = true;
			}
			else{
				/* Otherwise only generate/copy if the force flag is set */
				shouldOutput = p->forceMode;
			}
		}
		
		/* Generate output */
		if(shouldOutput){
			Logging_infof("Writing output file \"%s\"", outputPath);
			if(usingTemplate){
				/* Use template */
				outputFile = fopen(outputPath, "w");
				Template_execute(storyTemplate, currStory, outputFile);
				fclose(outputFile);
			}
			else{
				/* Straight copy */
				copyFile(inputPath, outputPath);
			}
		}
		else
			Logging_infof("Output file \"%s\" is up to date.", outputPath);
		
		/* Cleanup */
		mu_free(outputPath);
	}
	
	for(ii = 0; ii < List_length(s->indexes); ++ii){
		currIndex = (Index *)List_get(s->indexes, ii);
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
				outputPath = buildPath(outputDirectory, indexFile);
				outputFile = fopen(outputPath, "w");
				Template_execute(indexTemplate, currIndex, outputFile);
				fclose(outputFile);
				mu_free(outputPath);
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
	
	/* Copy subsections */
	for(ii = 0; ii < List_length(s->sections); ++ii){
		subSection = (Section *)List_get(s->sections, ii);
		if(s == p->root)
			outputPath = astrcpy(subSection->dir);
		else
			outputPath = buildPath(path, subSection->dir);
		generate(p, subSection, outputPath);
		mu_free(outputPath);
	}
}


/*
 * readConfig - read a configuration file and define variables and indexes
 */
void readConfig(Publication *p, Section *s, const char *fileName){
	const char *aLine = NULL;
	TextFile *t = new_TextFile(fileName);
	List     *l = NULL;
	bool     gotIndex = false;
	Index    *currIndex   = NULL;
	Vars     *currVars    = NULL;
	size_t   lineNo = 0;
	char     *varName = NULL;
	char     *varValue = NULL;
	Expr     *e = NULL;
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
			if(gotIndex){
				Logging_warnf("File %s, line %u: Duplicate index declaration", 
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
				/* Evaluate */
				e = new_Expr2(l, currVars);
				varValue = Expr_evaluate(e);
				/* Store value */
				Vars_let(currVars, varName, varValue);
				/* Cleanup */
				mu_free(varName);
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
}


/*
 * updateIndexes - add the story to section and publication indexes
 */
void updateIndexes(Publication *p, Section *s, Story *st){
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
	Vars_let(p->root->variables, "input_directory",   astrcpy(inputDirectory));
	Vars_let(p->root->variables, "output_directory",  astrcpy(outputDirectory));
	Vars_let(p->root->variables, "template_directory", astrcpy(templateDirectory));
	p->inputDirectory    = astrcpy(inputDirectory);
	p->outputDirectory   = astrcpy(outputDirectory);
	p->templateDirectory = astrcpy(templateDirectory);
	p->templateCache     = new_Dict();
	p->forceMode         = forceMode;
	p->verboseMode       = verboseMode;
	return p;
}


void Publication_build(Publication *p){
	addDir(p, p->root, (char *)NULL);
}


void Publication_generate(Publication *p){
	/* TODO: Check output directory exists and is writeable */
	generate(p, p->root, (char *)NULL);
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
	/* Add default variables */
	Vars_let(s->variables, "is_new", astrcpy("false"));
	Vars_let(s->variables, "is_modified", astrcpy("false"));
	return s;
}


Index *new_Index(Section *parent, const char *name){
	Index *i = NULL;
	i = (Index *)mu_malloc(sizeof(Index));
	i->type = BILE_INDEX;
	i->name = astrcpy(name);
	i->variables = new_Vars(parent->variables);
	Vars_let(i->variables, "index_file", astrcpy(""));
	i->stories = new_List();
	return i;
}


bool Index_add(Index *idx, Story *st){
	size_t ii;
	Story *ss = NULL;
	char *sortVar = NULL;
	bool added = false;
	
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
		if(sortVar[0] == '+'){
			for(ii = 0; ii < List_length(idx->stories); ++ii){
				ss = (Story *)List_get(idx->stories, ii);
				if(strcmp(Vars_get(ss->variables, &sortVar[1]), 
					Vars_get(st->variables, &sortVar[1])) == 1){
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
		else{
			for(ii = List_length(idx->stories); ii >= 1; --ii){
				ss = (Story *)List_get(idx->stories, ii - 1);
				if(strcmp(Vars_get(ss->variables, &sortVar[1]), 
					Vars_get(st->variables, &sortVar[1])) == -1){
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
				List_insert(idx->stories, 0, st);
				Logging_debugf("Added story %s at beginning of index %s",
					Vars_get(st->variables, "file_name"),
					idx->name
				);
			}
		}
	}
	return true;
}


void Index_dump(Index *idx){
	size_t ii;
	Story *st = NULL;
	
	Logging_debugf("Index: %s", idx->name);
	Logging_debug("Index variables:");
	Vars_dump(idx->variables);
	Logging_debug("Indexed files:");
	for(ii = 0; ii < List_length(idx->stories); ++ii){
		st = (Story *)List_get(idx->stories, ii);
		Logging_debugf("\t%s", Vars_get(st->variables, "file_name"));
	}
}

