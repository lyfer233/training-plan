#ifndef SKIPLIST_H_
#define SKIPLIST_H_

#include <cassert>
#include <cstdlib>
#include <vector>
#include "util/random.h"

namespace skiplist {

template <typename Key, class Comparator>
class SkipList {
 private:
  struct Node;

 public:
  // Create a new SkipList object that will use "cmp" for comparing keys,
  explicit SkipList(Comparator cmp) : compare_(cmp), head_(new Node(0, kMaxHeight)), rnd_(Random(114514)), max_height_(1) {}

  SkipList(const SkipList&) = delete;
  SkipList& operator=(const SkipList&) = delete;

  // Insert key into the list.
  // REQUIRES: nothing that compares equal to key is currently in the list.
  void Insert(const Key& key);
  // Remove key in the list iff the list contains the key
  bool Remove(const Key& key);
  // Returns true iff an entry that compares equal to key is in the list.
  bool Contains(const Key& key) const;

  // Iteration over the contents of a skip list
  class Iterator {
   public:
    // Initialize an iterator over the specified list.
    // The returned iterator is not valid.
    explicit Iterator(const SkipList* list) : list_(list), node_(list->head_), prevNode_(nullptr) {}

    // Returns true iff the iterator is positioned at a valid node.
    // valid indicates the iterator is not nullptr
    bool Valid() const {
      return node_ != nullptr;
    }

    // Returns the key at the current position.
    // REQUIRES: Valid()
    const Key& key() const {
      return node_->key;
    }

    // Advances to the next position.
    // REQUIRES: Valid()
    void Next() {
      prevNode_ = node_;
      node_ = node_->Next(1);
    }

    // Advances to the previous position.
    // REQUIRES: Valid()
    void Prev() {
      node_ = prevNode_;
    }

    // Advance to the first entry with a key >= target
    void Seek(const Key& target) {
      while (node_ != nullptr && node_->key < target) {
        prevNode_ = node_;
        node_ = node_->Next(1);
      }
    }

    // Position at the first entry in list.
    // Final state of iterator is Valid() iff list is not empty.
    void SeekToFirst() {
      prevNode_ = nullptr;
      node_ = list_->head_;
    }

    // Position at the last entry in list.
    // Final state of iterator is Valid() iff list is not empty.
    void SeekToLast() {
      while (node_->Next(1) != nullptr) {
        prevNode_ = node_;
        node_ = node_->Next(1);
      }
    }

   private:
    const SkipList* list_;
    Node* node_;
    Node* prevNode_;
  };

 private:
  enum { kMaxHeight = 12 };

  inline int GetMaxHeight() const { return max_height_; }

  Node* NewNode(const Key& key, int height) { return new Node(key, height); }
  int RandomHeight();
  bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }

  // Return true if key is greater than the data stored in "n"
  bool KeyIsAfterNode(const Key& key, Node* n) const;

  // Return the earliest node that comes at or after key.
  // Return nullptr if there is no such node.
  //
  // If prev is non-null, fills prev[level] with pointer to previous
  // node at "level" for every level in [0..max_height_-1].
  Node* FindGreaterOrEqual(const Key& key, Node** prev) const;

  // Return the latest node with a key < key.
  // Return head_ if there is no such node.
  Node* FindLessThan(const Key& key) const;

  // Return the last node in the list.
  // Return head_ if list is empty.
  Node* FindLast() const;

  // Immutable after construction
  Comparator const compare_;

  Node* const head_;

  // Modified only by Insert().
  int max_height_;  // Height of the entire list

  // Read/written only by Insert().
  Random rnd_;
};

// Implementation details follow
template <typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
  Node(const Key& k, int height) : key(k) {
    // resize the next_ array of size height
    for (int i = 0; i < height; i++) {
      next_[i] = nullptr;
    }
  }

  explicit Node() : key(0) {}
  Key const key;

  // Accessors/mutators for links.
  Node* Next(int n) {
    // return the next level n of this Node
    return next_[n];
  }
  void SetNext(int n, Node* x) {
    // set the next Node of level n
    next_[n] = x;
  }

 private:
  // Array of length equal to the node height.
  std::vector<Node*> next_;
};

// Implement your code

// Decide the level of a node.
template <typename Key, class Comparator>
int SkipList<Key, Comparator>::RandomHeight() {
  int level = 1;
  // 1/4 probability increase
  while (rnd_.Uniform(4) == 0 && level < kMaxHeight) {
    level += 1;
  }
  return level;
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert(const Key& key) {
  // Will insert previous Node
  Node* preNode[kMaxHeight];
  // The start node
  Node* start = head_;
  // Iterate from high to low.
  for (int height = max_height_; height >= 1; height--) {
    // Iterate from high to low.
    while (start->Next(height) != nullptr && start->Next(height)->key < key) {
      start = start->Next(height);
    }
    preNode[height] = start;
  }
  start = start->Next(0);

  // If key has exist, then doing.
  if (start != nullptr && start->key == key) {
    return;
  }

  // insert node
  int random_height = RandomHeight();
  if (random_height > max_height_) {
    // new node should take link with head
    for (int i = max_height_ + 1; i <= random_height; i++) {
      head_->SetNext(i, NewNode(key, i));
    }
    // Meantime we should update the max_height_
    max_height_ = random_height;
  }
  // insert a node in list
  for (int i = 1; i <= max_height_; i++) {
    Node* newNode = NewNode(key, i);
    newNode->SetNext(i, preNode[i]->Next(i));
    preNode[i]->SetNext(i, newNode);
  }

}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::Remove(const Key& key) {
  // Will insert previous Node
  Node* preNode[kMaxHeight];
  // The start node
  Node* start = head_;
  // Iterate from high to low.
  for (int height = max_height_; height >= 1; height--) {
    // Iterate from high to low.
    while (start->Next(height) != nullptr && start->Next(height)->key < key) {
      start = start->Next(height);
    }
    preNode[height] = start;
  }
  start = start->Next(0);

  // If key has exist, then doing.
  if (start != nullptr && start->key == key) {
    return false;
  }
  for (int i = 1; i <= max_height_; i++) {
    preNode[i]->SetNext(i, start->Next(i));
  }
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains(const Key& key) const {
  // The start node
  Node* start = head_;
  // Iterate from high to low.
  for (int height = kMaxHeight; height >= 1; height--) {
    // Iterate from left to right.
    while (start->Next(height) != nullptr && start->Next(height)->key < key) {
      start = start->Next(height);
    }
  }
  start = start->Next(0);

  return start != nullptr and start->key == key;
}


}  // namespace skiplist

#endif  // DSCL_SKIPLIST_H_
