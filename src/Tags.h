#ifndef TAGS_H
#define TAGS_H
#include "bool.h"
#include "BileObj.h"

Tags *new_Tags(Publication *parent, const char *name);
bool Tags_add(Tags *t, Story *st);

#endif /* TAGS_H */


