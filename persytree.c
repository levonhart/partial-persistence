#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
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
void transplant(persytree_t * tree, node_t * u, node_t * v);


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
	unsigned short version = tree->last_version += 1;
	if (version == NVERSIONS) return (--tree->last_version) && false;
	tree->root[version] = tree->root[version-1];


	node_t* x, *y, *z;
	z = persytree_search(tree, version, key);

	if(z == NULL) return false;

	if(node_get(tree, z, left, version, node_t*) == NULL ||
		node_get(tree, z, right, version, node_t*) == NULL)
		y = z;
	else
		y = persytree_successor(tree, version, key);


	if(node_get(tree, y, left, version, node_t*) != NULL)
		x = node_get(tree, y, left, version, node_t*);
	else
		x = node_get(tree, y, right, version, node_t*);

	node_t* y_parent = node_get(tree, y, parent, version, node_t*);
	if(x != NULL)
		node_set(tree, x, parent, y_parent, version, node_t*);

	if(y_parent == NULL) {
		tree->root[version] = x;
		return true;
	} else if(y == node_get(tree, y_parent, left, version, node_t*)){
			node_set(tree, y_parent, left, x, version, node_t*);
	} else node_set(tree, y_parent, right, x, version, node_t*);

	if(y != z){
		int y_key = node_get(tree, y, key, version, int);
		node_set(tree, z, key, y_key, version, int);
		// copy data to z
	}

	if(node_get(tree, y, color, version, char) == 'b'){

		if(x != NULL){
			delete_fixup(tree, x);

			return true;
		}
		node_t nil = {.parent=y_parent, .color='b', .n_mods=0,
		              .next_version=NULL, .created_at=version };
		delete_fixup(tree, &nil);

	}



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

node_t* persytree_predecessor(persytree_t * tree, unsigned version, int key) {
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
	return pred;
}

node_t* persytree_successor(persytree_t * tree, unsigned version, int key) {
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
	return succ;
}

node_t* persytree_minimum(persytree_t * tree, unsigned version, node_t* root){
	unsigned ver = tree->last_version < version ? tree->last_version : version;

	node_t * prev=NULL;
	node_t * iter = root == NULL ? tree->root[ver] : root;
	while (iter != NULL) {
		prev = iter;
		iter = node_get(tree, iter, left, ver, node_t*);
	}
	return prev;
}

node_t* persytree_maximum(persytree_t * tree, unsigned version, node_t* root){
	unsigned ver = tree->last_version < version ? tree->last_version : version;

	node_t * prev=NULL;
	node_t * iter = root == NULL ? tree->root[ver] : root;
	while (iter != NULL) {
		prev = iter;
		iter = node_get(tree, iter, right, ver, node_t*);
	}
	return prev;
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
	node_t* w, *left_w, *right_w;
	// TODO: CASO EM Q node É NIL(tree)
	if(node == NULL)
		return;


	while(node != NULL && node != tree->root[version] && node_get(tree, node, color, version, char) == 'b'){
		node_t* node_parent;

		node_parent = node_get(tree, node, parent, version, node_t*);

		if(node == node_get(tree, node_parent, left, version, node_t*)){
			w = node_get(tree, node_parent, right, version, node_t*);
			if (w == NULL) break;
			// CASE 1
			if(w != NULL && node_get(tree, w, color, version, char) == 'r'){
				node_set(tree, w, color, 'b', version, char);
				node_set(tree, node_parent, color, 'r', version, char);
				rotate_left(tree, node_parent);
				w =  node_get(tree, node_parent, right, version, node_t*);
			}

			left_w = NULL; right_w = NULL;
			if(w != NULL){
				right_w = node_get(tree, w, right, version, node_t*);
				left_w = node_get(tree, w, left, version, node_t*);
			}

			// CASE 2
			if((left_w == NULL || node_get(tree, left_w, color, version, char) == 'b') 
				&& (right_w == NULL || node_get(tree, right_w, color, version, char) == 'b')){

				node_set(tree, w, color, 'r', version, char);
				node = node_parent;


			// CASE 3
			} else {
				if(right_w == NULL || node_get(tree, right_w, color, version, char) == 'b'){

					node_set(tree, left_w, color, 'b', version, char);

					node_set(tree, w, color, 'r', version, char);

					rotate_right(tree, w);

					w = node_get(tree, node_parent, right, version, node_t*);

				}
				// CASE 4
				char node_parent_color = node_get(tree, node_parent, color, version, char);

				node_set(tree, w, color, node_parent_color, version, char);

				node_set(tree, node_parent, color, 'b', version, char);

				right_w = node_get(tree, w, right, version, node_t*);

				node_set(tree, right_w, color, 'b', version, char);


				rotate_left(tree, node_parent);


				node = tree->root[version];
			}

		}
		else {
			w = node_get(tree, node_parent, left, version, node_t*);
			if (w == NULL) break;
			// CASE 1
			if(w != NULL && node_get(tree, w, color, version, char) == 'r'){
				node_set(tree, w, color, 'b', version, char);
				node_set(tree, node_parent, color, 'r', version, char);
				rotate_right(tree, node_parent);
				w =  node_get(tree, node_parent, left, version, node_t*);
			}

			left_w = NULL; right_w = NULL;
			if(w != NULL){
				right_w = node_get(tree, w, right, version, node_t*);
				left_w = node_get(tree, w, left, version, node_t*);
			}

			// CASE 2
			if((right_w == NULL || node_get(tree, right_w, color, version, char) == 'b') 
				&& (left_w == NULL || node_get(tree, left_w, color, version, char) == 'b')){

				node_set(tree, w, color, 'r', version, char);
				node = node_parent;

			} else {
				// CASE 3
				if(left_w == NULL || node_get(tree, left_w, color, version, char) == 'b'){

					node_set(tree, right_w, color, 'b', version, char);


					node_set(tree, w, color, 'r', version, char);

					rotate_left(tree, w);

					w = node_get(tree, node_parent, left, version, node_t*);

				}
				// CASE 4
				char node_parent_color = node_get(tree, node_parent, color, version, char);

				node_set(tree, w, color, node_parent_color, version, char);

				node_set(tree, node_parent, color, 'b', version, char);

				left_w = node_get(tree, w, left, version, node_t*);

				node_set(tree, left_w, color, 'b', version, char);

				rotate_right(tree, node_parent);


				node = tree->root[version];
			}

		}
	}


	if(node != NULL){
		node_set(tree, node, color, 'b', version, char);
	}

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
			new->created_at=version;
			new->next_version=NULL;
			node->next_version = new;

			for (i = 0; i < node->n_mods; i++) {
				void * field = ((char *) new) + node->mods[i].member;
				void * mod_value = &(node->mods[i].ival);
				size_t field_size = node->mods[i].msize;
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
	unsigned i;
	while (node->next_version != NULL &&
			node->next_version->created_at <= version) {
		node = node->next_version;
		i++;
	}
	void * field = ((char *) node) + member;
	for (i = 0; i < node->n_mods; i++){
		if(node->mods[i].member == member &&
				node->mods[i].version <= version) {
			field = &(node->mods[i].ival);
		}
	}
	return field;
}

