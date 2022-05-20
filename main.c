#include <stdlib.h>
#include <stdio.h>
#include "persytree.h"

int main(int argc, char *argv[]){
	persytree_t *tree = persytree_create();
	node_t node;
	tree->n_versions = 1;
	tree->root[0] = &node;

	printf( "persytree_t size: %zu bytes\n"
			"node_t size:      %zu bytes\n",
			sizeof (*tree), sizeof node);
	free(tree);
	return 0;
}
