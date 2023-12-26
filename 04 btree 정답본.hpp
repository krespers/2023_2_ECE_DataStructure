#include <cstddef>
#include <array>
#include <iostream>
#include <optional>
#include <iterator>
#include <algorithm>
#include <cstring>
#include <string>
#include <sstream>
#include <functional>

enum class NodeType { LEAF, INTERNAL };

template<typename T, size_t B = 6>
struct BTreeNode;

template<typename T, size_t B = 6>
struct BTree {
    BTreeNode<T, B>* root = nullptr;

    ~BTree() { if (root) delete root; }

    bool insert(const T&);
    bool remove(const T&);

    void for_all(std::function<void(T&)>);
    void for_all_nodes(std::function<void(const BTreeNode<T,B>&)>);

    const std::optional<T> find_rightmost_key() const;
    const std::optional<size_t> depth() const;

    std::string format(void) const;
};

template<typename T, size_t B>
struct BTreeNode {
    NodeType type;
    size_t n;
    std::array<T, 2 * B - 1> keys;
    std::array<BTreeNode *, 2 * B> edges;

    BTreeNode();
    BTreeNode(const T& t);
    BTreeNode(std::initializer_list<T>);

    template<typename InputIt>
    BTreeNode(InputIt begin, InputIt end);

    ~BTreeNode();

    bool insert(const T& t);
    size_t get_index(const T& t);

    void for_all(std::function<void(T&)> func);

    bool remove(const T& t);

    size_t depth(void);
    std::string format_subtree(size_t) const;
    std::string format_level(size_t) const;
    std::string format_node(void) const;
    std::vector<BTreeNode<T, B>*> find_nodes_at_level(size_t) const;

    void for_all_nodes(std::function<void(const BTreeNode&)>);

    static std::pair<BTreeNode*, size_t> search(BTreeNode<T, B>*, const T& t);
    static void split_child(BTreeNode<T, B>&, size_t);
    static bool try_borrow_from_sibling(BTreeNode<T, B>&, size_t);
    static bool borrow_from_right(BTreeNode<T, B>&, size_t);
    static bool borrow_from_left(BTreeNode<T, B>&, size_t);

    /* NOTE: If the root node has only one key, it will be empty after
      merging the children. Take care of updating the root. I guess this is
      the only way a B-tree may shrink its height. */
    static bool merge_children(BTreeNode<T, B>&, size_t);

    static T& find_rightmost_key(BTreeNode<T, B>&);
};

template<typename T,  size_t B>
bool BTree<T, B>::insert(const T& t) {
    if (!root) {
        root = new BTreeNode<T, B>(t);
        return true;
    }

    /* Make sure the root node is not full. Create an empty tree which has
       the original root as a child. Then split the original root. */
    if (root->n >= 2 * B - 1) {
        BTreeNode<T, B>* new_root = new BTreeNode<T, B>{};
        new_root->edges[0] = root;
        BTreeNode<T, B>::split_child(*new_root, 0);
        root = new_root;
    }

    return root->insert(t);
}

/* By default, use in-order traversal */
template<typename T, size_t B>
void BTree<T, B>::for_all(std::function<void(T&)> func) {
    if (root)
        root->for_all(func);
}

/* This isn't necessarily the in-order traversal */
template<typename T, size_t B>
void BTree<T, B>::for_all_nodes(std::function<void(const BTreeNode<T,B>&)> func) {
    if (root)
        root->for_all_nodes(func);
}

template<typename T, size_t B>
const std::optional<T> BTree<T, B>::find_rightmost_key() const {
    if (!root)
        return std::nullopt;

    return BTreeNode<T, B>::find_rightmost_key(*root);
}

template<typename T, size_t B>
const std::optional<size_t> BTree<T, B>::depth() const {
    if (!root)
        return std::nullopt;

    return root->depth();
}

template<typename T, size_t B>
bool BTreeNode<T, B>::insert(const T& t) {
    // TODO
    if(type == NodeType::INTERNAL){
        if(edges[get_index(t)]->n == 2*B-1){
            split_child(*this, get_index(t));
        }
        return edges[get_index(t)]->insert(t);
    }
    else{

        int idx = get_index(t);
        for(int i = n-1; i >= idx; i--){
            keys[i+1] = keys[i];
        }
        

        keys[idx] = t;
        n++;
        return true;
    }
}

