#include <limits.h>
#include <stdlib.h>
#include "persytree.h"

/* [> convenience macros: <] */
/* #define node_set(NODE, MEMBER, VALUE, VERSION, TYPE) do { \ */
/*     (NODE)->MEMBER = (TYPE)(VALUE); \ */
/*     [> TODO <] \ */
/*     [> ptrdiff_t _offset = offsetof(node,member) <] \ */
/*     [> _p_node_set(node, _offset, value); <] \ */
/* } while (0); */
/*  */
/* #define node_get(NODE, MEMBER, VERSION, TYPE) (TYPE)((NODE)->MEMBER) */
/*  */
/* [> void _node_set(node_t * node, ptrdiff_t member, void * value, unsigned version, size_t msize); <] */
/* void * _node_get(node_t * node, ptrdiff_t member, unsigned version); */

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
	new->left = NULL; new->right=NULL; new->color='r';

	node_t * prev=NULL, * iter=tree->root[version];
	while (iter != NULL) {
		prev = iter;
		if (key < node_get(iter, key, version, int)) {
			iter = node_get(iter, left, version, node_t*);
		} else {
			iter = node_get(iter, right, version, node_t*);
		}
	}
	node_set(new, parent, prev, version, node_t*);
	if (prev == NULL) {
		tree->root[version] = new;
	} else if (key < node_get(prev, key, version, int)) {
		node_set(prev, left, new, version, node_t*);
	} else {
		node_set(prev, right, new, version, node_t*);
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

	y = z;
	char y_original_color = node_get(y, color, version, char);

	if(node_get(z, left, version, node_t*) == NULL){
		x = node_get(z, right, version, node_t*);
		transplant(tree, z, x);
	
	} else if(node_get(z, right, version, node_t*) == NULL){
		x = node_get(z, right, version, node_t*);
		transplant(tree, z, x);

	} else {
		node_t* right_z = node_get(z, right, version, node_t*);
		y = persytree_minimum(tree, version, right_z);
		
		y_original_color = node_get(y, color, version, char);
		x = node_get(y, right, version, node_t*);

		if(node_get(y, parent, version, node_t*) == z)
			node_set(x, parent, y, version, node_t*)
		else{
			node_t* y_right = node_get(y, right, version, node_t*);
			transplant(tree, y, y_right);
			node_t* z_right = node_get(z, right, version, node_t*);
			node_set(y, right, z_right, version, node_t*);
			node_set(y_right, parent, y, version, node_t*);

		}
		transplant(tree, z, y);
		node_t* z_left = node_get(z, left, version, node_t*);
		node_set(y, left, z_left, version, node_t*);

		node_t* y_left = node_get(y, left, version, node_t*);
		node_set(y_left, parent, y, version, node_t*);

		char z_color = node_get(z, color, version, char);
		node_set(y, color, z_color, version, char);

		if(y_original_color == 'b')
			delete_fixup(tree, x);

	}


	return true;
}


node_t * persytree_search(persytree_t * tree, unsigned version, int key){
	unsigned ver = tree->last_version < version ? tree->last_version : version;
	node_t * iter=tree->root[ver];
	while (iter != NULL) {
		int i = node_get(iter, key, ver, int);
		if (key == i) {
			return iter;
		}
		if (key < i) {
			iter = node_get(iter, left, ver, node_t*);
		} else {
			iter = node_get(iter, right, ver, node_t*);
		}
	}
	return NULL;
}

int persytree_predecessor(persytree_t * tree, unsigned version, int key) {
	node_t * pred=NULL, * prev=NULL, * iter=tree->root[version];
	while (iter != NULL) {
		prev = iter;
		if (key > node_get(iter, key, version, int)) {
			pred = iter;
			iter = node_get(iter, right, version, node_t*);
		} else {
			iter = node_get(iter, left, version, node_t*);
		}
	}
	return pred != NULL ? node_get(pred, key, version, int) : INT_MIN;
}

int persytree_successor(persytree_t * tree, unsigned version, int key) {
	node_t * succ=NULL, * prev=NULL, * iter=tree->root[version];
	while (iter != NULL) {
		prev = iter;
		if (key < node_get(iter, key, version, int)) {
			succ = iter;
			iter = node_get(iter, left, version, node_t*);
		} else {
			iter = node_get(iter, right, version, node_t*);
		}
	}
	return succ != NULL ? node_get(succ, key, ver, int) : INT_MAX;
}

