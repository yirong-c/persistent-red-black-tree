#ifndef _PERSISTENT_RED_BLACK_TREE_HPP
#define _PERSISTENT_RED_BLACK_TREE_HPP

#include <utility>
#include <iterator>
#include <stdexcept>
#include <memory>
#include <stack>

// ---------- declaration ----------

template <class Key, class T>
class PersistentRedBlackTree
{
public:
    typedef std::pair<const Key, T> ValueType;

#ifdef PRBT_TESTING
protected:
#else
private:
#endif
    struct Node
    {
        Node* left;
        Node* right;
        ValueType value;
        enum { BLACK, RED } color;
        int use_count;
        Node() : use_count(0) {}
        Node(const ValueType& value) : use_count(0), value(value) {}
    };
public:
    class Version
    {
    public:
        Version() : next_(nullptr), root_(nullptr) {}
    #ifdef PRBT_TESTING
    public:
    #else
    private:
    #endif
        friend class PersistentRedBlackTree<Key, T>;
        Version(Version* next, Node* root) : next_(next), root_(root) {}
        Version* next_;// singly linked list
        Node* root_;
    };
    class Iterator : public std::iterator<std::bidirectional_iterator_tag, ValueType>
    {
    public:
        Iterator& operator++() { node_ = tree_->TreeSuccessor(version_, node_); return *this; }
        Iterator& operator--() { node_ = tree_->TreePredecessor(version_, node_); return *this; }
        ValueType& operator*() const { return node_->value; }
        ValueType* operator->() const { return &(node_->value); }
        bool operator==(const Iterator& other) const { return node_ == other.node_; }
        bool operator!=(const Iterator& other) const { return node_ != other.node_; }
        Iterator() : node_(nullptr), tree_(nullptr), version_(nullptr) {}
        Version* version() { return version_; }
    private:
        friend class PersistentRedBlackTree<Key, T>;
        Iterator(Node* node, PersistentRedBlackTree<Key, T>* tree, Version* version) 
            : node_(node), tree_(tree), version_(version) {}
        Node* node_;
        PersistentRedBlackTree<Key, T>* tree_;
        Version* version_;
    };
    PersistentRedBlackTree();
    ~PersistentRedBlackTree();
    std::pair<Iterator, bool> Insert(Version* dependent_version, const ValueType& value);
    std::pair<Version*, bool> Delete(Version* dependent_version, const Key& key);
    void Clear();
    T& At(const Key& key);
    Iterator Find(const Key& key);
    Iterator Begin();
    Iterator End();
    T& operator[](Key&& key);

#ifdef PRBT_TESTING
protected:
#else
private:
#endif
    void LeftRotate(Node** subtree_root_node_ptr);
    void RightRotate(Node** subtree_root_nodet);
    void InsertFixup(std::stack<Node**>& path);
    Node* TreeMinimum(Node* sub_tree_root);
    Node* TreeMaximum(Node* sub_tree_root);
    Node* TreeSuccessor(Version* version, Node* node);
    Node* TreePredecessor(Version* version, Node* node);
    void DeleteFixup(std::stack<Node**>& path);
    void CreateCopyAndPlant(Node** node_ptr);
    // Node* root_;
    Node* nil_;
    Version* version_head_;
};

// ---------- definition ----------

template <class Key, class T>
PersistentRedBlackTree<Key, T>::PersistentRedBlackTree()
{
    nil_ = new Node();
    nil_->color = Node::BLACK;
    version_head_ = nullptr;
}

template <class Key, class T>
PersistentRedBlackTree<Key, T>::~PersistentRedBlackTree()
{
    Clear();
    delete nil_;
}

template <class Key, class T>
void PersistentRedBlackTree<Key, T>::LeftRotate(Node** subtree_root_node_ptr) 
{
    Node* new_root;
    new_root = (*subtree_root_node_ptr)->right;
    (*subtree_root_node_ptr)->right = new_root->left;
    new_root->left = (*subtree_root_node_ptr);
    *subtree_root_node_ptr = new_root;
}

