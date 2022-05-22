#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "persytree.h"

/* [> convenience macros: <] */
/* #define node_set(tree, NODE, MEMBER, VALUE, VERSION, TYPE) do { \ */
/*     (NODE)->MEMBER = (TYPE)(VALUE); \ */
/*     [> TODO <] \ */
/*     [> ptrdiff_t _offset = offsetof(node,member) <] \ */
/*     [> _ppersytree_node_set(tree, node, _offset, value); <] \ */
/* } while (0); */
/*  */
/* #define node_get(tree, NODE, MEMBER, VERSION, TYPE) (TYPE)((NODE)->MEMBER) */
/*  */
void persytree_node_set(persytree_t * tree, node_t * node, ptrdiff_t member, void * value, unsigned version, size_t msize);
void * persytree_node_get(persytree_t * tree, node_t * node, ptrdiff_t member, unsigned version);

void rotate_right(persytree_t * tree, node_t * node);
void rotate_left(persytree_t * tree, node_t * node);
void insert_fixup(persytree_t * tree, node_t * node);
void delete_fixup(persytree_t * tree, node_t * node);


persytree_t * persytree_create(){
	persytree_t * new = malloc(sizeof(persytree_t));
	new->last_version = 0;
	new->root[0] = NULL;
	return new;
}


bool persytree_insert(persytree_t * tree, int key){
	unsigned short version = tree->last_version += 1;
	if (version == NVERSIONS) return (--tree->last_version) && false;
	tree->root[version] = tree->root[version-1];
	node_t * new = malloc(sizeof(node_t));
	new->key = key; new->n_mods=0; new->next_version=NULL;
	new->left=NULL; new->right=NULL; new->color='r';
	new->created_at=version;

	node_t * prev=NULL, * iter=tree->root[version];
	while (iter != NULL) {
		prev = iter;
		if (key < node_get(tree, iter, key, version, int)) {
			iter = node_get(tree, iter, left, version, node_t*);
		} else {
			iter = node_get(tree, iter, right, version, node_t*);
		}
	}
	node_set(tree, new, parent, prev, version, node_t*);
	if (prev == NULL) {
		tree->root[version] = new;
	} else if (key < node_get(tree, prev, key, version, int)) {
		node_set(tree, prev, left, new, version, node_t*);
	} else {
		node_set(tree, prev, right, new, version, node_t*);
	}

	insert_fixup(tree, new);
	return true;
}


bool persytree_delete(persytree_t * tree, int key){
	return true;
}


node_t * persytree_search(persytree_t * tree, unsigned version, int key){
	unsigned ver = tree->last_version < version ? tree->last_version : version;
	node_t * iter=tree->root[ver];
	while (iter != NULL) {
		int i = node_get(tree, iter, key, version, int);
		if (key == i) {
			return iter;
		}
		if (key < i) {
			iter = node_get(tree, iter, left, version, node_t*);
		} else {
			iter = node_get(tree, iter, right, version, node_t*);
		}
	}
	return NULL;
}

int persytree_predecessor(persytree_t * tree, unsigned version, int key) {
	node_t * pred=NULL, * prev=NULL, * iter=tree->root[version];
	while (iter != NULL) {
		prev = iter;
		if (key > node_get(tree, iter, key, version, int)) {
			pred = iter;
			iter = node_get(tree, iter, right, version, node_t*);
		} else {
			iter = node_get(tree, iter, left, version, node_t*);
		}
	}
	return pred != NULL ? node_get(tree, pred, key, version, int) : INT_MIN;
}

int persytree_successor(persytree_t * tree, unsigned version, int key) {
	node_t * succ=NULL, * prev=NULL, * iter=tree->root[version];
	while (iter != NULL) {
		prev = iter;
		if (key < node_get(tree, iter, key, version, int)) {
			succ = iter;
			iter = node_get(tree, iter, left, version, node_t*);
		} else {
			iter = node_get(tree, iter, right, version, node_t*);
		}
	}
	return succ != NULL ? node_get(tree, succ, key, version, int) : INT_MAX;
}

