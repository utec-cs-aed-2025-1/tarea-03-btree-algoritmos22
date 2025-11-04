#ifndef BTree_H
#define BTree_H
#include <iostream>
#include "node.h"
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

template <typename TK>
class BTree {
 private:
  Node<TK>* root;
  int M;
  int n;

 public:
  BTree(int _M) : root(nullptr), M(_M), n(0) {}

  bool search(TK key);
  void insert(TK key);
  void remove(TK key);
  int height();
  string toString(const string& sep);
  vector<TK> rangeSearch(TK begin, TK end);

  TK minKey();
  TK maxKey();
  void clear();
  int size();
  
  static BTree* build_from_ordered_vector(vector<TK> elements, int M);
  bool check_properties();

  ~BTree();
};

#include "btree_impl.h"

#endif