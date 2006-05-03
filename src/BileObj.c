/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.c,v 1.8 2006/05/03 15:05:44 ken Exp $
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
	Section *currSection = NULL;
	Section *newSection  = NULL;
	Story   *newStory    = NULL;
	
	/* If section is NULL, we're at the top level */
	if(s == NULL){
		Logging_debug("Loading toplevel directory");
		configFileName = pubConfigFileName;
		fullPath = astrcpy(p->inputDirectory);
		currSection = (Section *)p;
		/* Add a few useful variables to the publication */
		Vars_let(p->variables, "pi", "3.141592653589793");
	}
	else{
		Logging_debugf("Loading directory %s", path);
		currSection = s;
		configFileName = sectionConfigFileName;
		fullPath = buildPath(p->inputDirectory, path);
		Vars_let(currSection->variables, "path", astrcpy(path));
		Vars_let(s->variables, "use_template", "false");
		/* TODO: Figure out what section variables shouldn't be inherited and default them */
	}
	Vars_let(currSection->variables, "section_id", asprintf("%d", sectionId++));
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
			if(s == NULL)
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
				newSection = new_Section(currSection, e->d_name);
				List_append(currSection->sections, newSection);
				addDir(p, newSection, newPath);
			}
			else {
				Logging_debugf("Reading metadata from file %s", fullName);
				newStory = new_Story(currSection);
				Vars_let(newStory->variables, "story_id", asprintf("%d", storyId++));
				/* Read metadata from file */
				/* TODO: Set up filehandlers properly */
				if(htmlCanHandle(fullName))
					htmlReadMetadata(fullName, newStory->variables);
				else if(imgCanHandle(fullName))
					imgReadMetadata(fullName, newStory->variables);
				defaultReadMetadata(fullName, newStory->variables);
				List_append(currSection->stories, newStory);
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
	Section *currSection = NULL;
	Section *subSection  = NULL;
	Story   *currStory   = NULL;
	size_t  ii;
	bool    usingTemplate    = false;
	bool    shouldOutput     = false;
	char    *storyFile       = NULL;
	char    *inputPath       = NULL;
	char    *outputDirectory = NULL;
	char    *outputPath      = NULL;
	char    *templateFile    = NULL;
	Template *storyTemplate  = NULL;
	FILE    *outputFile      = NULL;
	
	currSection = (s == NULL) ? (Section *)p : s;
	
	for(ii = 0; ii < List_length(currSection->stories); ++ii){
		/* Copy story file to output directory */
		/* TODO: Use template's file extension? */
		currStory  = (Story *)List_get(currSection->stories, ii);
		storyFile  = Vars_get(currStory->variables, "file_name");
		
		/* Determine output directory */
		if(s == NULL)
			outputDirectory = astrcpy(p->outputDirectory);
		else
			outputDirectory = buildPath(p->outputDirectory, path);
		
		/* Create directory if it doesn't exist */
		if(!directoryExists(outputDirectory)) mkdirs(outputDirectory);
		
		/* Determine full paths to input and output files */
		if(s == NULL)
			inputPath  = buildPath(p->inputDirectory, storyFile);
		else
			inputPath = asprintf("%s/%s/%s", p->inputDirectory, path, storyFile);
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
			Vars_let(currStory->variables, "is_new", "true");
			shouldOutput = true;
		}
		else{
			if(usingTemplate && (getFileModificationTime(outputPath) < storyTemplate->timestamp)){
				/* Update output if template has been altered (but don't set 
				 * the modification flag)
				 */
				shouldOutput = true;
			}
			else if(getFileModificationTime(outputPath) < getFileModificationTime(inputPath)){
				/* Update output if input has been altered */
				Vars_let(currStory->variables, "is_modified", "true");
				shouldOutput = true;
			}
			else{
				/* Otherwise only generate/copy if the force flag is set */
				shouldOutput = p->forceMode;
			}
		}
		
		/* Generate output */
		if(shouldOutput){
			if(usingTemplate){
				/* Use template */
				outputFile = fopen(outputPath, "w");
				Template_execute(storyTemplate, currStory->variables, inputPath, outputFile);
				fclose(outputFile);
			}
			else{
				/* Straight copy */
				copyFile(inputPath, outputPath);
			}
		}
		else
			Logging_debugf("Output file \"%s\" is up to date.", outputPath);
		
		/* Cleanup */
		mu_free(inputPath);
		mu_free(outputPath);
		mu_free(outputDirectory);
	}
	
	/* TODO: Generate index pages */
	
	/* Copy subsections */
	for(ii = 0; ii < List_length(currSection->sections); ++ii){
		subSection = (Section *)List_get(currSection->sections, ii);
		if(s == NULL)
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
	Section  *currSection = NULL;
	Index    *currIndex   = NULL;
	Vars     *currVars    = NULL;
	size_t   lineNo = 0;
	char     *varName = NULL;
	char     *varValue = NULL;
	Expr     *e = NULL;
	size_t   ii;
	
	Logging_debugf("Reading configuration file %s", fileName);
	if(s == NULL)
		currSection = (Section *)p;
	else
		currSection = s;
	currVars = currSection->variables;
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
				currIndex = new_Index(currSection, (char *)List_get(l, 1));
				List_append(currSection->indexes, currIndex);
				currVars = currIndex->variables;
			}
		}
		else if(strequals((char *)List_get(l, 0), "endindex")){
			if(gotIndex){
				gotIndex = false;
				Logging_debugf("Index variables:");
				Vars_dump(currVars);
				currVars = currSection->variables;
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
	if(s != NULL){
		Logging_debug("Section indexes:");
		for(ii = 0; ii < List_length(s->indexes); ++ii){
			Index_dump((Index *)List_get(s->indexes, ii));
		}
	}
}


/*
 * updateIndexes - add the story to section and publication indexes
 */
void updateIndexes(Publication *p, Section *s, Story *st){
	Index *idx;
	size_t ii;
	
	/* Update publication indexes */
	for(ii = 0; ii < List_length(p->indexes); ++ii){
		idx = (Index *)List_get(p->indexes, ii);
		Index_add(idx, st);
	}
	/* Update section indexes */
	if(s != NULL){
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
	p->dir = astrcpy(".");
	p->variables = new_Vars((Vars *)NULL);
	Vars_let(p->variables, "input_directory",   inputDirectory);
	Vars_let(p->variables, "output_directory",  outputDirectory);
	Vars_let(p->variables, "template_directory", templateDirectory);
	p->sections  = new_List();
	p->indexes   = new_List();
	p->stories   = new_List();
	p->inputDirectory    = astrcpy(inputDirectory);
	p->outputDirectory   = astrcpy(outputDirectory);
	p->templateDirectory = astrcpy(templateDirectory);
	p->templateCache     = new_Dict();
	p->forceMode         = forceMode;
	p->verboseMode       = verboseMode;
	return p;
}


void Publication_build(Publication *p){
	addDir(p, (Section *)NULL, (char *)NULL);
}


void Publication_generate(Publication *p){
	/* TODO: Check output directory exists and is writeable */
	generate(p, (Section *)NULL, (char *)NULL);
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
	s->variables = new_Vars(parent->variables);
	s->parent = parent;
	/* Add default variables */
	Vars_let(s->variables, "is_new", "false");
	Vars_let(s->variables, "is_modified", "false");
	return s;
}


Index *new_Index(Section *parent, const char *name){
	Index *i = NULL;
	i = (Index *)mu_malloc(sizeof(Index));
	i->name = astrcpy(name);
	i->variables = new_Vars(parent->variables);
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

