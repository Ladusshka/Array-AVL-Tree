# Array-AVL-Tree


This repository contains a template class Array<T> that behaves like an array interface but internally uses an AVL tree for storage. This allows fast insertion and deletion at arbitrary positions, at the cost of slightly slower element access. The main methods include:

size() – Returns the current number of elements.
operator[](i) – Returns the element at position i, throwing an std::out_of_range exception if i is invalid.
insert(i, t) – Inserts t at position i; shifts existing elements from index i onwards.
erase(i) – Erases the element at position i; throws an std::out_of_range if i is out of bounds.
An internal TesterInterface provides methods to query the underlying AVL tree structure (root(), parent(), left(), right(), value()), which are used for automated testing of both the binary search invariants and the AVL balancing properties.

Configuration
config::CHECK_DEPTH (default: false)
Enables AVL shape validation when true. Otherwise, only basic binary structure validation is performed.
config::PARENT_POINTERS (default: true)
Determines whether parent pointers are tested.
