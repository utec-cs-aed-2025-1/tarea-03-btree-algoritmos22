#ifndef NODE_H
#define NODE_H

using namespace std;

template <typename TK>
struct Node {
  TK* keys;
  Node** children;
  int count;
  bool leaf;

  Node() : keys(nullptr), children(nullptr), count(0), leaf(true) {}
  Node(int M) {
    keys = new TK[M - 1];
    children = new Node<TK>*[M];
    for (int i = 0; i < M; i++) {
        children[i] = nullptr;
    }
    count = 0;
    leaf = true;
  }

  void killSelf();
};

#endif