/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Template.h,v 1.12 2010/08/24 22:10:37 ken Exp $
 */
#ifndef TEMPLATE_H
#define TEMPLATE_H
#include <stdio.h>
#include <time.h>
#include "List.h"
#include "Vars.h"


typedef enum {ST_SIMPLE, ST_BEGIN, ST_END} StatementType;

/** \brief A Statement represents a particular instance of a Command within a 
 * Template
 *
 * \sa Command, Template
 */
typedef struct tag_statement{
   StatementType type;
   /** \brief The number of the line in the Template file on which the statement 
    * appears
	*/
   size_t        lineNo;
   /**
    * \brief The text of the command
	*/
   char          *cmd;
   /**
    * \brief Any parameters passed to the command
	*/
   char          *param;
   /**
    * \brief Set to True if the command is a block command and a \c [[BREAK]] or 
	* \c [[BREAKIF]] command has been called 
	*/
   bool          broken;
   /**
    * \brief Additional data the Command might need
    */
   void          *userData;
} Statement;

typedef struct tag_template{
	char   *fileName;
	time_t timestamp;
	void   *context;
	Vars   *variables;
	char   *inputFile;
	FILE   *outputFile;
	char   *outputFileName;
	List   *statements;
}Template;

Template *new_Template(void);
Template *Template_compile(char *fileName);
void     Template_execute(Template *template, void *context, char *outputFileName);
void     delete_Template(Template *t);

Statement *Template_findMatching(Template *template, Statement *s);


/* Debugging functions */
void Template_debugPrintTemplate(Template *template);


#endif /* TEMPLATE_H */
