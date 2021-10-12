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
        Version() : next_(nullptr), prev_(nullptr), root_(nullptr) {}
    #ifdef PRBT_TESTING
    public:
    #else
    private:
    #endif
        friend class PersistentRedBlackTree<Key, T>;
        Version(Version* next, Version* prev, Node* root) : next_(next), prev_(prev), root_(root) {}
        Version* next_;// linked list
        Version* prev_;// linked list
        Node* root_;
    };
    class ConstIterator : public std::iterator<std::bidirectional_iterator_tag, ValueType>
    {
    public:
        ConstIterator& operator++() { node_ = tree_->TreeSuccessor(version_, node_); return *this; }
        ConstIterator& operator--() { node_ = tree_->TreePredecessor(version_, node_); return *this; }
        const ValueType& operator*() const { return node_->value; }
        const ValueType* operator->() const { return &(node_->value); }
        bool operator==(const ConstIterator& other) const { return node_ == other.node_; }
        bool operator!=(const ConstIterator& other) const { return !(*this == other); }
        ConstIterator() : node_(nullptr), tree_(nullptr), version_(nullptr) {}
        Version* version() { return version_; }
    private:
        friend class PersistentRedBlackTree<Key, T>;
        ConstIterator(Node* node, PersistentRedBlackTree<Key, T>* tree, Version* version) 
            : node_(node), tree_(tree), version_(version) {}
        Node* node_;
        PersistentRedBlackTree<Key, T>* tree_;
        Version* version_;
    };

    PersistentRedBlackTree();
    ~PersistentRedBlackTree();
    std::pair<ConstIterator, bool> Insert(const ValueType& value, Version* dependent_version);
    std::pair<ConstIterator, bool> InsertOrAssign(const ValueType& value, Version* dependent_version);
    std::pair<Version*, bool> Delete(const Key& key, Version* dependent_version);
    std::pair<ConstIterator, bool> Insert(const ValueType& value);
    std::pair<ConstIterator, bool> InsertOrAssign(const ValueType& value);
    std::pair<Version*, bool> Delete(const Key& key);
    void RemoveVersion(Version* version);
    void Clear();
    const T& At(const Key& key, Version* version);
    ConstIterator Find(const Key& key, Version* version);
    ConstIterator CBegin(Version* version);
    ConstIterator CEnd();

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
    Node* TreeMinimumTraverseSingleUse(Node* sub_tree_root, std::stack<Node*>& path);
    // Node* root_;
    Node* nil_;
    Version* version_nil_;
};

// ---------- definition ----------

template <class Key, class T>
PersistentRedBlackTree<Key, T>::PersistentRedBlackTree()
{
    nil_ = new Node();
    nil_->color = Node::BLACK;
    version_nil_ = new Version();
    version_nil_->next_ = version_nil_->prev_ = version_nil_;
    version_nil_->root_ = nil_;
}

