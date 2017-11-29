/*
 * Copyright 2017 Ka Ho Ng <khng300@gmail.com>
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

#include "avl.h"

#define avl_cmp2idx(cmp) (!((cmp) < 0))
#define avl_balance2idx(cmp) (!((cmp) < 0))
#define avl_idx2cmp(idx) (!(idx)?-1:1)

/*
 * Generic search routine for AVL tree
 *
 * The key parameter needs not to be a valid AVL node, we only use it as an
 * parameter to cmpfunc.
 *
 * Return either NULL if the node with corresponding key is not found, or
 * pointer to the node with corresponding key
 */
avl_node_t *
avl_search(avl_root_t *avlroot, avl_node_t *key, avl_cmp_t *cmpfunc)
{
	avl_node_t *retval;
	
	/*
	 * Do the AVL lookup as normal binary search tree.
	 */
	retval = avlroot->avl_root;
	while (retval) {
		int cmp;

		cmp = cmpfunc(key, retval);
		if (!cmp)
			/* The node with exact key is found, so we leave the
			 * loop */
			break;
		retval = retval->avl_children[avl_cmp2idx(cmp)];
	}

	return retval;
}

/*
 * Find the predecessor or successor of the current node based on direction
 * given.
 * The direction must be either -1 (left) or 1 (right).
 *
 * Return either a valid AVL node or NULL.
 */
static avl_node_t *
avl_prev_next(avl_node_t *node, int dir)
{
	int which_child;
	avl_node_t *parent;

	which_child = avl_cmp2idx(dir);

	if (node->avl_children[which_child]) {
		node = node->avl_children[which_child];
		while (node->avl_children[!which_child])
			node = node->avl_children[!which_child];
		return node;
	}
	parent = node->avl_parent;
	while (parent && parent->avl_children[which_child] == node) {
		node = parent;
		parent = node->avl_parent;
	}
	return parent;
}

/*
 * Get the first element from the AVL tree based on direction given.
 * The direction must be either -1 (left) or 1 (right).
 * 
 * Return either a valid AVL node or NULL.
 */
static avl_node_t *
avl_leftmost_rightmost(avl_root_t *root, int dir)
{
	int which_child = avl_cmp2idx(dir);
	avl_node_t *prev = NULL;
	avl_node_t *node = root->avl_root;

	while (node) {
		prev = node;
		node = node->avl_children[which_child];
	}

	return prev;
}

/*
 * Get the first element from the AVL tree.
 * 
 * Return either a valid AVL node or NULL.
 */
avl_node_t *
avl_first(avl_root_t *root)
{
	return avl_leftmost_rightmost(root, -1);
}

/*
 * Get the last element from the AVL tree.
 * 
 * Return either a valid AVL node or NULL.
 */
avl_node_t *
avl_last(avl_root_t *root)
{
	return avl_leftmost_rightmost(root, 1);
}

/*
 * Find the predecessor of the current node based on direction given.
 *
 * Return either a valid AVL node or NULL.
 */
avl_node_t *
avl_prev(avl_node_t *node)
{
	return avl_prev_next(node, -1);
}

/*
 * Find the successor of the current node based on direction given.
 *
 * Return either a valid AVL node or NULL.
 */
avl_node_t *
avl_next(avl_node_t *node)
{
	return avl_prev_next(node, 1);
}

/*
 * Subtree rebalancing routine
 *
 * Whenever the balancing factor of node goes beyond 1 or -1, we need to
 * rebalance the subtree rooted at the given node so that the balancing factor
 * will be restored to within -1 and 1.
 * 
 * Return 0 if the height of the tree does not change, otherwise return 1 (The
 * tree can only shrink in height by one unit in that case) .
 */
