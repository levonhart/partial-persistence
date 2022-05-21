#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "persytree.h"

#define COMMAND_SIZE 4 /* command size (INC, REM, SUC, IMP) */
#define CSZ "3"

int print_tree(node_t * root, unsigned version, FILE * out, unsigned depth){
	int retl, retr, retp;

	if (root == NULL) return 0;

	node_t * left_child = node_get(root, left, ver, node_t*);
	node_t * right_child = node_get(root, right, ver, node_t*);

	if (left_child != NULL) {
		retl = print_tree(left_child, version, out, depth+1); retl++;
		fprintf(out, " ");
	}
	retp=fprintf(out, "%d,%u,%c",
			node_get(root, key, ver, int), depth,
			node_get(root, color, ver, char) == 'r' ? 'R' : 'N');
	if (right_child != NULL) {
		fprintf(out, " ");
		retr = print_tree(right_child, version, out, depth+1); retr++;
	}
	return (retl<0)||(retr<0)||(retp<0)?-1:retl+retp+retr;
}

void parse_file(FILE * file, persytree_t * dest, FILE * out){
	char command[COMMAND_SIZE];
	int value;
	unsigned version;
	while (fscanf(file, "\t%"CSZ"s", command) != EOF) {
		if (strncmp(command, "INC", COMMAND_SIZE) == 0) {
			fscanf(file, "%d", &value);
			persytree_insert(dest, value);
			fprintf(out, "INC %d\n", value);
		} else if (strncmp(command, "REM", COMMAND_SIZE) == 0) {
			fscanf(file, "%d", &value);
			persytree_delete(dest, value);
			fprintf(out, "REM %d\n", value);
		} else if (strncmp(command, "SUC", COMMAND_SIZE) == 0) {
			fscanf(file, "%d %u", &value, &version);
			fprintf(out, "SUC %d %u\n%d\n", value, version,
					persytree_successor(dest, value, version));
		} else if (strncmp(command, "IMP", COMMAND_SIZE) == 0) {
			fscanf(file, "%u", &version);
			fprintf(out, "IMP %u\n", version);
			unsigned ver = dest->last_version < version ? dest->last_version : version;
			node_t * root = dest->root[ver];
			print_tree(root, ver, out, 0);
			fprintf(out, "\n");
		}
	}
}

int main(int argc, char *argv[]){
	if (argc == 1) return EXIT_FAILURE;
	FILE * file = fopen(argv[1], "r");
	if (file == NULL) {
		perror("Invalid argument");
		return EXIT_FAILURE;
	}
	persytree_t *tree = persytree_create();

	char * path, defaultpath[] = "out.txt";
	if (argc > 2) { path = argv[2]; }
	else { path = defaultpath; }
	FILE * output = fopen(path, "w");
	parse_file(file, tree, output);

	printf( "persytree_t size: %zu bytes\n"
			"node_t size:      %zu bytes\n",
			sizeof (*tree), sizeof(node_t));
	free(tree);
	return 0;
}
