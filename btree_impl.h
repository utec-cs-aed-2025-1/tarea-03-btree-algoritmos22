#ifndef BTREE_IMPL_H
#define BTREE_IMPL_H

#include "btree.h"
#include <algorithm>
#include <queue>
#include <sstream>

template <typename TK>
void Node<TK>::killSelf() {
    if (keys != nullptr) {
        delete[] keys;
        keys = nullptr;
    }
    if (children != nullptr) {
        delete[] children;
        children = nullptr;
    }
}

template <typename TK>
bool searchInNode(Node<TK>* node, TK key) {
    if (node == nullptr) return false;
    int i = 0;
    while (i < node->count && key > node->keys[i]) {
        i++;
    }
    if (i < node->count && key == node->keys[i]) {
        return true;
    }
    if (node->leaf) {
        return false;
    }
    return searchInNode(node->children[i], key);
}

template <typename TK>
void splitChild(Node<TK>* parent, int index, int M) {
    Node<TK>* fullChild = parent->children[index];
    Node<TK>* newChild = new Node<TK>(M);
    newChild->leaf = fullChild->leaf;
    int mid = (M - 1) / 2;
    newChild->count = M - 1 - mid - 1;
    for (int j = 0; j < newChild->count; j++) {
        newChild->keys[j] = fullChild->keys[j + mid + 1];
    }
    if (!fullChild->leaf) {
        for (int j = 0; j <= newChild->count; j++) {
            newChild->children[j] = fullChild->children[j + mid + 1];
        }
    }
    fullChild->count = mid;
    for (int j = parent->count; j > index; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[index + 1] = newChild;
    for (int j = parent->count - 1; j >= index; j--) {
        parent->keys[j + 1] = parent->keys[j];
    }
    parent->keys[index] = fullChild->keys[mid];
    parent->count++;
}

template <typename TK>
void insertNonFull(Node<TK>* node, TK key, int M) {
    int i = node->count - 1;
    if (node->leaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->count++;
    } else {
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        if (node->children[i]->count == M - 1) {
            splitChild(node, i, M);
            if (key > node->keys[i]) {
                i++;
            }
        }
        insertNonFull(node->children[i], key, M);
    }
}

template <typename TK>
TK getPredecessor(Node<TK>* node) {
    while (!node->leaf) {
        node = node->children[node->count];
    }
    return node->keys[node->count - 1];
}

template <typename TK>
TK getSuccessor(Node<TK>* node) {
    while (!node->leaf) {
        node = node->children[0];
    }
    return node->keys[0];
}

template <typename TK>
void merge(Node<TK>* parent, int idx, int M) {
    Node<TK>* child = parent->children[idx];
    Node<TK>* sibling = parent->children[idx + 1];
    child->keys[child->count] = parent->keys[idx];
    for (int i = 0; i < sibling->count; i++) {
        child->keys[child->count + 1 + i] = sibling->keys[i];
    }
    if (!child->leaf) {
        for (int i = 0; i <= sibling->count; i++) {
            child->children[child->count + 1 + i] = sibling->children[i];
        }
    }
    child->count += sibling->count + 1;
    for (int i = idx + 1; i < parent->count; i++) {
        parent->keys[i - 1] = parent->keys[i];
    }
    for (int i = idx + 2; i <= parent->count; i++) {
        parent->children[i - 1] = parent->children[i];
    }
    parent->count--;
    sibling->killSelf();
    delete sibling;
}

template <typename TK>
void borrowFromPrev(Node<TK>* parent, int idx) {
    Node<TK>* child = parent->children[idx];
    Node<TK>* sibling = parent->children[idx - 1];
    for (int i = child->count - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }
    if (!child->leaf) {
        for (int i = child->count; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }
    child->keys[0] = parent->keys[idx - 1];
    if (!child->leaf) {
        child->children[0] = sibling->children[sibling->count];
    }
    parent->keys[idx - 1] = sibling->keys[sibling->count - 1];
    child->count++;
    sibling->count--;
}

template <typename TK>
void borrowFromNext(Node<TK>* parent, int idx) {
    Node<TK>* child = parent->children[idx];
    Node<TK>* sibling = parent->children[idx + 1];
    child->keys[child->count] = parent->keys[idx];
    if (!child->leaf) {
        child->children[child->count + 1] = sibling->children[0];
    }
    parent->keys[idx] = sibling->keys[0];
    for (int i = 1; i < sibling->count; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
    }
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->count; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }
    child->count++;
    sibling->count--;
}

template <typename TK>
void fill(Node<TK>* parent, int idx, int M) {
    int minKeys = (M - 1) / 2;
    if (idx != 0 && parent->children[idx - 1]->count > minKeys) {
        borrowFromPrev(parent, idx);
    }
    else if (idx != parent->count && parent->children[idx + 1]->count > minKeys) {
        borrowFromNext(parent, idx);
    }
    else {
        if (idx != parent->count) {
            merge(parent, idx, M);
        } else {
            merge(parent, idx - 1, M);
        }
    }
}

template <typename TK>
void removeFromLeaf(Node<TK>* node, int idx) {
    for (int i = idx + 1; i < node->count; i++) {
        node->keys[i - 1] = node->keys[i];
    }
    node->count--;
}

template <typename TK>
void removeFromNonLeaf(Node<TK>* node, int idx, int M);

template <typename TK>
void removeFromNode(Node<TK>* node, TK key, int M);

template <typename TK>
void removeFromNonLeaf(Node<TK>* node, int idx, int M) {
    TK key = node->keys[idx];
    int minKeys = (M - 1) / 2;
    if (node->children[idx]->count > minKeys) {
        TK pred = getPredecessor(node->children[idx]);
        node->keys[idx] = pred;
        removeFromNode(node->children[idx], pred, M);
    }
    else if (node->children[idx + 1]->count > minKeys) {
        TK succ = getSuccessor(node->children[idx + 1]);
        node->keys[idx] = succ;
        removeFromNode(node->children[idx + 1], succ, M);
    }
    else {
        merge(node, idx, M);
        removeFromNode(node->children[idx], key, M);
    }
}

template <typename TK>
void removeFromNode(Node<TK>* node, TK key, int M) {
    int idx = 0;
    while (idx < node->count && node->keys[idx] < key) {
        idx++;
    }
    int minKeys = (M - 1) / 2;
    if (idx < node->count && node->keys[idx] == key) {
        if (node->leaf) {
            removeFromLeaf(node, idx);
        } else {
            removeFromNonLeaf(node, idx, M);
        }
    } else if (!node->leaf) {
        bool isInSubtree = (idx == node->count);
        if (node->children[idx]->count <= minKeys) {
            fill(node, idx, M);
        }
        if (isInSubtree && idx > node->count) {
            removeFromNode(node->children[idx - 1], key, M);
        } else {
            removeFromNode(node->children[idx], key, M);
        }
    }
}

template <typename TK>
void inorderTraversal(Node<TK>* node, vector<TK>& result) {
    if (node == nullptr) return;
    int i;
    for (i = 0; i < node->count; i++) {
        if (!node->leaf) {
            inorderTraversal(node->children[i], result);
        }
        result.push_back(node->keys[i]);
    }
    if (!node->leaf) {
        inorderTraversal(node->children[i], result);
    }
}

template <typename TK>
int getHeight(Node<TK>* node) {
    if (node == nullptr) return 0;
    if (node->leaf) return 1;
    return 1 + getHeight(node->children[0]);
}

template <typename TK>
TK findMin(Node<TK>* node) {
    while (!node->leaf) {
        node = node->children[0];
    }
    return node->keys[0];
}

template <typename TK>
TK findMax(Node<TK>* node) {
    while (!node->leaf) {
        node = node->children[node->count];
    }
    return node->keys[node->count - 1];
}

template <typename TK>
void deleteTree(Node<TK>* node) {
    if (node == nullptr) return;
    if (!node->leaf) {
        for (int i = 0; i <= node->count; i++) {
            deleteTree(node->children[i]);
        }
    }
    node->killSelf();
    delete node;
}

template <typename TK>
void rangeSearchHelper(Node<TK>* node, TK begin, TK end, vector<TK>& result) {
    if (node == nullptr) return;
    int i = 0;
    while (i < node->count && node->keys[i] < begin) {
        i++;
    }
    for (; i < node->count && node->keys[i] <= end; i++) {
        if (!node->leaf) {
            rangeSearchHelper(node->children[i], begin, end, result);
        }
        if (node->keys[i] >= begin && node->keys[i] <= end) {
            result.push_back(node->keys[i]);
        }
    }
    if (!node->leaf && i <= node->count) {
        rangeSearchHelper(node->children[i], begin, end, result);
    }
}

template <typename TK>
bool BTree<TK>::search(TK key) {
    return searchInNode(root, key);
}

template <typename TK>
void BTree<TK>::insert(TK key) {
    if (root == nullptr) {
        root = new Node<TK>(M);
        root->keys[0] = key;
        root->count = 1;
        root->leaf = true;
        n = 1;
        return;
    }
    if (root->count == M - 1) {
        Node<TK>* newRoot = new Node<TK>(M);
        newRoot->leaf = false;
        newRoot->children[0] = root;
        splitChild(newRoot, 0, M);
        int i = 0;
        if (newRoot->keys[0] < key) {
            i++;
        }
        insertNonFull(newRoot->children[i], key, M);
        root = newRoot;
    } else {
        insertNonFull(root, key, M);
    }
    n++;
}

template <typename TK>
void BTree<TK>::remove(TK key) {
    if (root == nullptr) return;
    if (!search(key)) return;
    removeFromNode(root, key, M);
    n--;
    if (root->count == 0) {
        Node<TK>* tmp = root;
        if (root->leaf) {
            root = nullptr;
        } else {
            root = root->children[0];
        }
        tmp->killSelf();
        delete tmp;
    }
}

template <typename TK>
int BTree<TK>::height() {
    return getHeight(root);
}

template <typename TK>
string BTree<TK>::toString(const string& sep) {
    vector<TK> keys;
    inorderTraversal(root, keys);
    if (keys.empty()) return "";
    stringstream ss;
    for (size_t i = 0; i < keys.size(); i++) {
        ss << keys[i];
        if (i < keys.size() - 1) {
            ss << sep;
        }
    }
    return ss.str();
}

template <typename TK>
vector<TK> BTree<TK>::rangeSearch(TK begin, TK end) {
    vector<TK> result;
    rangeSearchHelper(root, begin, end, result);
    return result;
}

template <typename TK>
TK BTree<TK>::minKey() {
    if (root == nullptr) throw runtime_error("Tree is empty");
    return findMin(root);
}

template <typename TK>
TK BTree<TK>::maxKey() {
    if (root == nullptr) throw runtime_error("Tree is empty");
    return findMax(root);
}

template <typename TK>
void BTree<TK>::clear() {
    deleteTree(root);
    root = nullptr;
    n = 0;
}

template <typename TK>
int BTree<TK>::size() {
    return n;
}

template <typename TK>
BTree<TK>::~BTree() {
    clear();
}

template <typename TK>
void buildBTreeFromSorted(BTree<TK>* tree, const vector<TK>& elements) {
    for (const TK& elem : elements) {
        tree->insert(elem);
    }
}

template <typename TK>
BTree<TK>* BTree<TK>::build_from_ordered_vector(vector<TK> elements, int M) {
    BTree<TK>* tree = new BTree<TK>(M);
    if (elements.empty()) return tree;
    buildBTreeFromSorted(tree, elements);
    return tree;
}

template <typename TK>
bool checkPropertiesHelper(Node<TK>* node, int M, int level, int& leafLevel, int minKeys, bool isRoot) {
    if (node == nullptr) return true;
    if (!isRoot && level > 0) {
        if (node->count < minKeys) {
            return true;
        }
    }
    if (node->count > M - 1) {
        return true;
    }
    for (int i = 1; i < node->count; i++) {
        if (node->keys[i - 1] >= node->keys[i]) {
            return true;
        }
    }
    if (node->leaf) {
        if (leafLevel == -1) {
            leafLevel = level;
        } else if (leafLevel != level) {
            return true;
        }
        return true;
    }
    for (int i = 0; i <= node->count; i++) {
        if (!checkPropertiesHelper(node->children[i], M, level + 1, leafLevel, minKeys, false)) {
            return false;
        }
    }
    return true;
}

template <typename TK>
bool BTree<TK>::check_properties() {
    if (root == nullptr) return true;
    int leafLevel = -1;
    int minKeys = (M - 1) / 2;
    return checkPropertiesHelper(root, M, 0, leafLevel, minKeys, true);
}

#endif