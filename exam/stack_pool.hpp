#include <iostream>
#include <vector>
#include "ap_error.hpp"

/**
 * A simple class to handle wrong inputs
 */
struct Invalind_input: public std::runtime_error{
  using std::runtime_error::runtime_error; // using the same constructors
                                           // of the parent
};

template <typename SP, typename ST, typename V>
class _iterator {
  SP* pool;
  ST current;  // current head
 public:
  using value_type = V;
  using stack_type = ST;
  using stack_pool = SP;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  explicit _iterator(stack_pool* p, stack_type head) : pool{p}, current{head} {}

  reference operator*() const noexcept { return pool->value(current); }

  _iterator& operator++() {  // pre-increment
    current = pool->next(current);
    return *this;
  }

  _iterator operator++(int) {  // post_increment
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const _iterator& x, const _iterator& y) noexcept {
    return x.current == y.current;
  }

  friend bool operator!=(const _iterator& x, const _iterator& y) noexcept {
    return !(x == y);
  }
};

template <typename T, typename N = std::size_t>
class stack_pool {
  struct node_t {
    T value;
    N next;

    node_t() = default;

    // should I check next>=0 ?
    template <typename X>
    node_t(X&& val, N next) : value{std::forward<X>(val)}, next{next} {};

    ~node_t() = default;
  };
  std::vector<node_t> pool;
  using stack_type = N;
  using value_type = T;
  using size_type = typename std::vector<node_t>::size_type;
  stack_type free_nodes;  // at the beginning, it is empty

  node_t& node(stack_type x) noexcept { return pool[x - 1]; }
  const node_t& node(stack_type x) const noexcept { return pool[x - 1]; }

  /**
   * Function to push a new value in the stack
   * If the first empty node is the last then we create a new node directly inside our pool with emplace_back
   * If the first empty node is not the last this means that we have some free space somewhere, so there is no need to create a node.
   * 
   */
  template <typename X>
  stack_type _push(X&& val, stack_type head) {
    if (empty(free_nodes)) {
      pool.emplace_back(std::forward<X>(val), head);
      return static_cast<stack_type>(pool.size()); 
    } else {
      stack_type new_head = free_nodes;
      free_nodes = next(free_nodes);  // we update free_nodes with the next position
      value(new_head) = std::forward<X>(val);
      next(new_head) = head;  // the next of the new_head is the previous head
      return new_head;
    }
  }

 public:
  stack_pool() : pool{}, free_nodes{end()} {};

  explicit stack_pool(size_type n) : stack_pool{} {
    reserve(n);
  };  // reserve n nodes in the pool

  ~stack_pool() = default;

  using iterator = _iterator<stack_pool, stack_type, value_type>;
  using const_iterator = _iterator<stack_pool, stack_type, const value_type>;

  iterator begin(stack_type x) { return iterator(this, x); }
  iterator end(stack_type) { return iterator(this, end()); } // this is not a typo

  const_iterator begin(stack_type x) const { return const_iterator(this, x); }
  const_iterator end(stack_type) const  { return const_iterator(this, end()); }

  const_iterator cbegin(stack_type x) const { return const_iterator(this, x); }
  const_iterator cend(stack_type) const { return const_iterator(this, end()); }

  /**
   * Create an empty stack. Initial value is (std::size_t(0)).
   */
  stack_type new_stack() noexcept { return end(); };  // return an empty stack

  void reserve(size_type n) {
    AP_ASSERT_GT(n,0);
    AP_ASSERT_GT(n, capacity());
    pool.reserve(n);
  };  // reserve n nodes in the pool


  /**
   * @brief gives the capacity of the pool
   */
  size_type capacity() const {  // the capacity of the pool
    return pool.capacity();
  }

  /**
   * @brief Checking if the given stack is empty or not
   */
  bool empty(stack_type x) const {  // forse noexcept
    return x == end();
  }

  /**
   * @brief Returning the value 0, which is the index of the last node in the vector. 
  */
  stack_type end() const noexcept { return stack_type(0); }

  
  /**
   * @brief Returning a reference to the value of the node. 
  */
  T& value(stack_type x) { return node(x).value; }
  const T& value(stack_type x) const { return node(x).value; }

  /**
   * @brief Returning a reference to the next node of the node x. 
  */
  stack_type& next(stack_type x) { return node(x).next; }
  const stack_type& next(stack_type x) const { return node(x).next; }

  stack_type push(const T& val, stack_type head) { return _push(val, head); }
  stack_type push(T&& val, stack_type head) {
    return _push(std::move(val), head);
  }

  /**
   * Function to remove from the stack the last inserted node, that will be put in the free_nodes
  */
  stack_type pop(stack_type x) {
    // what if x is empty, does not exists or it is free_nodes?
    if (empty(x)) {
      return end();
    } else {
      auto new_head = next(x);
      next(x) = free_nodes;
      free_nodes = x;
      return new_head;
    }
  }  // delete first node

  /**
   * Function to free a single stack. The stack is resetted to its initial
   * value (std::size_t(0)). Its nodes are added to free_nodes.
   */
  stack_type free_stack(stack_type x) {
    // what if x does not exist or it is free nodes?
    while (!empty(x)) {
      x = this->pop(x);
    }
    return (x);
  } // free entire stack


  /**
   * Function to print a single stack
  */
  void print_stack(stack_type x) {
    auto start = begin(x);
    std::cout << "[ ";
    while (start != end(x)) {
        std::cout << *start << " ";
        ++start;
    }
    std::cout << "]" << std::endl;
  }

  /**
   * Function to print the content of the pool
  */
  void print_pool() {
    std::cout << "pool = [ ";
    for (auto i = 0; i < pool.size(); ++i) {
        std::cout << pool.at(i).value << " ";
    }
    std::cout << "]" << std::endl;
  }
};