template <class Key, class T>
PersistentRedBlackTree<Key, T>::~PersistentRedBlackTree()
{
    Clear();
    delete version_nil_;
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

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::ConstIterator PersistentRedBlackTree<Key, T>::Find
    (const Key& key, Version* version)
{
    Node* now;
    now = version->root_;
    while (now != nil_)
    {
        if (now->value.first == key)
            break;
        else if (now->value.first < key)
            now = now->right;
        else
            now = now->left;
    }
    return ConstIterator(now, this, version);
}
 
template <class Key, class T>
const T& PersistentRedBlackTree<Key, T>::At(const Key& key, Version* version)
{
    Node* now;
    now = version->root_;
    while (now != nil_)
    {
        if (now->value.first == key)
            return now->value.second;
        else if (now->value.first < key)
            now = now->right;
        else
            now = now->left;
    }
    throw std::out_of_range("the container does not have an element with the specified key");
}

template <class Key, class T>
std::pair<typename PersistentRedBlackTree<Key, T>::ConstIterator, bool> 
    PersistentRedBlackTree<Key, T>::InsertOrAssign
    (const ValueType& value)
{
    return InsertOrAssign(value, version_nil_->next_);
}

template <class Key, class T>
std::pair<typename PersistentRedBlackTree<Key, T>::ConstIterator, bool> 
    PersistentRedBlackTree<Key, T>::InsertOrAssign
    (const ValueType& value, Version* dependent_version)
{
    std::pair<ConstIterator, bool> insert_result;
    insert_result = Insert(value, dependent_version);
    if (insert_result.second == false) insert_result.first.node_->value.second = value.second;
    return insert_result;
}

template <class Key, class T>
std::pair<typename PersistentRedBlackTree<Key, T>::ConstIterator, bool> 
    PersistentRedBlackTree<Key, T>::Insert
    (const ValueType& value)
{
    return Insert(value, version_nil_->next_);
}

template <class Key, class T>
std::pair<typename PersistentRedBlackTree<Key, T>::ConstIterator, bool> 
    PersistentRedBlackTree<Key, T>::Insert
    (const ValueType& value, Version* dependent_version)
{
    Node *dep_now, **new_next_ptr;
    Version* new_version;
    std::stack<Node**> path;
    new_version = new Version(version_nil_->next_, version_nil_, nil_);
    version_nil_->next_ = new_version;
    new_version->next_->prev_ = new_version;
    new_next_ptr = &(new_version->root_);
    // dep_now = dependent_version == nullptr ? nil_ : dependent_version->root_;
    dep_now = dependent_version->root_;
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
            return std::make_pair(ConstIterator(*new_next_ptr, this, new_version), false);
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
    return std::make_pair(ConstIterator(*new_next_ptr, this, new_version), true);
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
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreeMinimum
    (Node* sub_tree_root)
{
    while (sub_tree_root->left != nil_)
        sub_tree_root = sub_tree_root->left;
    return sub_tree_root;
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreeMinimumTraverseSingleUse
    (Node* sub_tree_root, std::stack<Node*>& path)
{
    while (sub_tree_root->left != nil_ && sub_tree_root->left->use_count == 0)
    {
        path.push(sub_tree_root);
        sub_tree_root = sub_tree_root->left;
    }
    return sub_tree_root;
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreeMaximum
    (Node* sub_tree_root)
{
    while (sub_tree_root->right != nil_)
        sub_tree_root = sub_tree_root->right;
    return sub_tree_root;
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreeSuccessor
    (Version* version, Node* node)
{
    Node *now, *succ;
    if (node->right != nil_) return TreeMinimum(node->right);
    now = version->root_;
    succ = nil_;
    while (now != nil_)
    {
        if (now->value.first == node->value.first)
        {
            return succ;
        }
        else if (now->value.first > node->value.first)
        {
            succ = now;
            now = now->left;
        }
        else
        {
            now = now->right;
        }
    }
    throw std::runtime_error("node is not found in the version");
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::Node* PersistentRedBlackTree<Key, T>::TreePredecessor
    (Version* version, Node* node)
{
    Node *now, *prev;
    if (node->left != nil_) return TreeMaximum(node->left);
    now = version->root_;
    prev = nil_;
    while (now != nil_)
    {
        if (now->value.first == node->value.first)
        {
            return prev;
        }
        else if (now->value.first >node->value.first)
        {
            now = now->left;
        }
        else
        {
            prev = now;
            now = now->right;
        }
    }
    throw std::runtime_error("node is not found in the version");
}

template <class Key, class T>
std::pair<typename PersistentRedBlackTree<Key, T>::Version*, bool> 
PersistentRedBlackTree<Key, T>::Delete(const Key& key)
{
    return Delete(key, version_nil_->next_);
}

template <class Key, class T>
std::pair<typename PersistentRedBlackTree<Key, T>::Version*, bool> 
PersistentRedBlackTree<Key, T>::Delete(const Key& key, Version* dependent_version)
{
    Node *dep_now, **new_next_ptr, *deleted;
    Version* new_version;
    std::stack<Node**> path;
    bool is_black_deleted;
    new_version = new Version(version_nil_->next_, version_nil_, nil_);
    version_nil_->next_ = new_version;
    new_version->next_->prev_ = new_version;
    new_next_ptr = &(new_version->root_);
    // dep_now = dependent_version == nullptr ? nil_ : dependent_version->root_;
    dep_now = dependent_version->root_;
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
                    // case 4 will override the colors
                    // (*sibling_ptr)->left->color = Node::BLACK;
                    // (*sibling_ptr)->color = Node::RED;
                    RightRotate(sibling_ptr);
                    // sibling_ptr = &((*parent_ptr)->right);
                }
                else
                {
                    CreateCopyAndPlant(sibling_ptr);
                    CreateCopyAndPlant(&((*sibling_ptr)->right));
                }
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
void PersistentRedBlackTree<Key, T>::RemoveVersion(Version* version)
{
    Node *now, *parent;
    std::stack<Node*> path;
    if (version->root_ != nil_) 
    {
        path.push(nil_);
        now = TreeMinimumTraverseSingleUse(version->root_, path);
        --now->left->use_count;
        while (now != nil_)
        {
            while (now->right != nil_ && now->right->use_count == 0)
            {
                path.push(now);
                now = now->right;
                now = TreeMinimumTraverseSingleUse(now, path);
                --now->left->use_count;
            }
            --now->right->use_count;
            parent = path.top();
            while (parent != nil_ && parent->right == now)
            {
                delete now;
                now = parent;
                path.pop();
                parent = path.top();
            }
            delete now;
            now = parent;
            path.pop();
        }
        version->root_ = nil_;
    }
    version->prev_->next_ = version->next_;
    version->next_->prev_ = version->prev_;
    delete version;
}

template <class Key, class T>
void PersistentRedBlackTree<Key, T>::Clear()
{
    while (version_nil_->next_ != version_nil_) RemoveVersion(version_nil_->next_);
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::ConstIterator PersistentRedBlackTree<Key, T>::CBegin(Version* version)
{
    return ConstIterator(TreeMinimum(version->root_), this, version);
}

template <class Key, class T>
typename PersistentRedBlackTree<Key, T>::ConstIterator PersistentRedBlackTree<Key, T>::CEnd()
{
    return ConstIterator(nil_, this, version_nil_);
}

#endif
