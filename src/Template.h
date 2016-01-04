/** \file Template.h
*** \brief Implements the template processor
**/

#ifndef TEMPLATE_H
#define TEMPLATE_H
#include <stdio.h>
#include <time.h>
#include "List.h"
#include "Vars.h"

/** Indicates the type of Command represented by the Statement */
typedef enum {
	/** \brief Statement represents a non-block Command */
	ST_SIMPLE,
	/** \brief Statement represents the opening of a block Command */
	ST_BEGIN, 
	/** \brief Statement represents the closing of a block Commant */
	ST_END
} StatementType;

/** \brief A Statement represents a particular instance of a Command within a 
*** Template
***
*** \sa Command, Template
**/
typedef struct tag_statement {
   StatementType type;
   /** \brief The number of the line in the Template file on which the statement 
   *** appears
   **/
   size_t        lineNo;
   /** \brief The text of the command */
   char          *cmd;
   /** \brief Any parameters passed to the command */
   char          *param;
   /** \brief Set to True if the command is a block command and a \c [[BREAK]] 
   *** or \c [[BREAKIF]] command has been called during its execution
   **/
   bool          broken;
   /** \brief Additional data the Command might need */
   void          *userData;
} Statement;

/** The Template data type */
typedef struct tag_template {
	/** \brief The name of the template file */
	char   *fileName;
	/** \brief The last modification date of the template file
	***
	*** This is used to determine when a Story file should be processed by the 
	*** template processor.  It can be overridden, for example, if a Command 
	*** inserts dynamic content into the Template
	**/
	time_t timestamp;
	/** \brief The Bile object being processed by the Template */
	void   *context;
	/** \brief Saved context */
	void   *savedContext;
	/** The name of the template file */
	char   *inputFile;
	FILE   *outputFile;
	/** \brief The name of the file which the Template's output will be written 
	***  to.
	***
	*** If this member is NULL, the Template's output will be written to 
	*** standard output
	**/
	char   *outputFileName;
	/** \brief The Statements that make up the Template */
	List   *statements;
} Template;

Template *new_Template(void);
Template *Template_compile(char *fileName);
void     Template_execute(Template *template, void *context, char *outputFileName);
void     delete_Template(Template *t);

Statement *Template_findMatching(Template *template, Statement *s);


/* Debugging functions */
void Template_debugPrintTemplate(Template *template);


#endif /* TEMPLATE_H */
