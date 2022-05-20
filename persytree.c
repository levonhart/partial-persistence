#include <stdlib.h>
#include "persytree.h"

/* convenience macros: */
#define node_set(NODE, MEMBER, VALUE) do { \
	NODE.MEMBER = VALUE; \
	/* TODO */ \
	/* ptrdiff_t _offset = offsetof(node,member) */ \
	/* _p_node_set(node, _offset, value); */ \
} while (0);

void _node_set(node_t * node, ptrdiff_t member, void * value);



persytree_t * persytree_create(){
	persytree_t * new = malloc(sizeof(persytree_t));
	new->n_versions = 0;
	new->root[0] = NULL;
	return new;
}
