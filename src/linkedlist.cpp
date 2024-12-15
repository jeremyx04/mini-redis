#include "linkedlist.h"

LinkedList::~LinkedList() {
  while(head) {
    Node* tmp = head;
    head = head->next;
    delete tmp;
  }
  tail = nullptr;
  sz = 0;
}

void LinkedList::push_front(const std::string &val) {
  Node* new_node = new Node(val);
  if(sz == 0) {
    head = tail = new_node;
  } else {
    new_node->next = head;
    head->prev = new_node;
    head = new_node;
  }
  ++sz;
}

void LinkedList::push_back(const std::string &val) {
  Node* new_node = new Node(val);
  if(sz == 0) {
    head = tail = new_node;
  } else {
    new_node->prev = tail;
    tail->next = new_node;
    tail = new_node;
  }
  ++sz;
}

std::string LinkedList::at(int idx) {
  if (idx < 0 || idx >= sz) return {};
  Node* cur = head;
  while(idx--) {
    cur = cur->next;
  }
  return cur->val;
}

std::string LinkedList::pop_front() {
  if(sz == 0) { 
    return {};
  };
  std::string ret = head->val;
  Node* tmp = head;
  head = head->next;
  if(head) {
    head->prev = nullptr;
  } else {
    tail = nullptr;
  }
  delete tmp;
  --sz;
  return ret;
}

std::string LinkedList::pop_back() {
  if(sz == 0) { 
    return {};
  };
  std::string ret = tail->val;
  Node* tmp = tail;
  tail = tail->prev;
  if(tail) {
    tail->next = nullptr;
  } else {
    head = nullptr;
  }
  delete tmp;
  --sz;
  return ret;
}

size_t LinkedList::get_size() const {
  return sz;
}