static int
avl_rebalance(avl_root_t *avlroot, avl_node_t *node)
{
	int which_child;
	avl_node_t *child;

	which_child = avl_balance2idx(node->avl_balance);
	child = node->avl_children[which_child];

	if (node->avl_balance == child->avl_balance) {
		avl_node_t **slot;
		avl_node_t *R, *S, *R_parent, *B;

		/*
		 * Case 1:
		 * Single rotation is required
		 *
		 *     **R              S
		 *      / \            / \
		 *    *S   Y    ->    X   R
		 *    / \ (h)       (h+1)/ \
		 *   X   B              B   Y
		 * (h+1)(h)            (h) (h)
		 *
		 * Since the height of subtree is shortened in this case, the
		 * adjustment of balance factor needs to continue at the parent
		 * of this subtree.
		 */
		
		R = node;
		S = child;
		R_parent = R->avl_parent;
		B = S->avl_children[!which_child];
		if (!R_parent)
			slot = &avlroot->avl_root;
		else
			slot = R_parent->avl_children + avl_which_child(R);

		*slot = S;
		S->avl_parent = R_parent;
		
		R->avl_parent = S;
		S->avl_children[!which_child] = R;

		R->avl_children[which_child] = B;
		if (B)
			B->avl_parent = R;
		
		S->avl_balance = R->avl_balance = 0;
		return 1;
	} else if (child->avl_balance) {
		avl_node_t **slot;
		avl_node_t *R, *S, *Q, *R_parent, *B, *C;

		/*
		 * Case 2:
		 * Double rotation is required (there are 3 minor cases)
		 *
		 *
		 * Minor case 1: Q is balanced
		 *
		 *    **R                 Q
		 *     / \             /     \
		 *    S*  D           S       R
		 *   / \ (h)    ->   / \     / \
		 *  A   Q           A   B   C   D
		 * (h) / \         (h) (h) (h) (h)
		 *    B   C
		 *   (h) (h)
		 *
		 * Minor case 2: Q has equal balance factor as S
		 *
		 *    **R                 Q
		 *     / \             /     \
		 *    S*  D          *S       R
		 *   / \ (h)    ->   / \     / \
		 *  A   Q*          A   B   C   D
		 * (h) / \         (h)(h-1)(h) (h)
		 *    B   C
		 *  (h-1)(h)
		 *
		 * Minor case 3: Q has non-equal balance factor as S
		 *
		 *    **R                 Q
		 *     / \             /     \
		 *    S*  D           S       R*
		 *   / \ (h)    ->   / \     / \
		 *  A  *Q           A   B   C   D
		 * (h) / \         (h) (h) (h-1)(h)
		 *    B   C
		 *   (h)(h-1)
		 *
		 * Since the height of subtree is shortened in this case, the
		 * adjustment of balance factor needs to continue at the parent
		 * of this subtree.
		 */

		R = node;
		S = child;
		Q = S->avl_children[!which_child];
		R_parent = R->avl_parent;
		B = Q->avl_children[!which_child];
		C = Q->avl_children[which_child];
		if (!R->avl_parent)
			slot = &avlroot->avl_root;
		else
			slot = R->avl_parent->avl_children + avl_which_child(R);

		*slot = Q;
		Q->avl_parent = R_parent;
		
		R->avl_children[which_child] = B;
		if (B)
			B->avl_parent = R;
		S->avl_children[!which_child] = C;
		if (C)
			C->avl_parent = S;
		
		Q->avl_children[!which_child] = R;
		R->avl_parent = Q;
		Q->avl_children[which_child] = S;
		S->avl_parent = Q;

		if (Q->avl_balance == S->avl_balance) {
			S->avl_balance = Q->avl_balance * -1;
			R->avl_balance = 0;
		} else if (Q->avl_balance) {
			R->avl_balance = Q->avl_balance * -1;
			S->avl_balance = 0;
		} else {
			S->avl_balance = 0;
			R->avl_balance = 0;
		}	
		Q->avl_balance = 0;
		return 1;
	} else {
		avl_node_t **slot;
		avl_node_t *R, *S, *R_parent, *B;

		/*
		 * Case 3:
		 * Single rotation is required (only happens on deletion)
		 *
		 *     **R              S*
		 *      / \            / \
		 *     S   Y    ->    X  *R
		 *    / \(h-1)      (h)  / \
		 *   X   B              B   Y
		 * (h)  (h)            (h) (h-1)
		 *
		 * Since the height of subtree remains the same in this case,
		 * the adjustment of balance factor needs not to continue at the
		 * parent of this subtree.
		 */

		R = node;
		S = child;
		R_parent = R->avl_parent;
		B = S->avl_children[!which_child];
		if (!R_parent)
			slot = &avlroot->avl_root;
		else
			slot = R_parent->avl_children + avl_which_child(R);

		*slot = S;
		S->avl_parent = R_parent;
		
		R->avl_parent = S;
		S->avl_children[!which_child] = R;

		R->avl_children[which_child] = B;
		if (B)
			B->avl_parent = R;

		S->avl_balance = R->avl_balance * -1;
	}
	return 0;
}

/*
 * Generic insert routine for AVL tree
 *
 * The node parameter is the new node to be inserted into the AVL tree.
 *
 * Return the node pointer with the same key as node parameter. That is, if
 * there exists a node with the same key as node parameter, the pointer returned
 * will be different from node parameter.
 */