template <class Key, class T>
void PersistentRedBlackTree<Key, T>::RightRotate(Node** subtree_root_node_ptr) 
{
    Node* new_root;
    new_root = (*subtree_root_node_ptr)->left;
    (*subtree_root_node_ptr)->left = new_root->right;
    new_root->right = (*subtree_root_node_ptr);
    *subtree_root_node_ptr = new_root;
}

// template <class Key, class T>
// typename PersistentRedBlackTree<Key, T>::Iterator PersistentRedBlackTree<Key, T>::Find(const Key& key)
// {
//     Node* now;
//     now = root_;
//     while (now != nil_)
//     {
//         if (now->value.first == key)
//             break;
//         else if (now->value.first < key)
//             now = now->right;
//         else
//             now = now->left;
//     }
//     return Iterator(now, this);
// }
 
// template <class Key, class T>
// T& PersistentRedBlackTree<Key, T>::At(const Key& key)
// {
//     Node* now;
//     now = root_;
//     while (now != nil_)
//     {
//         if (now->value.first == key)
//             return now->value.second;
//         else if (now->value.first < key)
//             now = now->right;
//         else
//             now = now->left;
//     }
//     throw std::out_of_range("the container does not have an element with the specified key");
// }

// template <class Key, class T>
// T& PersistentRedBlackTree<Key, T>::operator[](Key&& key)
// {
//     Node *node, **now_ptr, *parent;
//     parent = nil_;
//     now_ptr = &root_;
//     while (*now_ptr != nil_)
//     {
//         parent = *now_ptr;
//         if (key == (*now_ptr)->value.first)
//             return (*now_ptr)->value.second;
//         else if (key < (*now_ptr)->value.first)
//             now_ptr = &((*now_ptr)->left);
//         else
//             now_ptr = &((*now_ptr)->right);
//     }
//     node = new Node(std::make_pair(key, T()));
//     *now_ptr = node;
//     node->parent = parent;
//     node->right = node->left = nil_;
//     node->color = Node::RED;
//     InsertFixup(node);
//     return node->value.second;
// }

template <class Key, class T>
std::pair<typename PersistentRedBlackTree<Key, T>::Iterator, bool> 
    PersistentRedBlackTree<Key, T>::Insert
    (Version* dependent_version, const ValueType& value)
{
    Node *dep_now, **new_next_ptr;
    Version* new_version;
    std::stack<Node**> path;
    new_version = new Version(version_head_, nil_);
    version_head_ = new_version;
    new_next_ptr = &(new_version->root_);
    dep_now = dependent_version == nullptr ? nil_ : dependent_version->root_;
    while (dep_now != nil_)
    {
        *new_next_ptr = new Node(dep_now->value);
        path.push(new_next_ptr);
        (*new_next_ptr)->color = dep_now->color;
        if (value.first == dep_now->value.first)
        {
            (*new_next_ptr)->left = dep_now->left;
            ++dep_now->left->use_count;
            (*new_next_ptr)->right = dep_now->right;
            ++dep_now->right->use_count;
            return std::make_pair(Iterator(*new_next_ptr, this, new_version), false);
        }
        else if (value.first < dep_now->value.first)
        {
            (*new_next_ptr)->right = dep_now->right;
            ++dep_now->right->use_count;
            dep_now = dep_now->left;
            new_next_ptr = &((*new_next_ptr)->left);
        }
        else
        {
            (*new_next_ptr)->left = dep_now->left;
            ++dep_now->left->use_count;
            dep_now = dep_now->right;
            new_next_ptr = &((*new_next_ptr)->right);
        }
    }
    *new_next_ptr = new Node(value);
    path.push(new_next_ptr);
    (*new_next_ptr)->color = Node::RED;
    (*new_next_ptr)->left = (*new_next_ptr)->right = nil_;
    InsertFixup(path);
    return std::make_pair(Iterator(*new_next_ptr, this, new_version), true);
}