/**
 * Find the desired position of t in current node.
 *
 * For example, if `n` looks like the following:
 *
 * [ 3 | 9 | 13 | 27 ]
 *
 * Then,
 *     n.get_index(2) = 0
 *     n.get_index(5) = 1
 *     n.get_index(10) = 2
 *     n.get_index(19) = 3
 *     n.get_index(31) = 4
 */
template<typename T, size_t B>
size_t BTreeNode<T, B>::get_index(const T& t) {
    // TODO
    for(int i = 0; i < n; i++){
        if(t < keys[i]) return i;
    }
    return n;
}

// NOTE: `for_all` and `for_all_nodes` are used internally for testing.
// I'd not recommend using them in your functions...
template<typename T, size_t B>
void BTreeNode<T, B>::for_all(std::function<void(T&)> func) {
    if (type == NodeType::LEAF) {
        for (auto j = 0; j < n; j++)
            func(keys[j]);
    } else {
        if (n < 1)
            return;

        for (auto j = 0; j < n; j++) {
            edges[j]->for_all(func);
            func(keys[j]);
        }

        /* The rightest edge */
        edges[n]->for_all(func);
    }
}

/* This isn't necessarily the in-order traversal */
template<typename T, size_t B>
void BTreeNode<T, B>::for_all_nodes(std::function<void(const BTreeNode<T,B>&)> func) {
    if (type == NodeType::LEAF) {
        func(*this);
    } else {
        if (n < 1)
            return;

        func(*this);

        for (auto j = 0; j < n + 1; j++) {
            edges[j]->for_all_nodes(func);
        }
    }
}

/* Assume this is called only when the child parent->edges[idx] is full, and
   the parent is not full. */
template<typename T, size_t B>
void BTreeNode<T, B>::split_child(BTreeNode<T, B>& parent, size_t idx) {
    // TODO
    parent.edges[parent.n + 1] = parent.edges[parent.n];

    for(int i = parent.n-1; i >= (int)idx; i--){
        parent.keys[i+1] = parent.keys[i];
        parent.edges[i+1] = parent.edges[i];
    }

    BTreeNode<T, B> *new_left = new BTreeNode<T, B>{},
        *original = parent.edges[idx+1];

    for(int i = 0; i < B-1; i++){
        new_left->keys[i] = original->keys[i];
        original->keys[i] = original->keys[B + i];
    }
    parent.keys[idx] = original->keys[B-1];

    if(original->type == NodeType::INTERNAL){
        for(int i = 0; i < B; i++){
            new_left->edges[i] = original->edges[i];
            original->edges[i] = original->edges[B + i];
        }
        new_left->type = NodeType::INTERNAL;
    }

    new_left->n = B-1;
    original->n = B-1;
    
    parent.edges[idx] = new_left;
    parent.n ++;
    parent.type = NodeType::INTERNAL;
}

template<typename T, size_t B>
bool BTree<T, B>::remove(const T& t) {
    if (!root)
        return false;

    root->remove(t);

    /* After merging, the size of the root may become 0. */
    if (root->n == 0 && root->type == NodeType::INTERNAL) {
        auto prev_root = root;
        root = root->edges[0];
        prev_root->type = NodeType::LEAF;
        delete prev_root;
    }

    return true;
}

template<typename T, size_t B>
bool BTreeNode<T, B>::remove(const T& t) {
    // TODO
    //std::cout<<"remove "<<t<<std::endl;
    int i;
    for(i=0; i<n; i++){
        if(t == keys[i]){
            if(type == NodeType::INTERNAL){
                if(edges[i]->n == B-1){
                    if(!try_borrow_from_sibling(*this, i)) {
                        merge_children(*this, i);
                        return edges[i]->remove(t);
                    }
                    if(t != keys[i]) return edges[i]->remove(t);
                }

                //std::cout<<"switch"<<std::endl;
                keys[i] = find_rightmost_key(*edges[i]);
                return edges[i]->remove(keys[i]);
            }
            else{ //type == NodeType::LEAF
                for(int j = i; j < n-1; j++){
                    keys[j] = keys[j+1];
                }
                n--;
                return true;
            }
        }
        if(t < keys[i]) break;
    }

    if(type == NodeType::LEAF) return false;

    if(edges[i]->n == B-1){
        if(!try_borrow_from_sibling(*this, i)) {
            if(i == n) {merge_children(*this, i-1); i--;}
            else merge_children(*this, i);
        }
    }
    return edges[i]->remove(t);
}

