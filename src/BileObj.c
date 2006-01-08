/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: BileObj.c,v 1.1 2006/01/08 18:21:07 ken Exp $
 */
#include <string.h>
#include <unistd.h>
#include "Logging.h"
#include "memutils.h"
#include "TextFile.h"

Publication *new_Publication(char *configFileName){
	Publication *p    = NULL;
	TextFile    *t    = NULL;
	char        *line = NULL;
	
	p = mu_malloc(sizeof(Publication));
	p->variables = new_Vars();
	p->sections  = new_List();
	p->indexes   = new_List();
	p->stories   = new_List();
	
	if(configFileName != NULL){
		if(access(configFileName, R_OK) == 0){
			t = new_TextFile(configFileName);
			while((line = TextFile_readLine(t)) != NULL){
			}
			delete_TextFile(t);
		}
		else{
			Logging_fatalf("%s: Unable to open configuration file '%s': %s",
				__FUNCTION__, configFileName, strerror(errno));
		}
	}
	return p;
}

void Publication_dump(Publication *p){
	if(p != NULL){
	}
	else
		Logging_warnf("%s: NULL argument", __FUNCTION__);
}
