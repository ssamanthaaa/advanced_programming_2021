#include "ap_error.hpp"
#include <iostream>
#include <vector>
#include <iterator>

/**
 * @brief An ERROR to check if the passed stack to a method is valid.
 *
 * In this case the stack is considered valid also if its head is empty. Of
 * course it can not be greater than the index of the last node of the pool.
 *
 */
#define AP_ERROR_CUSTOM_POSITIVE(a)                                            \
  AP_ERROR(((a >= size_type{end()}) && (a <= pool.size())), Invalid_input)     \
      << "The stack must be >= " << end() << " and <= " << pool.size() << "."  \
      << "\n"                                                                  \
      << "You have passed: " << a << "." << std::endl;

/**
 * @brief An ERROR to check if the passed stack to a method is valid.
 *
 * In this case if the stack is empty it is not considered valid. Of course it
 * can not be greater than the index of the last node of the pool.
 *
 */
#define AP_ERROR_CUSTOM_STRICT_POSITIVE(a)                                     \
  AP_ERROR(((a > size_type{end()}) && (a <= pool.size())), Invalid_input)      \
      << "The stack must be > " << end() << " and <= " << pool.size() << "."   \
      << "\n"                                                                  \
      << "You have passed: " << a << "." << std::endl;

/**
 * @brief A simple class to handle wrong inputs
 */
struct Invalid_input : public std::runtime_error {
  using std::runtime_error::runtime_error;  // using the same constructors
                                            // of the parent
};

/**
 * @brief A class to iterate through the stack_pool
 *
 * The class is templated on stack_pool, stack_type and value_type
 */
template <typename SP, typename ST, typename V>
class _iterator {
  /** A pointer to the stack_pool that contains the stack on which we iterate */
  SP* pool;

  /** The index in the pool of the current stack's node */
  ST current;  // current head
 public:
  using value_type = V;
  using stack_type = ST;
  using stack_pool = SP;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  /** Constructor of the class */
  explicit _iterator(stack_pool* p, stack_type head) : pool{p}, current{head} {}

  /** Dereference operator to get the value of the current node */
  reference operator*() const noexcept { return pool->value(current); }

  /** A pre-increment operator. It is used to increment the iterator before
   * returning the value */
  _iterator& operator++() {  // pre-increment
    current = pool->next(current);
    return *this;
  }

  /** A post-increment operator. It is used to increment the iterator before
   * returning the value. */
  _iterator operator++(int) {  // post_increment
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  /** Equal-equal operator for 2 iterators (return true if they "points" to the
   * same node) */
  friend bool operator==(const _iterator& x, const _iterator& y) noexcept {
    return x.current == y.current;
  }

  /** Not-equal operator for 2 iterators (return true if they are "pointing" to
   * differen nodes) */
  friend bool operator!=(const _iterator& x, const _iterator& y) noexcept {
    return !(x == y);
  }
};

/** @brief A template class that takes two template parameters.*/
template <typename T, typename N = std::size_t>
class stack_pool {
  /* A struct that contains two members: value and next. */
  struct node_t {
    T value;
    N next;

    node_t() = default;

    /**
     * @brief A constructor of the node_t struct. It is used to create a new
     * node in the pool.
     */
    template <typename X>
    node_t(X&& val, N next) : value{std::forward<X>(val)}, next{next} {};

    ~node_t() = default;
  };
  std::vector<node_t> pool;
  using stack_type = N;
  using value_type = T;
  using size_type = typename std::vector<node_t>::size_type;
  stack_type free_nodes;  // at the beginning, it is empty

  /**
   * @brief  Returning a reference to the node_t struct that is stored in the
   * pool at the index x-1.
   */
  node_t& node(stack_type x) noexcept { return pool[x - 1]; }
  /**
   * @brief Returning a const reference to the node_t struct that is stored in
   * the pool at the index x-1.
   */
  const node_t& node(stack_type x) const noexcept { return pool[x - 1]; }

  /**
   * @brief Function to push a new value in the stack
   *
   * If the first empty node is the last then we create a new node directly
   * inside our pool with emplace_back If the frist empty node is not the last
   * this means that we have some free space somewhere, so there is no need to
   * create a node.
   */
  template <typename X>
  stack_type _push(X&& val, stack_type head) {
    AP_ASSERT(head >= end(), Invalid_input)
        << "The head of a stack can not be negative" << std::endl;
    if (empty(free_nodes)) {
      pool.emplace_back(std::forward<X>(val), head);
      return static_cast<stack_type>(pool.size());
    } else {
      stack_type new_head = free_nodes;
      free_nodes =
          next(free_nodes);  // we update free_nodes with the next position
      value(new_head) = std::forward<X>(val);
      next(new_head) = head;  // the next of the new_head is the previous head
      return new_head;
    }
  }

 public:
  /**
   * @brief Construct a new stack pool object
   */
  stack_pool() noexcept : pool{}, free_nodes{end()} {};

  /**
   * @brief Construct a new stack pool object with capactiy equal to n
   */
  explicit stack_pool(size_type n) : stack_pool{} {
    AP_ERROR(n >= end(), Invalid_input)
        << "You can not reserve negative space in the pool" << std::endl;
    reserve(n);
  };  // reserve n nodes in the pool

  ~stack_pool() = default;

