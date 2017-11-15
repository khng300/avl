#include "avl.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define offset_of(type, field) ((size_t)(&((type *)0)->field))
#define node_of(ptr, type, field) ((type *)((char *)(ptr) - offset_of(type, field)))

avl_root_t avlroot;

struct int_node {
	int key;
	avl_node_t node;
};

static int
avl_cmp(avl_node_t *a, avl_node_t *b)
{
	struct int_node *a1, *b1;
	
	a1 = node_of(a, struct int_node, node);
	b1 = node_of(b, struct int_node, node);
	
	if (a1->key < b1->key)
		return -1;
	else if (a1->key > b1->key)
		return 1;
	return 0;
}

void
avl_dump_tree(struct int_node *node, int depth)
{
	int whichchild;

	whichchild = avl_which_child(&node->node);
	printf("%*c|- key:%d balance:%d whichchild:%d\n", depth, ' ', node->key, node->node.avl_balance, whichchild);
	fflush(stdout);
	
	if (node->node.avl_children[0])
		avl_dump_tree(node_of(node->node.avl_children[0], struct int_node, node), depth + 1);
	if (node->node.avl_children[1])
		avl_dump_tree(node_of(node->node.avl_children[1], struct int_node, node), depth + 1);
}

int
avl_check(struct int_node *node, int depth)
{
	int height_diff;
	int rheight = 0, lheight = 0;
	int whichchild;
	
	if (node->node.avl_children[0])
		lheight = avl_check(node_of(node->node.avl_children[0], struct int_node, node), depth + 1) + 1;
	if (node->node.avl_children[1])
		rheight = avl_check(node_of(node->node.avl_children[1], struct int_node, node), depth + 1) + 1;
	
	height_diff = rheight - lheight;
	if (height_diff != node->node.avl_balance) {
		printf("Incorrect tree!!!\n");
		fflush(stdout);
		assert(0);
	}
	if (avl_abs_balance(height_diff) >= 2) {
		printf("Incorrect tree 2!!!\n");
		fflush(stdout);
		assert(0);
	}
	
	if (rheight < lheight)
		return lheight;
	return rheight;
}

#define COUNT 200

int
main()
{
	int i;
	struct int_node *nodes = malloc(sizeof(struct int_node) * COUNT);
	
	for (i = 0; i < COUNT; ++i) {
		avl_node_t *ptr;

		printf("id: %d\n", i);
		if (avlroot.avl_root) {
			avl_dump_tree(node_of(avlroot.avl_root, struct int_node, node), 1);
			avl_check(node_of(avlroot.avl_root, struct int_node, node), 0);
		}
		do {
			nodes[i].key = rand();
			ptr = avl_insert(&avlroot, &nodes[i].node, avl_cmp);
		} while (ptr != &nodes[i].node);
		printf(" Inserted: %d\n", nodes[i].key);
		if (avlroot.avl_root) {
			avl_dump_tree(node_of(avlroot.avl_root, struct int_node, node), 1);
			avl_check(node_of(avlroot.avl_root, struct int_node, node), 0);
		}
	}

	for (i = 0; i < COUNT; ++i) {
		printf(" Deleting: %d\n", nodes[i].key);
		avl_remove(&avlroot, &nodes[i].node);
		if (avlroot.avl_root) {
			avl_dump_tree(node_of(avlroot.avl_root, struct int_node, node), 1);
			avl_check(node_of(avlroot.avl_root, struct int_node, node), 0);
		}
	}
	
	free(nodes);
	return 0;
}
