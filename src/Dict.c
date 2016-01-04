#include <stdlib.h>
#include <string.h>
#include "Dict.h"
#include "astring.h"
#include "Buffer.h"
#include "Logging.h"
#include "memutils.h"
#include "Pair.h"
#include "stringext.h"


bool keyToIndex(Dict *d, const char *key, size_t *index) {
	bool   retVal = false;
	List   *l = (List *)d;
	Pair   *p = NULL;
	size_t ii;
	
	for (ii = 0; ii < List_length(l); ++ii) {
		p = (Pair *)List_get(l, ii);
		if (strxequals(p->key, key)) {
			*index = ii;
			retVal = true;
			break;
		}
	}
	return retVal;
}

Dict *new_Dict() {
	return (Dict *)new_List();
}


/**
*** \brief Destroys the specified Dict, freeing the memory associated with it
*** \param d The Dict to destroy
*** \param freeData True if the storage for the items in the Dict is to be freed 
*** also
**/
void delete_Dict(Dict *d, bool freeData) {
	List *l = (List *)d;
	Pair *p = NULL;
	
	while (List_length(l) > 0) {
		p = (Pair *)List_get(l, 0);
		mu_free(p->key);
		if (freeData) mu_free(p->value);
		List_remove(l, 0, true);
	}
}


/**
*** \brief Returns the number of items in the Dict
**/
size_t Dict_length(Dict *d) {
	return ((List *)d)->length;
}


/**
*** \brief Returns True if an item with the given key exists in the Dict
**/
bool Dict_exists(Dict *d, const char *key) {
	size_t tmp = 0;
	
	return keyToIndex(d, key, &tmp);
}


/**
*** \brief Stores a value in the dictionary with the specified key.  
***
*** If a value with that key already exists in the dictionary, it will be 
*** replaced with the new value.
*** \note This is a potential memory leak!
**/
bool Dict_put(Dict *d, const char *key, void *value) {
	bool   retVal = false;
	size_t idx = 0;
	Pair   *p  = NULL;
	
	if(keyToIndex(d, key, &idx)){
		p = (Pair *)List_get((List *)d, idx);
		p->value = value;
		retVal = true;
	}
	else{
		p = new_Pair(key, value);
		retVal = List_append((List *)d, p);
	}
	Logging_tracef("++++ Added pointer 0x%x to Dict 0x%x", (unsigned int)value, (unsigned int)d);
	return retVal;
}


/**
*** \brief Stores a value in the dictionary with the specified key, maintaining the 
*** keys in alphabetical order.
*** 
*** If a value with that key already exists in the 
*** dictionary, it will be replaced with the new value 
*** \note This is a potential memory leak!
**/
bool Dict_putSorted(Dict *d, const char *key, void *value) {
	bool   retVal = false;
	bool   added  = false;
	size_t idx    = 0;
	Pair   *p     = NULL;
	Pair   *pp    = NULL;
	size_t ii;
	
	if (keyToIndex(d, key, &idx)) {
		p = (Pair *)List_get((List *)d, idx);
		p->value = value;
		retVal = true;
	}
	else {
		p = new_Pair(key, value);
		added = false;
		if (List_length((List *)d) > 0) {
			for (ii = 0; ii < List_length((List *)d); ++ii) {
				pp = (Pair *)List_get((List *)d, ii);
				if (strcmp(key, pp->key) < 0) {
					List_insert((List *)d, ii, p);
					added = true;
					break;
				}
			}
		}
		if (!added) retVal = List_append((List *)d, p);
	}
	Logging_tracef("++++ Added pointer 0x%x to Dict 0x%x", (unsigned int)value, (unsigned int)d);
	return retVal;
}


void *Dict_get(Dict *d, const char *key) {
	void   *retVal = NULL;
	size_t idx     = 0;
	
	if (keyToIndex(d, key, &idx)) {
		retVal = ((Pair *)List_get((List *)d, idx))->value;
	}
	return retVal;
}


bool Dict_remove(Dict *d, const char *key, bool freeData) {
	bool   retVal = false;
	size_t idx    = 0;
	Pair   *p     = NULL;
	
	if (keyToIndex(d, key, &idx)) {
		p = (Pair *)List_get((List *)d, idx);
		if(freeData) mu_free(p->value);
		mu_free(p->key);
		retVal = List_remove((List *)d, idx, true);
	}
	return retVal;
}


/** Logs the contents of the dictionary to the logger, with the string "prefix" 
*** prepended to each line.  This function assumes all values in the dictionary 
*** are strings.
**/
void Dict_dump(Dict *d, char *prefix) {
	size_t ii;
	List   *l = NULL;
	Pair   *p = NULL;
	char   *v = NULL;
	Buffer *b = NULL;
	char  n[] = "(null)";
	
	if (d != NULL) {
		l = (List *)d;
		b = new_Buffer(0);
		for (ii = 0; ii < List_length(l); ++ii) {
			p = (Pair *)List_get(l, ii);
			v = (char *)p->value;
			if (prefix != NULL) Buffer_appendString(b, prefix);
			Buffer_appendString(b, p->key);
			Buffer_appendString(b, " = ");
			if (v == NULL) {
				Buffer_appendString(b, n);
			}
			else {
				Buffer_appendChar(b, '\"');
				Buffer_appendString(b, v);
				Buffer_appendChar(b, '\"');
			}
			Logging_debugf("%s", b->data);
			Buffer_reset(b);
		}
		delete_Buffer(b);
	}
	else {
		Logging_warnf("%s: NULL argument", __FUNCTION__);
	}
}

