//
// Created by pvelp on 4/15/22.
//
#ifndef TEMPLATE_MQUEUE_HPP
#define TEMPLATE_MQUEUE_HPP
#include <queue>
#include <iostream>
#include <mutex>

template <typename T>
class Queue{

 private:
  size_t count;
  std::mutex mutex;
  std::queue<T> queue;

 public:
  Queue() : count(0) {};

  void push(T&& val){
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(val);
    count++;
  }

  T front(){
    std::lock_guard<std::mutex> lock(mutex);
    T tmp = queue.front();
    return tmp;
  }

  void pop(){
    std::lock_guard<std::mutex> lock(mutex);
    queue.pop();
    count--;
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
  }

  size_t get_count(){
    return count;
  }
};

#endif  // TEMPLATE_MQUEUE_HPP