  using iterator =
      _iterator<stack_pool<value_type, stack_type>, stack_type, value_type>;
  using const_iterator = _iterator<const stack_pool<value_type, stack_type>,
                                   stack_type,
                                   const value_type>;

  /**
   * @brief Returning an iterator to the beginning of the stack.
   */
  iterator begin(stack_type x) {
    AP_ERROR_CUSTOM_POSITIVE(x);
    return iterator(this, x);
  }

  /**
   * @brief Returning an iterator to the end of the stack
   *
   */
  iterator end(stack_type) noexcept {
    return iterator(this, end());
  }  // this is not a typo

  /**
   * @brief Returning a const_iterator to the beginning of the stack.
   */
  const_iterator begin(stack_type x) const {
    AP_ERROR_CUSTOM_POSITIVE(x);
    return const_iterator(this, x);
  }

  /**
   * @brief Returning a const_iterator to the end of the stack.
   */
  const_iterator end(stack_type) const noexcept {
    return const_iterator(this, end());
  }

  /**
   * @brief Returning a const_iterator to the beginning of the stack.
   */
  const_iterator cbegin(stack_type x) const {
    AP_ERROR_CUSTOM_POSITIVE(x);
    return const_iterator(this, x);
  }

  /**
   * @brief Returning a const_iterator to the end of the stack.
   */
  const_iterator cend(stack_type) const noexcept {
    return const_iterator(this, end());
  }

  /**
   * @brief Create an empty stack. Initial value is (std::size_t(0)).
   */
  stack_type new_stack() noexcept { return end(); };  // return an empty stack

  /**
   * @brief A function that reserves n nodes in the pool
   */
  void reserve(size_type n) {
    AP_ERROR(n >= end(), Invalid_input)
        << "You can not reserve negative space in the pool" << std::endl;
    pool.reserve(n);
  };  // reserve n nodes in the pool

  /**
   * @brief Gives the capacity of the pool
   */
  size_type capacity() const noexcept {  // the capacity of the pool
    return pool.capacity();
  }

  /**
   * @brief Checking if the given stack is empty or not
   */
  bool empty(stack_type x) const {  // forse noexcept
    AP_ERROR_CUSTOM_POSITIVE(x);
    return x == end();
  }

  /**
   * @brief Returning the value 0, which is the index of the last node in the
   * vector.
   */
  stack_type end() const noexcept { return stack_type(0); }

  /**
   * @brief Returning a reference to the value of the node.
   */
  T& value(stack_type x) {
    AP_ERROR_CUSTOM_STRICT_POSITIVE(x);
    return node(x).value;
  }

  /**
   * @brief Returning a const reference to the value of the node.
   */
  const T& value(stack_type x) const {
    AP_ERROR_CUSTOM_STRICT_POSITIVE(x);
    return node(x).value;
  }

  /**
   * @brief Returning a reference to the next node of the node x.
   */
  stack_type& next(stack_type x) {
    AP_ERROR_CUSTOM_STRICT_POSITIVE(x);
    return node(x).next;
  }

  /**
   * @brief Returning a const reference to the next node of the node x
   */
  const stack_type& next(stack_type x) const {
    AP_ERROR_CUSTOM_STRICT_POSITIVE(x);
    return node(x).next;
  }

  /**
   * @brief Function to push an element to the front of the stack
   */
  stack_type push(const T& val, stack_type head) {
    AP_ERROR_CUSTOM_POSITIVE(head);
    return _push(val, head);
  }
  stack_type push(T&& val, stack_type head) {
    AP_ERROR_CUSTOM_POSITIVE(head);
    return _push(std::move(val), head);
  }

  /**
   * @brief Function to remove from the stack the last inserted node, that will
   * be put in the free_nodes
   */
  stack_type pop(stack_type x) {
    AP_ERROR_CUSTOM_STRICT_POSITIVE(x);
    AP_ERROR(x != free_nodes, Invalid_input)
        << "You can not pop from the stack of free_nodes" << std::endl;
    auto new_head = next(x);
    next(x) = free_nodes;
    free_nodes = x;
    return new_head;
  }  // delete first node

  /**
   * @brief Function to free a single stack. The stack is resetted to its
   * initial value (std::size_t(0)). Its nodes are added to free_nodes.
   */
  stack_type free_stack(stack_type x) {
    AP_ERROR_CUSTOM_STRICT_POSITIVE(x);
    AP_ERROR(x != free_nodes, Invalid_input)
        << "You can not free the stack of free_nodes" << std::endl;
    while (!empty(x)) {
      x = this->pop(x);
    }
    return (x);
  }  // free entire stack

  /**
   * @brief Function to print a single stack
   */
  void print_stack(const stack_type x) const {
    AP_ERROR_CUSTOM_POSITIVE(x);
    auto start = begin(x);
    std::cout << "[ ";
    while (start != end(x)) {
      std::cout << *start << " ";
      ++start;
    }
    std::cout << "]" << std::endl;
  }

  /**
   * @brief Function to print the content of the pool
   */
  void print_pool() const noexcept {
    std::cout << "pool = [ ";
    for (auto i = 0; i < pool.size(); ++i) {
      std::cout << pool.at(i).value << " ";
    }
    std::cout << "]" << std::endl;
  }
};