/**
 * Try to borrow a key from sibling.
 *
 * @e: The index of the edge that are trying to borrow a key
 * @return true if borrowing succeed, false otherwise
 */
template<typename T, size_t B>
bool BTreeNode<T, B>::try_borrow_from_sibling(BTreeNode<T, B>&node, size_t e) {
    // TODO
    //std::cout<<"try borrow"<<std::endl;
    if(e+1 <= node.n && node.edges[e+1]->n > B-1){
        return borrow_from_right(node, e);
    }
    else if((int)e-1 >= 0 && node.edges[e-1]->n > B-1){
        return borrow_from_left(node, e);
    }
    return false;
}

template<typename T, size_t B>
bool BTreeNode<T, B>::borrow_from_right(BTreeNode<T, B>& node, size_t edge) {
    // TODO
    //std::cout<<"borrow right"<<std::endl;
    BTreeNode<T, B> *borrower = node.edges[edge], *lender = node.edges[edge+1];

    borrower->keys[borrower->n] = node.keys[edge];
    node.keys[edge] = lender->keys[0];
    for(int i = 0; i < lender->n-1; i++){ //pull n-1 keys
        lender->keys[i] = lender->keys[i+1];
    }

    if(lender->type == NodeType::INTERNAL){
        borrower->edges[borrower->n+1] = lender->edges[0];
        for(int i = 0; i < lender->n; i++){ //pull n edges
            lender->edges[i] = lender->edges[i+1];
        }
    }

    borrower->n ++;
    lender->n --;

    return true;
}

template<typename T, size_t B>
bool BTreeNode<T, B>::borrow_from_left(BTreeNode<T, B>& node, size_t edge) {
    // TODO
    //std::cout<<"borrow left"<<std::endl;
    BTreeNode<T, B> *borrower = node.edges[edge], *lender = node.edges[edge-1];

    for(int i = borrower->n; i > 0; i--){ //push n keys
        borrower->keys[i] = borrower->keys[i-1];
    }
    borrower->keys[0] = node.keys[edge-1];
    node.keys[edge-1] = lender->keys[lender->n-1];

    if(lender->type == NodeType::INTERNAL){
        for(int i = borrower->n+1; i > 0; i--){ //push n+1 edges
            borrower->edges[i] = borrower->edges[i-1];
        }
        borrower->edges[0] = lender->edges[lender->n];
    }

    borrower->n ++;
    lender->n --;

    return true;
}

template<typename T, size_t B>
bool BTreeNode<T, B>::merge_children(BTreeNode<T, B> & node, size_t idx) {
    // TODO
    //std::cout<<"merge"<<std::endl;
    BTreeNode<T, B> *target = node.edges[idx], *victim = node.edges[idx+1];
    target->keys[target->n] = node.keys[idx];
    for(int i = 0; i < victim->n; i++){
        target->keys[target->n+1 + i] = victim->keys[i];
    }

    if(victim->type == NodeType::INTERNAL){
        for(int i = 0; i <= victim->n; i++){
            target->edges[target->n+1 + i] = victim->edges[i];
            victim->edges[i] = nullptr;
        }
    }

    for(int i = idx; i < node.n-1; i++){
        node.keys[i] = node.keys[i+1];
    }
    for(int i = idx+1; i < node.n; i++){
        node.edges[i] = node.edges[i+1];
    }

    target->n += victim->n + 1;
    node.n --;

    delete victim;
    //std::cout<<"merge complete"<<std::endl;
    return true;
}

