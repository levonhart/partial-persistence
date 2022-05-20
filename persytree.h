#ifndef PERSYTREE_H
#define PERSYTREE_H

#include <stddef.h>
#include <stdbool.h>

#define NMODS 3
#define NVERSIONS 100 /* maximum number of safe inclusions/deletions */


/*! \struct persytree_t
 *  \brief RB-tree with partial persistence.
 *
 *  This structure implements a red-black tree that stores every
 *  changes made to its content.
 */
typedef struct persytree_t{
	unsigned int n_versions;
	unsigned short root_changes;
	struct node_t * root[NVERSIONS];
} persytree_t;

struct mod_t {
	ptrdiff_t member;
	union {
		int ival;
		struct node_t * pval;
	};
};

typedef struct node_t {
	int key;
	struct node_t * left,
				  * right,
				  * parent;
	unsigned short n_mods;
	struct mod_t mods[NMODS];
	struct node_t * next_version;
} node_t;

persytree_t * persytree_create();
/* void          persytree_destroy(persytree_t * tree); */
bool          persytree_insert(persytree_t * tree, int key);
bool          persytree_delete(persytree_t * tree, int key);
node_t *      persytree_search(persytree_t * tree, unsigned version, int key);
/* node_t *      persytree_predecessor(persytree_t * tree, unsigned version, int key); */
/* node_t *      persytree_successor(persytree_t * tree, unsigned version, int key); */


#endif /* end of include guard: PERSYTREE_H */
