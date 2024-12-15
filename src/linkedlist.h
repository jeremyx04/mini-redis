#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <string>

struct Node {
  std::string val;
  Node *next;
  Node *prev;

  Node(const std::string &val) : val(val), next(nullptr), prev(nullptr) {}
};

// Doubly Linked List
class LinkedList {
  Node* head;
  Node* tail;
  size_t sz; 
 public:
  LinkedList() : head(nullptr), tail(nullptr), sz(0) {}
  ~LinkedList();
  void push_front(const std::string &val);
  void push_back(const std::string &val);
  std::string at(int idx);
  std::string pop_front();
  std::string pop_back();
  size_t get_size() const;
};

#endif