template<typename T, size_t B>
T& BTreeNode<T, B>::find_rightmost_key(BTreeNode<T, B>& node) {
    if (node.type == NodeType::LEAF)
        return node.keys[node.n - 1];

    return find_rightmost_key(*node.edges[node.n]);
}

// NOTE: `search` function is originally intended to be used by testing code.
// Don't modify this function. You can reuse this function 'as-is', or, if
// you want to do something different, add another function based on this function.
template<typename T, size_t B>
std::pair<BTreeNode<T, B>*, size_t>
BTreeNode<T, B>::search(BTreeNode<T, B>* node, const T& t) {
    if (node->type == NodeType::LEAF) {
        for (auto i = 0; i < node->keys.size(); i++)
            if (t == node->keys[i])
                return { node, i };

        return { nullptr, -1 };
    }

    size_t i;
    for (i = 0; i < node->n; i++) {
        if (t == node->keys[i])
            return { node, i };

        if (t < node->keys[i]) {
            return search(node->edges[i], t);
        }
    }

    return search(node->edges[i], t);
}

template<typename T, size_t B>
size_t BTreeNode<T, B>::depth(void) {
    if (type == NodeType::LEAF)
        return 0;

    return 1 + edges[0]->depth();
}

template<typename T, size_t B>
std::ostream& operator<<(std::ostream& os, const BTree<T, B>& btree) {
    os << btree.format();
    return os;
}

template <typename T, size_t B>
std::string BTree<T, B>::format(void) const {
    if (!root)
        return std::string{};

    return root->format_subtree(root->depth());
}

template<typename T, size_t B>
std::string BTreeNode<T, B>::format_subtree(size_t depth) const {
    std::ostringstream os;

    for (auto i = 0; i <= depth; i++)
        os << format_level(i) << '\n';

    return os.str();
}

template<typename T, size_t B>
std::string BTreeNode<T, B>::format_level(size_t level) const {
    std::ostringstream os;
    auto nodes_at_level = find_nodes_at_level(level);

    for (auto node : nodes_at_level)
        os << node->format_node() << ' ';

    return os.str();
}


template<typename T, size_t B>
std::string BTreeNode<T, B>::format_node(void) const {
    std::ostringstream os;

    if (n < 1) {
        os << "[]";
        return os.str();
    }

    os << '[';
    for (auto i = 0; i < n - 1; i++)
        os << keys[i] << '|';
    os << keys[n - 1];
    os << ']';

    return os.str();
}

template<typename T, size_t B>
std::vector<BTreeNode<T, B>*> BTreeNode<T, B>::find_nodes_at_level(size_t lv) const {
    std::vector<BTreeNode<T, B>*> nodes;

    if (lv == 0) {
        nodes.emplace_back(const_cast<BTreeNode<T, B>*>(this));
        return nodes;
    } else {
        std::vector<BTreeNode<T, B>*> tmp;
        for (auto i = 0; i < n + 1; i++) {
            tmp = edges[i]->find_nodes_at_level(lv - 1);
            std::copy(tmp.begin(), tmp.end(), std::back_inserter(nodes));
        }

        return nodes;
    }
}

template<typename T, size_t B>
BTreeNode<T, B>::BTreeNode() : n(0), type(NodeType::LEAF) {}

template<typename T, size_t B>
BTreeNode<T, B>::BTreeNode(const T& t) : n(1), type(NodeType::LEAF) {
    keys[0] = t;
}

/* Assume the input initializer list is sorted */
template<typename T, size_t B>
BTreeNode<T, B>::BTreeNode(std::initializer_list<T> l)
    : n(l.size()), type(NodeType::LEAF) {
    std::copy(l.begin(), l.end(), keys.begin());
}

/* Assume the input iterator is sorted. */
template<typename T, size_t B>
template<typename InputIt>
BTreeNode<T, B>::BTreeNode(InputIt begin, InputIt end)
    : n(end - begin), type(NodeType::LEAF) {
    std::copy(begin, end, keys.begin());
}

template<typename T, size_t B>
BTreeNode<T, B>::~BTreeNode() {
    if (this->type == NodeType::LEAF)
        return;

    for (auto i = 0; i < n + 1; i++)
        if (edges[i]) delete edges[i];
}