int persytree_minimum(persytree_t * tree, unsigned version){
	unsigned ver = tree->last_version < version ? tree->last_version : version;
	node_t * prev=NULL, * iter=tree->root[ver];
	while (iter != NULL) {
		prev = iter;
		iter = node_get(tree, iter, left, ver, node_t*);
	}
	return node_get(tree, prev, key, ver, int);
}

int persytree_maximum(persytree_t * tree, unsigned version){
	unsigned ver = tree->last_version < version ? tree->last_version : version;
	node_t * prev=NULL, * iter=tree->root[ver];
	while (iter != NULL) {
		prev = iter;
		iter = node_get(tree, iter, right, ver, node_t*);
	}
	return node_get(tree, prev, key, ver, int);
}


void rotate_right(persytree_t * tree, node_t * node){
	unsigned version = tree->last_version;
	node_t * node_parent = node_get(tree, node, parent, version, node_t*);
	node_t * left_child = node_get(tree, node, left, version, node_t*);
	node_t * right_grandchild = node_get(tree, left_child, right, version, node_t*);
	node_set(tree, node, left, right_grandchild, version, node_t*);
	if (right_grandchild != NULL) {
		node_set(tree, right_grandchild, parent, node, version, node_t*);
	}
	node_set(tree, left_child, parent, node_parent, version, node_t*);
	if (node_parent == NULL) {
		tree->root[version] = left_child;
	} else {
		node_t * right_sibling = node_get(tree, node_parent, right, version, node_t*);
		if (node == right_sibling) {
			node_set(tree, node_parent, right, left_child, version, node_t*);
		} else {
			node_set(tree, node_parent, left, left_child, version, node_t*);
		}
	}
	node_set(tree, left_child, right, node, version, node_t*);
	node_set(tree, node, parent, left_child, version, node_t*);
}


void rotate_left(persytree_t * tree, node_t * node){
	unsigned version = tree->last_version;
	node_t * node_parent = node_get(tree, node, parent, version, node_t*);
	node_t * right_child = node_get(tree, node, right, version, node_t*);
	node_t * left_grandchild = node_get(tree, right_child, left, version, node_t*);
	node_set(tree, node, right, left_grandchild, version, node_t*);
	if (left_grandchild != NULL) {
		node_set(tree, left_grandchild, parent, node, version, node_t*);
	}
	node_set(tree, right_child, parent, node_parent, version, node_t*);
	if (node_parent == NULL) {
		tree->root[version] = right_child;
	} else {
		node_t * left_sibling = node_get(tree, node_parent, left, version, node_t*);
		if (node == left_sibling) {
			node_set(tree, node_parent, left, right_child, version, node_t*);
		} else {
			node_set(tree, node_parent, right, right_child, version, node_t*);
		}
	}
	node_set(tree, right_child, left, node, version, node_t*);
	node_set(tree, node, parent, right_child, version, node_t*);
}

void insert_fixup(persytree_t * tree, node_t * node){
	unsigned version = tree->last_version;
	node_t * iter = node_get(tree, node, parent, version, node_t*), * temp;
	while (	iter != NULL && node_get(tree, iter, color, version, char) == 'r') {
		node_t * par = node_get(tree, iter, parent, version, node_t*);
		node_t * left_sibling = node_get(tree, par, left, version, node_t*);
		if (iter == left_sibling) {
			node_t * right_sibling = node_get(tree, par, right, version, node_t*);
			if (right_sibling !=NULL &&
				  node_get(tree, right_sibling, color, version, char) == 'r') {
				node_set(tree, iter, color, 'b', version, char)
				node_set(tree, right_sibling, color, 'b', version, char)
				node_set(tree, par, color, 'r', version, char)
				node = par;
				iter = node_get(tree, node, parent, version, node_t*);
			} else {
				if (node == node_get(tree, iter, right, version, node_t*)) {
					rotate_left(tree, iter);
					temp = node; node = iter; iter = temp;
				}
				node_set(tree, iter, color, 'b', version, char);
				node_set(tree, par, color, 'r', version, char);
				rotate_right(tree, par);
			}
		} else {
			if (left_sibling != NULL &&
				  node_get(tree, left_sibling, color, version, char) == 'r') {
				node_set(tree, iter, color, 'b', version, char)
				node_set(tree, left_sibling, color, 'b', version, char)
				node_set(tree, par, color, 'r', version, char)
				node = par;
				iter = node_get(tree, node, parent, version, node_t*);
			} else {
				if (node == node_get(tree, iter, left, version, node_t*)) {
					rotate_right(tree, iter);
					temp = node; node = iter; iter = temp;
				}
				node_set(tree, iter, color, 'b', version, char);
				node_set(tree, par, color, 'r', version, char);
				rotate_left(tree, par);
			}
		}
	}
	node_set(tree, tree->root[version], color, 'b', version, char);
}