node_t* persytree_minimum(persytree_t * tree, unsigned version, node_t* root){
	unsigned ver = tree->last_version < version ? tree->last_version : version;

	node_t * prev=NULL;
	node_t * iter = root == NULL ? tree->root[ver] : root;
	while (iter != NULL) {
		prev = iter;
		iter = node_get(iter, left, ver, node_t*);
	}
	return prev;
}

node_t* persytree_maximum(persytree_t * tree, unsigned version, node_t* root){
	unsigned ver = tree->last_version < version ? tree->last_version : version;

	node_t * prev=NULL;
	node_t * iter = root == NULL ? tree->root[ver] : root;
	while (iter != NULL) {
		prev = iter;
		iter = node_get(iter, right, ver, node_t*);
	}
	return prev;
}


void rotate_right(persytree_t * tree, node_t * node){
	unsigned version = tree->last_version;
	node_t * node_parent = node_get(node, parent, version, node_t*);
	node_t * left_child = node_get(node, left, version, node_t*);
	node_t * right_grandchild = node_get(left_child, right, version, node_t*);
	node_set(node, left, right_grandchild, version, node_t*);
	if (right_grandchild != NULL) {
		node_set(right_grandchild, parent, node, version, node_t*);
	}
	node_set(left_child, parent, node_parent, version, node_t*);
	if (node_parent == NULL) {
		tree->root[version] = left_child;
	} else {
		node_t * right_sibling = node_get(node_parent, right, version, node_t*);
		if (node == right_sibling) {
			node_set(node_parent, right, left_child, version, node_t*);
		} else {
			node_set(node_parent, left, left_child, version, node_t*);
		}
	}
	node_set(left_child, right, node, version, node_t*);
	node_set(node, parent, left_child, version, node_t*);
}


void rotate_left(persytree_t * tree, node_t * node){
	unsigned version = tree->last_version;
	node_t * node_parent = node_get(node, parent, version, node_t*);
	node_t * right_child = node_get(node, right, version, node_t*);
	node_t * left_grandchild = node_get(right_child, left, version, node_t*);
	node_set(node, right, left_grandchild, version, node_t*);
	if (left_grandchild != NULL) {
		node_set(left_grandchild, parent, node, version, node_t*);
	}
	node_set(right_child, parent, node_parent, version, node_t*);
	if (node_parent == NULL) {
		tree->root[version] = right_child;
	} else {
		node_t * left_sibling = node_get(node_parent, left, version, node_t*);
		if (node == left_sibling) {
			node_set(node_parent, left, right_child, version, node_t*);
		} else {
			node_set(node_parent, right, right_child, version, node_t*);
		}
	}
	node_set(right_child, left, node, version, node_t*);
	node_set(node, parent, right_child, version, node_t*);
}

#include <stdio.h>

void insert_fixup(persytree_t * tree, node_t * node){
	unsigned version = tree->last_version;
	node_t * iter = node_get(node, parent, version, node_t*), * temp;
	while (	iter != NULL && node_get(iter, color, version, char) == 'r') {
		node_t * par = node_get(iter, parent, version, node_t*);
		node_t * left_sibling = node_get(par, left, version, node_t*);
		if (iter == left_sibling) {
			node_t * right_sibling = node_get(par, right, version, node_t*);
			if (right_sibling !=NULL &&
				  node_get(right_sibling, color, version, char) == 'r') {
				node_set(iter, color, 'b', version, char)
				node_set(right_sibling, color, 'b', version, char)
				node_set(par, color, 'r', version, char)
				node = par;
				iter = node_get(node, parent, version, node_t*);
			} else {
				if (node == node_get(iter, right, version, node_t*)) {
					rotate_left(tree, iter);
					temp = node; node = iter; iter = temp;
				}
				node_set(iter, color, 'b', version, char);
				node_set(par, color, 'r', version, char);
				rotate_right(tree, par);
			}
		} else {
			if (left_sibling != NULL &&
				  node_get(left_sibling, color, version, char) == 'r') {
				node_set(iter, color, 'b', version, char)
				node_set(left_sibling, color, 'b', version, char)
				node_set(par, color, 'r', version, char)
				node = par;
				iter = node_get(node, parent, version, node_t*);
			} else {
				if (node == node_get(iter, left, version, node_t*)) {
					rotate_right(tree, iter);
					temp = node; node = iter; iter = temp;
				}
				node_set(iter, color, 'b', version, char);
				node_set(par, color, 'r', version, char);
				rotate_left(tree, par);
			}
		}
	}
	node_set(tree->root[version], color, 'b', version, char);
}


