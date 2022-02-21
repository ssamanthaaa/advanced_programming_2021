#include <iostream>
#include <vector>

template <typename T, typename N = std::size_t>
class stack_pool {
  struct node_t {
    T value;
    N next;

    node_t() = default;

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
   * If the first empty node is the last then we create a new node, we used emplace_back so the object is constructed directly
   * If the frist empty node is not the last this means that we have some free space somewhere.
   */
  template <typename X>
  stack_type _push(X&& val, stack_type head) {
    // I have to push into the vector a new node
    if (free_nodes == end()) { // == empty(free_nodes)
      pool.emplace_back(std::forward<X>(val));
      return pool.size();// next free node is the last
    } 
    else { // we have a free space somewhere
      stack_type new_head = free_nodes; 
      free_nodes = next(free_nodes); // we update free_nodes with the next position
      value(new_head) =std::forward<X>(val);
      next(new_head) = head; // the next of the new_head is the previous head
      return new_head;
    }
  }

 public:
  stack_pool() : pool{}, free_nodes{end()} {};

  explicit stack_pool(size_type n);  // reserve n nodes in the pool

  ~stack_pool() = default;

  using iterator = ...;
  using const_iterator = ...;

  iterator begin(stack_type x);
  iterator end(stack_type);  // this is not a typo

  const_iterator begin(stack_type x) const;
  const_iterator end(stack_type) const;

  const_iterator cbegin(stack_type x) const;
  const_iterator cend(stack_type) const;

  stack_type new_stack();  // return an empty stack

  void reserve(size_type n);   // reserve n nodes in the pool
  size_type capacity() const { // the capacity of the pool
    // return pool.capacity();
  }
  bool empty(stack_type x) const { // forse noexcept
    return x == end();
  }

  stack_type end() const noexcept { return stack_type(0); }

  T& value(stack_type x) { return node(x).value; }
  const T& value(stack_type x) const { return node(x).value; }

  stack_type& next(stack_type x) { return node(x).next; }
  const stack_type& next(stack_type x) const { return node(x).next; }

  stack_type push(const T& val, stack_type head) {
    return _push(val, head);
  }
  stack_type push(T&& val, stack_type head) {
    return _push(std::move(val), head);
  }

  stack_type pop(stack_type x);  // delete first node

  stack_type free_stack(stack_type x);  // free entire stack
};