template <class Key, class T>
void PersistentRedBlackTree<Key, T>::InsertFixup(std::stack<Node**>& path)
{
    Node **uncle_ptr, **grandparent_ptr, **parent_ptr, *node, *tmp;
    node = *path.top();
    path.pop();// now, top is parent of node
    while (true)
    {
        if (path.empty()) { node->color = Node::BLACK; break; }
        parent_ptr = path.top();
        path.pop();// now, top is grandparent of node
        if ((*parent_ptr)->color == Node::BLACK) break;
        grandparent_ptr = path.top();
        path.pop();// now, top is great-grandparent of node
        if (*parent_ptr == (*grandparent_ptr)->left)
        {
            uncle_ptr = &((*grandparent_ptr)->right);
            if ((*uncle_ptr)->color == Node::RED)
            {
                // create copy of uncle
                CreateCopyAndPlant(uncle_ptr);
                // try to fix property 4
                (*uncle_ptr)->color = (*parent_ptr)->color = Node::BLACK;
                (*grandparent_ptr)->color = Node::RED;
                // start a new loop
                node = *grandparent_ptr;
            }
            else
            {
                if (node == (*parent_ptr)->right)
                {
                    node = *parent_ptr;
                    LeftRotate(parent_ptr);
                    // notice that varible grandparent do not need to update here,
                    // since it is still the grandparent of varible node
                }
                (*parent_ptr)->color = Node::BLACK;
                (*grandparent_ptr)->color = Node::RED;
                RightRotate(grandparent_ptr);
                // finish
                break;
            }
        }
        else
        {
            uncle_ptr = &((*grandparent_ptr)->left);
            if ((*uncle_ptr)->color == Node::RED)
            {
                // create copy of uncle
                CreateCopyAndPlant(uncle_ptr);
                // try to fix property 4
                (*uncle_ptr)->color = (*parent_ptr)->color = Node::BLACK;
                (*grandparent_ptr)->color = Node::RED;
                // start a new loop
                node = *grandparent_ptr;
            }
            else
            {
                if (node == (*parent_ptr)->left)
                {
                    node = *parent_ptr;
                    RightRotate(parent_ptr);
                    // notice that varible grandparent do not need to update here,
                    // since it is still the grandparent of varible node
                }
                (*parent_ptr)->color = Node::BLACK;
                (*grandparent_ptr)->color = Node::RED;
                LeftRotate(grandparent_ptr);
                // finish
                break;
            }
        }
        
    }
    // root_->color = Node::BLACK;
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreeMinimum(Node* sub_tree_root)
{
    while (sub_tree_root->left != nil_)
        sub_tree_root = sub_tree_root->left;
    return sub_tree_root;
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreeMaximum(Node* sub_tree_root)
{
    while (sub_tree_root->right != nil_)
        sub_tree_root = sub_tree_root->right;
    return sub_tree_root;
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreeSuccessor
    (Version* version, Node* node)
{
    // Node* parent;
    // if (node->right != this->nil_)
    //     return this->TreeMinimum(node->right);
    // parent = node->parent;
    // while (parent != this->nil_ && parent->right == node)
    // {
    //     node = parent;
    //     parent = node->parent;
    // }
    // return parent;
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreePredecessor
    (Version* version, Node* node)
{
    // Node* parent;
    // if (node->left)
    //     return TreeMaximum(node->left);
    // parent = node->parent;
    // while (parent && parent->left == node)
    // {
    //     node = parent;
    //     parent = node->parent;
    // }
    // return parent;
}

template <class Key, class T>
std::pair<typename PersistentRedBlackTree<Key, T>::Version*, bool> 
PersistentRedBlackTree<Key, T>::Delete(Version* dependent_version, const Key& key)
{
    Node *dep_now, **new_next_ptr, *deleted;
    Version* new_version;
    std::stack<Node**> path;
    bool is_black_deleted;
    new_version = new Version(version_head_, nil_);
    version_head_ = new_version;
    new_next_ptr = &(new_version->root_);
    dep_now = dependent_version == nullptr ? nil_ : dependent_version->root_;
    while (dep_now != nil_)
    {
        if (key == dep_now->value.first)
        {
            // found the node; perform delete
            is_black_deleted = dep_now->color == Node::BLACK;
            if (dep_now->left == nil_)
            {
                *new_next_ptr = dep_now->right;
                ++dep_now->right->use_count;
                path.push(new_next_ptr);// push replaced_replaced
            }
            else if (dep_now->right == nil_)
            {
                *new_next_ptr = dep_now->left;
                ++dep_now->left->use_count;
                path.push(new_next_ptr);// push replaced_replaced
            }
            else
            {
                *new_next_ptr = new Node();// place holder
                deleted = *new_next_ptr;
                path.push(new_next_ptr);
                (*new_next_ptr)->color = dep_now->color;
                (*new_next_ptr)->left = dep_now->left;
                ++dep_now->left->use_count;
                dep_now = dep_now->right;
                new_next_ptr = &((*new_next_ptr)->right);
                // find successor
                while (dep_now->left != nil_)
                {
                    *new_next_ptr = new Node(dep_now->value);
                    path.push(new_next_ptr);
                    (*new_next_ptr)->color = dep_now->color;
                    (*new_next_ptr)->right = dep_now->right;
                    ++dep_now->right->use_count;
                    dep_now = dep_now->left;
                    new_next_ptr = &((*new_next_ptr)->left);
                }
                // now, dep_now is successor
                const_cast<Key&>(deleted->value.first) = dep_now->value.first;
                deleted->value.second = dep_now->value.second;
                is_black_deleted = dep_now->color == Node::BLACK;
                *new_next_ptr = dep_now->right;
                ++dep_now->right->use_count;
                path.push(new_next_ptr);// push replaced_replaced
            }
            if (is_black_deleted)
            // In order to maintain property 5,
            // "replaced_replaced" node has extra black (either "doubly black" or "red-and-black", contributes either 2 or 1)
                DeleteFixup(path);
            return std::make_pair(new_version, true);// finish
        }
        *new_next_ptr = new Node(dep_now->value);
        path.push(new_next_ptr);
        (*new_next_ptr)->color = dep_now->color;
        if (key < dep_now->value.first)
        {
            (*new_next_ptr)->right = dep_now->right;
            ++dep_now->right->use_count;
            dep_now = dep_now->left;
            new_next_ptr = &((*new_next_ptr)->left);
        }
        else
        {
            (*new_next_ptr)->left = dep_now->left;
            ++dep_now->left->use_count;
            dep_now = dep_now->right;
            new_next_ptr = &((*new_next_ptr)->right);
        }
    }
    *new_next_ptr = nil_;
    return std::make_pair(new_version, false);
}

template <class Key, class T>
void PersistentRedBlackTree<Key, T>::CreateCopyAndPlant(Node** node_ptr)
{
    Node *tmp;
    tmp = *node_ptr;
    --tmp->use_count;
    *node_ptr = new Node((*node_ptr)->value);
    (*node_ptr)->color = tmp->color;
    (*node_ptr)->left = tmp->left;
    ++tmp->left->use_count;
    (*node_ptr)->right = tmp->right;
    ++tmp->right->use_count;
}

template <class Key, class T>
void PersistentRedBlackTree<Key, T>::DeleteFixup(std::stack<Node**>& path)
{
    Node **sibling_ptr, **parent_ptr, *node;
    node = *path.top();
    path.pop();// now, top is parent of node
    while (path.empty() == false /* node != root_ */ && node->color == Node::BLACK)
    {
        parent_ptr = path.top();
        path.pop();// now, top is grandparent of node
        if (node == (*parent_ptr)->left)
        {
            sibling_ptr = &((*parent_ptr)->right);
            if ((*sibling_ptr)->color == Node::RED)
            {
                CreateCopyAndPlant(sibling_ptr);
                // perform case 1
                (*sibling_ptr)->color = Node::BLACK;
                (*parent_ptr)->color = Node::RED;
                LeftRotate(parent_ptr);
                path.push(parent_ptr);
                parent_ptr = &((*parent_ptr)->left);
                *sibling_ptr = (*parent_ptr)->right;
            }
            if ((*sibling_ptr)->left->color == Node::BLACK && (*sibling_ptr)->right->color == Node::BLACK)
            {
                CreateCopyAndPlant(sibling_ptr);
                // perform case 2
                (*sibling_ptr)->color = Node::RED;
                node = *parent_ptr;
            }
            else
            {
                if ((*sibling_ptr)->right->color == Node::BLACK)
                {
                    CreateCopyAndPlant(sibling_ptr);
                    CreateCopyAndPlant(&((*sibling_ptr)->left));
                    // perform case 3
                    (*sibling_ptr)->left->color = Node::BLACK;
                    (*sibling_ptr)->color = Node::RED;
                    RightRotate(sibling_ptr);
                    // sibling_ptr = &((*parent_ptr)->right);
                }
                CreateCopyAndPlant(sibling_ptr);
                CreateCopyAndPlant(&((*sibling_ptr)->right));
                // perform case 4
                (*sibling_ptr)->color = (*parent_ptr)->color;
                (*parent_ptr)->color = Node::BLACK;
                (*sibling_ptr)->right->color = Node::BLACK;
                LeftRotate(parent_ptr);
                break;// finish
            }
        }
        else
        {
            sibling_ptr = &((*parent_ptr)->left);
            if ((*sibling_ptr)->color == Node::RED)
            {
                CreateCopyAndPlant(sibling_ptr);
                // perform case 1
                (*sibling_ptr)->color = Node::BLACK;
                (*parent_ptr)->color = Node::RED;
                RightRotate(parent_ptr);
                path.push(parent_ptr);
                parent_ptr = &((*parent_ptr)->right);
                *sibling_ptr = (*parent_ptr)->left;
            }
            if ((*sibling_ptr)->left->color == Node::BLACK && (*sibling_ptr)->right->color == Node::BLACK)
            {
                CreateCopyAndPlant(sibling_ptr);
                // perform case 2
                (*sibling_ptr)->color = Node::RED;
                node = *parent_ptr;
            }
            else
            {
                if ((*sibling_ptr)->left->color == Node::BLACK)
                {
                    CreateCopyAndPlant(sibling_ptr);
                    CreateCopyAndPlant(&((*sibling_ptr)->right));
                    // perform case 3
                    (*sibling_ptr)->right->color = Node::BLACK;
                    (*sibling_ptr)->color = Node::RED;
                    LeftRotate(sibling_ptr);
                    // sibling_ptr = &((*parent_ptr)->left);
                }
                CreateCopyAndPlant(sibling_ptr);
                CreateCopyAndPlant(&((*sibling_ptr)->left));
                // perform case 4
                (*sibling_ptr)->color = (*parent_ptr)->color;
                (*parent_ptr)->color = Node::BLACK;
                (*sibling_ptr)->left->color = Node::BLACK;
                RightRotate(parent_ptr);
                break;// finish
            }
        }
    }
    node->color = Node::BLACK;
}

template <class Key, class T>
void PersistentRedBlackTree<Key, T>::Clear()
{
    // Node *now, *parent;
    // now = TreeMinimum(root_);
    // while (now != nil_)
    // {
    //     while (now->right != nil_)
    //     {
    //         now = TreeMinimum(now->right);
    //     }
    //     parent = now->parent;
    //     while (parent != nil_ && parent->right == now)
    //     {
    //         delete now;
    //         now = parent;
    //         parent = now->parent;
    //     }
    //     delete now;
    //     now = parent;
    // }
    // root_ = nil_;
}

// template <class Key, class T>
// typename PersistentRedBlackTree<Key, T>::Iterator PersistentRedBlackTree<Key, T>::Begin()
// {
//     return Iterator(TreeMinimum(root_), this);
// }

// template <class Key, class T>
// typename PersistentRedBlackTree<Key, T>::Iterator PersistentRedBlackTree<Key, T>::End()
// {
//     return Iterator(nil_, this);
// }

#endif
