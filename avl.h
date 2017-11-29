/*
 * Copyright 2017 Ka Ho Ng <ngkaho1234@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef __AVL_H__
#define __AVL_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Intrusive node structure embedded as data members.
 *
 * For balance factor, a negative integer indicates the subtree rooted at the current
 * node is left-heavy, on the other hand a positive integer indicates the subtree is
 * right-heavy. Zero balance factor indicates the subtree is in perfect balance.
 */
typedef struct avl_node_s {
	struct avl_node_s *avl_children[2];	/* Pointers to left child and right child */
	struct avl_node_s *avl_parent;		/* Pointer to parent node */
	int avl_balance;			/* Balance factor */
} avl_node_t;

/*
 * A root structure that holds the whole AVL tree.
 */
typedef struct avl_root_s {
	avl_node_t *avl_root;			/* Pointer to the root node */
} avl_root_t;

/*
 * AVL comparsion routine provided by user
 */
typedef int avl_cmp_t(avl_node_t *a, avl_node_t *b);

/*
 * Determine whether this node is the left or right child of
 * its parent.
 * Return zero if the node is the left node, one if the node is the right node,
 * otherwise -1 (when the node is already the root)
 */
static inline int avl_which_child(avl_node_t *node)
{
	avl_node_t *parent;

	parent = node->avl_parent;
	if (!parent)
		return -1;

	if (node == parent->avl_children[0])
		return 0;
	return 1;
}

/*
 * Determine whether the subtree rooted at this node is left-heavy or right-heavy
 */
static inline int avl_abs_balance(int balance)
{
	if (balance < 0)
		return balance * -1;
	return balance;
}

avl_node_t *
avl_search(avl_root_t *avlroot, avl_node_t *key, avl_cmp_t *cmpfunc);

avl_node_t *
avl_insert(avl_root_t *avlroot, avl_node_t *node, avl_cmp_t *cmpfunc);

void
avl_remove(avl_root_t *avlroot, avl_node_t *node);

avl_node_t *
avl_first(avl_root_t *root);

avl_node_t *
avl_last(avl_root_t *root);

avl_node_t *
avl_prev(avl_node_t *node);

avl_node_t *
avl_next(avl_node_t *node);


#ifdef __cplusplus
}
#endif

#endif /* __AVL_H__ */
