#ifndef _TREE_H
#define _TREE_H

typedef struct _tree{
   struct _tree *parent;
   void         *data;
   struct _tree *left;
   struct _tree *right;
} Tree;

#endif /* _TREE_H */
