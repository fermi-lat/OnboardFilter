/* ---------------------------------------------------------------------- *//*!
   
   \file  BTREE.c
   \brief Binary Tree, implementation file
   \author JJRussell - russell@slac.stanford.edu

\verbatim

CVS $Id
\endverbatim

  \warning
   In their current state, these routines are not fit for Flight use.
   They can be used in studies. However they malloc all over the place
   and involve recursive calling techniques, neither of is appropriate
   for Flight code.
                                                                          */
/* ---------------------------------------------------------------------- */

#include <stdlib.h>
#include "DFC/BTREE.h"


/* ---------------------------------------------------------------------- *//*!

  \fn    void BTREE_init (BTREE *tree, BTREE_destroy destroy)
  \brief Initializes the context control structure 

  \param    tree  The binary tree context to initialize
  \param destroy  Callback function to free the nodes in a binary tree,
                  may be NULL.
									  */
/* ---------------------------------------------------------------------- */
void BTREE_init (BTREE *tree, BTREE_destroy destroy)
{
   tree->size    = 0;
   tree->destroy = destroy;
   tree->root    = NULL;

   return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     void BTREE_free (BTREE *tree)
  \brief  Frees all the data associated with the specified tree

  \param  tree  The root of the tree to free
									  */
/* ---------------------------------------------------------------------- */
void BTREE_free (BTREE *tree)
{
   BTREE_removeL (tree, NULL);
   return;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     int BTREE_insertL (BTREE *tree, BTREE_node *node, const void *data)
  \brief  Inserts the data as the left node of the specified node
  \retval  0, if successful
  \retval -1, if failure

  \param  tree  The binary tree
  \param  node  The parent node, may be NULL if this is the first node
  \param  data  Pointer to the data to be inserted

   Failure can occur if
     - \a node is specified as NULL, but the tree is not empty
     - the left node of \a node is already filled
     - allocation of a new node fails
									  */
/* ---------------------------------------------------------------------- */
int BTREE_insertL (BTREE *tree, BTREE_node *node, const void *data)
{
   BTREE_node  *new_node;
   BTREE_node **position;
   int              size;

   size = BTREE__size (tree);
   
   if (node == NULL)
   {
       /* Insertion at the root only in an empty tree */
       if (size > 0) return -1;
       position = &tree->root;
   }
   else
   {
       /* Normally all insertion only at the end of a branch */
       if (BTREE__left (node) != NULL) return -1;

       position = &node->left;
   }

   /* Allocate storage for the new node */
   new_node = (BTREE_node *)malloc (sizeof (BTREE_node));
   if (new_node == NULL) return -1;

   /* Insert the node into the tree */
   new_node->data  = (void *)data;
   new_node->left  = NULL;
   new_node->right = NULL;
  *position        = new_node;

   tree->size = size + 1;

   return 0;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn     int BTREE_insertR (BTREE *tree, BTREE_node *node, const void *data)
  \brief  Inserts the data as the right node of the specified node
  \retval  0, if successful
  \retval -1, if failure

  \param  tree  The binary tree
  \param  node  The parent node, may be NULL if this is the first node
  \param  data  Pointer to the data to be inserted

   Failure can occur if
     - \a node is specified as NULL, but the tree is not empty
     - the right node of \a node is already filled
     - allocation of a new node fails
									  */
/* ---------------------------------------------------------------------- */
int BTREE_insertR (BTREE *tree, BTREE_node *node, const void *data)
{
   BTREE_node  *new_node;
   BTREE_node **position;
   int              size;

   size = BTREE__size (tree);
   
   if (node == NULL)
   {
       /* Insertion at the root only in an empty tree */
       if (size > 0) return -1;
       position = &tree->root;
   }
   else
   {
       /* Normally all insertion only at the end of a branch */
       if (BTREE__right (node) != NULL) return -1;

       position = &node->right;
   }

   /* Allocate storage for the new node */
   new_node = (BTREE_node *)malloc (sizeof (BTREE_node));
   if (new_node == NULL) return -1;

   /* Insert the node into the tree */
   new_node->data  = (void *)data;
   new_node->left  = NULL;
   new_node->right = NULL;
  *position        = new_node;

   tree->size = size + 1;

   return 0;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn    void BTREE_removeL (BTREE *tree, BTREE_node *node)
  \brief Removes the left node and all its branches 

  \param tree  The binary tree
  \param node  The parent node
									  */
/* ---------------------------------------------------------------------- */
void BTREE_removeL (BTREE *tree, BTREE_node *node)
{
   BTREE_node **position;
   int              size;

   size = BTREE__size (tree);

   /* Don't try to remove from an empty tree */
   if (size == 0) return;

   /* Decide where to remove the nodes from  */
   position = node == NULL ? &tree->root : &node->left;

   /* Remove the nodes */
   if (*position != NULL)
   {
       BTREE_removeL (tree, *position);
       BTREE_removeR (tree, *position);

       /* Call the user defined destructor, if any */
       if (tree->destroy != NULL)
       {
           tree->destroy ((*position)->data);
       }

       free (*position);
      *position = NULL;
   }

   tree->size = size - 1;
}
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \fn    void BTREE_removeR (BTREE *tree, BTREE_node *node)
  \brief Removes the right node and all its branches 

  \param tree  The binary tree
  \param node  The parent node
									  */
/* ---------------------------------------------------------------------- */
void BTREE_removeR (BTREE *tree, BTREE_node *node)
{
   BTREE_node **position;
   int              size;

   size = BTREE__size (tree);

   /* Don't try to remove from an empty tree */
   if (size == 0) return;

   /* Decide where to remove the nodes from  */
   position = node == NULL ? &tree->root : &node->right;

   
   /* Remove the nodes */
   if (*position != NULL)
   {
       BTREE_removeL (tree, *position);
       BTREE_removeR (tree, *position);

       /* Call the user defined destructor, if any */
       if (tree->destroy != NULL)
       {
           tree->destroy ((*position)->data);
       }

       free (*position);
      *position = NULL;
   }

   tree->size = size - 1;
}
/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- *//*!

  \fn    int BTREE_merge (BTREE     *merge, 
                          BTREE      *left,
                          BTREE     *right,
                          const void *data)
  \brief  Merges two trees into a single parent tree, adding the data of
          the merged tree
  \return Status 

  \param merge  The merged tree
  \param  left  The left tree to merge
  \param right  The right tree to merge
  \param  data  The data to add
									  */
/* ---------------------------------------------------------------------- */
int BTREE_merge (BTREE *merge, BTREE *left, BTREE *right, const void *data)
{
   /*  Initialize the merged tree */
   BTREE_init (merge, left->destroy);

   
   /* Insert the data for the root node of the merged tree */
   if (BTREE_insertL (merge, NULL, data) != 0)
   {
       BTREE_free (merge);
       return -1;
   }
   

   /* Merge the two binary trees into a single tree */
   BTREE__root (merge)->left  = BTREE__root (left);
   BTREE__root (merge)->right = BTREE__root (right);

   
   /* Adjust the size of the new binary tree */
   merge->size = merge->size + BTREE__size (left) + BTREE__size (right);

   
   /* Kill access of the original trees to the merged nodes */
   left->root  = NULL;
   left->size  = 0;
   right->root = NULL;
   right->size = 0;

   return 0;
}
/* ---------------------------------------------------------------------- */
   
