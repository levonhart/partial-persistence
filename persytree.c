#include <stdlib.h>
#include "persytree.h"

/* convenience macros: */
#define node_set(NODE, MEMBER, VALUE, TYPE) do { \
	(NODE)->MEMBER = (TYPE)(VALUE); \
	/* TODO */ \
	/* ptrdiff_t _offset = offsetof(node,member) */ \
	/* _p_node_set(node, _offset, value); */ \
} while (0);

#define node_get(NODE, MEMBER, VERSION, TYPE) (TYPE)((NODE)->MEMBER)

void _node_set(node_t * node, ptrdiff_t member, void * value);
/* node_t * _node_get(node_t * node, ptrdiff_t member); */

void rotate_right(node_t * node);
void rotate_left(node_t * node);
void insert_fixup(node_t * node);
void delete_fixup(node_t * node);


persytree_t * persytree_create(){
	persytree_t * new = malloc(sizeof(persytree_t));
	new->n_versions = 1;
	new->root_changes = 0;
	new->root[0] = NULL;
	return new;
}


bool persytree_insert(persytree_t * tree, int key){
	return false;
}


bool persytree_delete(persytree_t * tree, int key){
	return false;
}


node_t * persytree_search(persytree_t * tree, unsigned version, int key){
	return NULL;
}


void rotate_right(node_t * node){
}


void rotate_left(node_t * node){
}


void insert_fixup(node_t * node){
}


void delete_fixup(node_t * node){
}

