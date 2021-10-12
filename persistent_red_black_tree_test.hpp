#ifndef _PERSISTENT_RED_BLACK_TREE_TEST_HPP
#define _PERSISTENT_RED_BLACK_TREE_TEST_HPP

#define PRBT_TESTING

#include "persistent_red_black_tree.hpp"

#include <iostream>
#include <vector>
#include <list>

#define OUT_RESET   "\033[0m"
#define OUT_BLACK   "\033[30m"      /* Black */
#define OUT_RED     "\033[31m"      /* Red */
#define OUT_GREEN   "\033[32m"      /* Green */
#define OUT_YELLOW  "\033[33m"      /* Yellow */
#define OUT_BLUE    "\033[34m"      /* Blue */
#define OUT_MAGENTA "\033[35m"      /* Magenta */
#define OUT_CYAN    "\033[36m"      /* Cyan */
#define OUT_WHITE   "\033[37m"      /* White */
#define OUT_BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define OUT_BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define OUT_BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define OUT_BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define OUT_BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define OUT_BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define OUT_BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define OUT_BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

template <class Key, class T>
class PersistentRedBlackTreeTest : public PersistentRedBlackTree<Key, T>
{
public:
    typedef PersistentRedBlackTree<Key, T> Tree;
    typedef typename Tree::Node Node;
    typedef typename Tree::Version* VersionPtr;
    typedef typename Tree::ConstIterator CIterator;
    typedef typename std::pair<Key, T> NonConstValueType;
    // return number of black nodes on the simple path from the subtree_root to descendant leaves
    // return -1 means the RBT is invalid
    int CheckRBSubtreeValid(const Node* subtree_root)
    {
        int black_node_num, left_black_node_num, right_black_node_num;
        if (subtree_root == this->nil_)
            return 1;
        if (subtree_root->color == Node::RED && (subtree_root->left->color != Node::BLACK || subtree_root->right->color != Node::BLACK))
            return -1;
        left_black_node_num = CheckRBSubtreeValid(subtree_root->left);
        right_black_node_num = CheckRBSubtreeValid(subtree_root->right);
        if (left_black_node_num == -1 || right_black_node_num == -1 || left_black_node_num != right_black_node_num)
            return -1;
        return left_black_node_num + ((subtree_root->color == Node::BLACK) ? 1 : 0);
    }

    bool CheckTreeValid(VersionPtr version)
    {
        CIterator it, it_last;
        if (CheckRBSubtreeValid(version->root_) == -1) return false;
        it = this->CBegin(version);
        if (it != this->CEnd())
        {
            it_last = it;
            ++it;
            while (it != this->CEnd())
            {
                if (it_last->first >= it->first) return false;
                it_last = it;
                ++it;
            }
        }
        return true;
    }

    bool CheckTreeValid(VersionPtr version, const std::vector<NonConstValueType>& require_values)
    {
        size_t require_values_index;
        CIterator it, it_last;
        if (CheckRBSubtreeValid(version->root_) == -1) return false;
        it = this->CBegin(version);
        require_values_index = 0;
        if (it != this->CEnd())
        {
            if (require_values[require_values_index].first != it->first ||
                require_values[require_values_index].second != it->second) 
                return false;
            ++require_values_index;
            it_last = it;
            ++it;
            while (it != this->CEnd())
            {
                if (require_values[require_values_index].first != it->first ||
                    require_values[require_values_index].second != it->second) 
                    return false;
                ++require_values_index;
                if (it_last->first >= it->first) return false;
                it_last = it;
                ++it;
            }
        }
        if (require_values_index != require_values.size()) return false;
        return true;
    }

    bool CheckTreeValidAllVersion()
    {        
        VersionPtr now;
        for (now = this->version_nil_->next_; now != this->version_nil_; now = now->next_)
        {
            if (CheckTreeValid(now) == false) return false;
        }
        return true;
    }

    void PrintNode(Node* node)
    {
        if (node->color == Node::RED) std::cout << OUT_RED;
        std::cout << "(" << node->value.first << ", " << node->value.second << "):";
        std::cout << node->use_count;
        if (node->color == Node::RED) std::cout << OUT_RESET;
    }

    void PrintTree(VersionPtr version)
    {
        std::list<Node*> now, todo;
        bool all_nil;
        std::cout << "----------" << std::endl;
        now.push_back(version->root_);
        while (true)
        {
            all_nil = true;
            for (Node* node : now)
            {
                if (node != this->nil_)
                {
                    all_nil = false;
                    break;
                }
            }
            if (all_nil) break;
            for (Node* node : now)
            {
                if (node == this->nil_)
                {
                    std::cout << "NULL";
                    todo.push_back(this->nil_);
                    todo.push_back(this->nil_);
                }
                else
                {
                    PrintNode(node);
                    todo.push_back(node->left);
                    todo.push_back(node->right);
                }
                std::cout << "   ";
            }
            std::cout << std::endl;
            now = todo;
            todo.clear();
        }
    }

};

#endif
