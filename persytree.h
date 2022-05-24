#ifndef PERSYTREE_H
#define PERSYTREE_H

#include <stddef.h>
#include <limits.h>
#include <stdbool.h>

#define NMODS 1
#define NVERSIONS 101 /* maximum number of safe inclusions/deletions */


/*! \struct persytree_t
 *  \brief RB-tree with partial persistence.
 *
 *  This structure implements a red-black tree that stores every
 *  changes made to its content.
 */
typedef struct persytree_t{
	unsigned int last_version;
	struct node_t * root[NVERSIONS];
} persytree_t;

typedef struct mod_t {
	ptrdiff_t member;
	short msize;
	union {
		int ival;
		struct node_t * pval;
	};
	unsigned int version;
} mod_t;

typedef struct node_t {
	int key;
	char color;
	struct node_t * left,
				  * right,
				  * parent;
	unsigned short n_mods;
	unsigned created_at;
	struct mod_t mods[NMODS];
	struct node_t * next_version;
} node_t;

persytree_t * persytree_create();
/* void          persytree_destroy(persytree_t * tree); */
bool          persytree_insert(persytree_t * tree, int key);
bool          persytree_delete(persytree_t * tree, int key);
node_t *      persytree_search(persytree_t * tree, unsigned version, int key);
node_t *      persytree_predecessor(persytree_t * tree, unsigned version, int key);
node_t *      persytree_successor(persytree_t * tree, unsigned version, int key);
node_t *      persytree_minimum(persytree_t * tree, unsigned version, node_t* root);
node_t *      persytree_minimum(persytree_t * tree, unsigned version, node_t* root);


void persytree_node_set(persytree_t * tree, node_t ** nodeptr,
		ptrdiff_t member, void * value, unsigned version,
		size_t msize);
void * persytree_node_get(persytree_t * tree, node_t * node,
		ptrdiff_t member, unsigned version);

#include <string.h>
/* convenience macros: */
#define node_set(TREE, NODEPTR, MEMBER, VALUE, VERSION, TYPE) do { \
	TYPE _new_value = (VALUE); \
	persytree_node_set((TREE), (NODEPTR), offsetof(node_t,MEMBER), &_new_value, (VERSION), sizeof(TYPE)); \
} while (0);
// #define node_set(TREE, NODE, MEMBER, VALUE, VERSION, TYPE) \
//     (NODE)->MEMBER = (TYPE) VALUE;

#define node_get(TREE, NODE, MEMBER, VERSION, TYPE) \
	(*((TYPE *) persytree_node_get((TREE),(NODE),offsetof(node_t,MEMBER), (VERSION))))

// #define node_get(TREE, NODE, MEMBER, VERSION, TYPE) \
//     ((TYPE) ((NODE)->MEMBER))

#endif /* end of include guard: PERSYTREE_H */
