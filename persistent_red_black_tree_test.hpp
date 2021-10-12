#ifndef _PERSISTENT_RED_BLACK_TREE_TEST_HPP
#define _PERSISTENT_RED_BLACK_TREE_TEST_HPP

#define PRBT_TESTING

#include "persistent_red_black_tree.hpp"

#include <iostream>

template <class Key, class T>
class PersistentRedBlackTreeTest : public PersistentRedBlackTree<Key, T>
{
public:
    typedef typename PersistentRedBlackTree<Key, T>::Node PrbtNode;
    typedef typename PersistentRedBlackTree<Key, T>::Version PrbtVersion;

    // return number of black nodes on the simple path from the subtree_root to descendant leaves
    // return -1 means the RBT is invalid
    int CheckRBSubtreeValid(const PrbtNode* subtree_root)
    {
        int black_node_num, left_black_node_num, right_black_node_num;
        if (subtree_root == this->nil_)
            return 1;
        if (subtree_root->color == PrbtNode::RED && (subtree_root->left->color != PrbtNode::BLACK || subtree_root->right->color != PrbtNode::BLACK))
            return -1;
        left_black_node_num = CheckRBSubtreeValid(subtree_root->left);
        right_black_node_num = CheckRBSubtreeValid(subtree_root->right);
        if (left_black_node_num == -1 || right_black_node_num == -1 || left_black_node_num != right_black_node_num)
            return -1;
        return left_black_node_num + ((subtree_root->color == PrbtNode::BLACK) ? 1 : 0);
    }

    bool CheckTreeValid()
    {        
        PrbtVersion* now;
        for (now = this->version_head_; now != nullptr; now = now->next_)
        {
            if (CheckRBSubtreeValid(now->root_) == -1) return false;
        }
        return true;
        // const Key* last_key_ptr;
        // last_key_ptr = nullptr;
        // now = this->TreeMinimum(this->root_);
        // if (this->root_->color != PrbtNode::BLACK || this->nil_->color != PrbtNode::BLACK)
        //     return false;
        // while (now != this->nil_)
        // {
        //     //test node relationship
        //     if (CheckNodeValid(now) == false)
        //         return false;
        //     //test BST
        //     if (last_key_ptr && *last_key_ptr >= now->value.first)
        //         return false;
        //     last_key_ptr = &now->value.first;
        //     now = this->TreeSuccessor(now);
        // }
        // if (CheckRBSubtreeValid(this->root_) == -1)
        //     return false;
        // return true;
    }

};

#endif
