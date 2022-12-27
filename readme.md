# persistent red black tree

This module is a part of [yirong-c/CLRS](https://github.com/yirong-c/CLRS)

## Description

- Red-black trees
that maintain past versions of a red-black tree.

- Guarantee O(lg n) running time and space
per selection insertion, or deletion.

![](https://github.com/yirong-c/persistent-red-black-tree/blob/master/persistent-dynamic-set.png)

## File Structure

```bash
.
├── persistent_red_black_tree.hpp          # main part of red black tree
├── persistent_red_black_tree_test.hpp     # auxiliary test functions
└── persistent_red_black_tree_test.cpp     # test cases (catch2)
```

## Bibliography

Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). Introduction to algorithms  (Third edition.). MIT Press.