avl_node_t *
avl_insert(avl_root_t *avlroot, avl_node_t *node, avl_cmp_t *cmpfunc)
{
	avl_node_t *cur, *parent, **curp;
	
	/*
	 * Do the AVL lookup as normal binary search tree.
	 */
	curp = &avlroot->avl_root;
	cur = *curp;
	parent = NULL;
	while (cur) {
		int cmp;

		cmp = cmpfunc(node, cur);
		if (!cmp)
			/* The node with exact key is found, so we return the
			 * found node */
			return cur;
		
		curp = &cur->avl_children[avl_cmp2idx(cmp)];
		parent = cur;
		cur = *curp;
	}

	/* Insert the node into the tree */
	*curp = cur = node;
	cur->avl_parent = parent;
	cur->avl_balance = 0;
	cur->avl_children[0] = cur->avl_children[1] = NULL;
	
	/*
	 * Recalculate balance factor from the parent of the inserted node up to
	 * possibly the root of the tree
	 */
	while (parent) {
		int which_child;
		int balance, abs_balance;

		which_child = avl_which_child(node);
		balance = avl_idx2cmp(which_child);

		/*
		 * Calculate the balance factor on the current node.
		 *
		 * If we introduce perfect balance, we break from the loop.
		 * Otherwise, we have to continue along the path.
		 *
		 * As long as balance factor is [-1,1], we do not need to do
		 * rotation on the node. If the node violates the AVL tree
		 * properties, we have to do rotation to restore AVL tree
		 * properties.
		 */
		abs_balance = avl_abs_balance(parent->avl_balance + balance);
		if (!abs_balance) {
			/* Perfect balance is introduced, thus we simply break
			 * the loop */
			parent->avl_balance += balance;
			break;
		} else if (abs_balance == 1) {
			parent->avl_balance += balance;
		} else {
			/* The node violates AVL tree properties, we need to
			 * do rotation on the node to restore the balance of
			 * the tree. Since we only need to do it once along
			 * the path in order to restore AVL tree properties,
			 * after the rotation we simply break the loop */
			avl_rebalance(avlroot, parent);
			break;
		}

		/* The next iteration will start at the parent of the node on
		 * which we have adjusted its balance factor */
		node = parent;
		parent = parent->avl_parent;
	}
	return cur;
}

/*
 * Generic node removal routine for AVL tree
 *
 * Remove the specified node from AVL tree rooted at avlroot
 */
void
avl_remove(avl_root_t *avlroot, avl_node_t *node)
{
	int which_child;
	avl_node_t *parent;

	if (!node->avl_children[0] && !node->avl_children[1]) {
		parent = node->avl_parent;
		if (!parent) {
			/* This is the only node in the tree, thus we reset
			 * avlroot and return. */
			avlroot->avl_root = NULL;
			return;
		}

		which_child = avl_which_child(node);
		parent->avl_children[which_child] = NULL;
	} else {
		int gchild_idx;
		avl_node_t *child;
		avl_node_t *gchild;

		if (node->avl_balance < 0) {
			/* First select the in-order predecessor node based on
			 * balance factor */
			child = avl_prev_next(node, -1);
			if (!child)
				child = avl_prev_next(node, 1);
		} else {
			child = avl_prev_next(node, 1);
			if (!child)
				child = avl_prev_next(node, -1);
		}

		gchild_idx = avl_balance2idx(child->avl_balance);
		gchild = child->avl_children[gchild_idx];
		/* The parent of the deleted node should be at @child if
		 * @child is @node's direct child */
		parent = child;
		which_child = avl_which_child(child);
		if (child->avl_parent != node) {
			/* The parent of the deleted node should be at @child as
			 * @child is not the direct child of @node */
			parent = child->avl_parent;

			/*
			 * Update the pointer of @child to point to the subtree
			 * that @child originally resides in
			 */
			child->avl_children[which_child] =
			    node->avl_children[which_child];
			if (child->avl_children[which_child])
				child->avl_children[which_child]->avl_parent =
				    child;

			/*
			 * Update the parent of grandchild
			 */
			parent->avl_children[which_child] = gchild;
			if (gchild)
				gchild->avl_parent = parent;
		}
		
		child->avl_children[!which_child] =
		    node->avl_children[!which_child];
		if (child->avl_children[!which_child])
			child->avl_children[!which_child]->avl_parent = child;
		child->avl_balance = node->avl_balance;

		/*
		 * Update the parent of @child to be the parent of @node
		 */
		child->avl_parent = node->avl_parent;
		if (!child->avl_parent)
			avlroot->avl_root = child;
		else
			node->avl_parent->avl_children[avl_which_child(node)] =
			    child;
	}
	
	/*
	 * Recalculate balance factor from the parent of the deleted node up to
	 * possibly the root of the tree
	 */
	while (parent) {
		int balance, abs_balance;

		balance = avl_idx2cmp(which_child) * -1;

		/* The next iteration will start at the parent of the node on
		 * which we have adjusted its balance factor */
		which_child = avl_which_child(parent);
		node = parent;
		parent = parent->avl_parent;

		/*
		 * Calculate the balance factor on the current node.
		 *
		 * As long as balance factor is [-1,1], we do not need to do
		 * rotation on the node. If the node violates the AVL tree
		 * properties, we have to do rotation to restore AVL tree
		 * properties.
		 */		
		abs_balance = avl_abs_balance(node->avl_balance + balance);
		if (!abs_balance) {
			node->avl_balance += balance;
		} else if (abs_balance == 1) {
			node->avl_balance += balance;
			break;
		} else if (!avl_rebalance(avlroot, node)) {
			break;
		}
	}
}
