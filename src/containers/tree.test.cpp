/**
 * \file dictionary.test.cpp
 *
 * \brief Test dictionary
 */

#include "tree.h"

#include "catch2/catch.hpp"

#include <vector>

template <typename T>
int compare(const void *lhs, const void *rhs, void *user_data) {
  const T *lhs_val = reinterpret_cast<const T *>(lhs);
  const T *rhs_val = reinterpret_cast<const T *>(rhs);
  if (*lhs_val < *rhs_val) {
    return -1;
  } else if (*lhs_val == *rhs_val) {
    return 0;
  } else {
    return 1;
  }
};

TEST_CASE("Tree", "[containers][tree]") {
  unitnos_tree *tree = unitnos_tree_create(compare<int>, nullptr, nullptr);

  SECTION("Value insertion") {
    std::vector<int> values = {2, 1, 3};
    REQUIRE(unitnos_tree_size(tree) == 0);

    size_t i = 0;
    for (auto &val : values) {
      unitnos_tree_insert(tree, &val);
      ++i;
      REQUIRE(unitnos_tree_size(tree) == i);
    }

    REQUIRE(unitnos_tree_size(tree) == values.size());
  }

  GIVEN("A tree filled with values") {
    std::vector<int> values = {2, 1, 3};
    for (auto &val : values) {
      unitnos_tree_insert(tree, &val);
    }
    SECTION("for each order") {
      std::vector<int> v;
      unitnos_tree_foreach(
          tree,
          [](void *val, void *v) {
            std::vector<int> *vec = reinterpret_cast<std::vector<int> *>(v);
            vec->push_back(*((int *)val));
            return false;
          },
          &v);

      REQUIRE(v[0] == 1);
      REQUIRE(v[1] == 2);
      REQUIRE(v[2] == 3);
    }

    SECTION("lookup") {
      int n = 2;
      void *val = unitnos_tree_lookup(tree, &n);
      REQUIRE(val != &n);
      REQUIRE(val == &values[0]);

      n = 5;
      val = unitnos_tree_lookup(tree, &n);
      REQUIRE(val == nullptr);
    }
    SECTION("contains") {
      int n = 2;
      bool contains = unitnos_tree_contains(tree, &n);
      REQUIRE(contains);
      n = 123;
      contains = unitnos_tree_contains(tree, &n);
      REQUIRE_FALSE(contains);
    }

    SECTION("Value removal") {

      unitnos_tree_remove(tree, &values[0]);
      REQUIRE(unitnos_tree_size(tree) == 2);
      unitnos_tree_remove(tree, &values[1]);
      REQUIRE(unitnos_tree_size(tree) == 1);
      unitnos_tree_remove(tree, &values[2]);
      REQUIRE(unitnos_tree_size(tree) == 0);
    }
  }

  unitnos_tree_destroy(tree);
}