void delete_fixup(persytree_t * tree, node_t * node){
	unsigned version = tree->last_version;
}


void persytree_node_set(persytree_t * tree, node_t * node, ptrdiff_t member,
	                    void * value, unsigned version, size_t msize){
	void * field;
	while (node->next_version != NULL) { /** this avoids loosing  */
		node = node->next_version;        /* track of the last    */
	}									  /* version of the node **/
	if (node->created_at == version) {
		memcpy(((char *) node) + member, value, msize);
	} else {
		unsigned  i = 0;
		while(i < node->n_mods){
			if(node->mods[i].member == member &&
					node->mods[i].version == version) {
				field = &(node->mods[i].ival);
				memcpy(field, value, msize);
				return;
			}
			i++;
		}
		assert(i==node->n_mods);
		if (i < NMODS) {
			node->n_mods += 1;
			node->mods[i].member = member;
			node->mods[i].version = version;
			node->mods[i].msize = msize;
			field = &(node->mods[i].ival);
			memcpy(field, value, msize);
		} else {
			/* create new node. new node starts likes old */
			node_t * new = malloc(sizeof(node_t));
			new->key = node->key;
			new->color = node->color;
			new->left = node->left;
			new->right = node->right;
			new->parent = node->parent;
			new->n_mods=0;
			new->created_at = version;
			new->next_version = NULL;
			node->next_version = new;

			for (i = 0; i < node->n_mods; i++) {
				void * field, * mod_value;
				short field_size;
				field = ((char *) new) + node->mods[i].member;
				mod_value = &(node->mods[i].ival);
				field_size = node->mods[i].msize;
				memcpy(field, mod_value, field_size);
			}

			memcpy(((char *) new) + member, value, msize);

			/* modify pointers recursively */
			node_t * left = new->left,
				   * right = new->right,
				   * parent = new->parent;
			if (left != NULL) {
				persytree_node_set(tree, left, offsetof(node_t,parent),
								   &new, version, sizeof(node_t*));
			}
			if (right != NULL) {
				persytree_node_set(tree, right, offsetof(node_t,parent),
								   &new, version, sizeof(node_t*));
			}
			if (parent != NULL) {
				if (node == node_get(tree, parent, left, version, node_t*)) {
					persytree_node_set(tree, parent, offsetof(node_t,left),
								   &new, version, sizeof(node_t*));
				} else {
					persytree_node_set(tree, parent, offsetof(node_t,right),
								   &new, version, sizeof(node_t*));
				}
			} else {
				tree->root[version] = new;
			}

		}
	}
}

void * persytree_node_get(persytree_t * tree, node_t * node, ptrdiff_t member, unsigned version) {
	assert(node != NULL);
	while (node->next_version != NULL &&
			node->next_version->created_at <= version) {
		node = node->next_version;
	}
	void * field = ((char *) node) + member;
	unsigned i;
	for (i = 0; i < node->n_mods; i++){
		if(node->mods[i].member == member &&
				node->mods[i].version <= version) {
			field = &(node->mods[i].ival);
		}
	}
	return field;
}