void delete_fixup(persytree_t * tree, node_t * node){
	unsigned version = tree->last_version;
	node_t* w, *left_w, *right_w;
	node_t* node_parent = node_get(node, parent, version, node_t*);
	
	while(node != tree->root[version] && node_get(node, color, version, char) == 'b'){
		if(node_parent != NULL && node == node_get(node_parent, left, version, node_t*)){
			w = node_get(node_parent, right, version, node_t*);
			if(w != NULL && node_get(w, color, version, char) == 'r'){
				node_set(w, color, 'r', version, char);
				rotate_left(tree, node_parent);
				w =  node_get(node_parent, right, version, node_t*);
			}
			left_w = node_get(w, left, version, node_t*);
			right_w = node_get(w, right, version, node_t*);
			if((left_w == NULL || node_get(left_w, color, version, char) == 'b') 
				&& (right_w == NULL || node_get(right_w, color, version, char) == 'b')){
				node_set(w, color, 'r', version, char);
				node = node_parent;
				node_parent = node_get(node, parent, version, node_t*);
			} else {
				if(right_w == NULL || node_get(right_w, color, version, char) == 'b'){
					node_set(left_w, color, 'b', version, char);
					node_set(w, color, 'r', version, char);
					rotate_right(tree, w);
					
					w = node_get(node_parent, right, version, node_t*);

				}
				char node_parent_color = node_get(node_parent, color, version, char);
				node_set(w, color, node_parent_color, version, char);
				node_set(node_parent, color, 'b', version, char);
				right_w = node_get(w, right, version, node_t*);
				node_set(right_w, color, 'b', version, char);
				rotate_left(tree, node_parent);
				node = tree->root[version];
				node_parent = NULL;
			}

		} else {
			w = node_get(node_parent, left, version, node_t*);
			if(w != NULL && node_get(w, color, version, char) == 'r'){
				node_set(w, color, 'r', version, char);
				rotate_right(tree, node_parent);
				w =  node_get(node_parent, left, version, node_t*);
			}
			right_w = node_get(w, right, version, node_t*);
			left_w = node_get(w, left, version, node_t*);
			if((right_w == NULL || node_get(right_w, color, version, char) == 'b') 
				&& (left_w == NULL || node_get(left_w, color, version, char) == 'b')){
				node_set(w, color, 'r', version, char);
				node = node_parent;
				node_parent = node_get(node, parent, version, node_t*);
			} else {
				if(left_w == NULL || node_get(left_w, color, version, char) == 'b'){
					node_set(right_w, color, 'b', version, char);
					node_set(w, color, 'r', version, char);
					rotate_left(tree, w);
					
					w = node_get(node_parent, left, version, node_t*);

				}
				char node_parent_color = node_get(node_parent, color, version, char);
				node_set(w, color, node_parent_color, version, char);
				node_set(node_parent, color, 'b', version, char);
				left_w = node_get(w, left, version, node_t*);
				node_set(left_w, color, 'b', version, char);
				rotate_right(tree, node_parent);
				node = tree->root[version];
				node_parent = NULL;
			}
		}
	

	}


}

void transplant(persytree_t * tree, node_t * u, node_t * v){
	unsigned version = tree->last_version;
	node_t* u_parent = node_get(u, parent, version, node_t*);
	if(u_parent == NULL){
		tree->root[version] = v;
	} else if(u == node_get(u_parent, left, version, node_t*)){
		node_set(u_parent, left, v, version, node_t*);		
	} else node_set(u_parent, right, v, version, node_t*);
	node_set(v, parent, u_parent, version, node_t*);

}

