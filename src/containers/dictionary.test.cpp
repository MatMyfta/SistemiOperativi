/**
 * \file dictionary.test.cpp
 *
 * \brief Test dictionary
 */

#include "dictionary.h"

#include "catch2/catch.hpp"

#include <map>
#include <utility>

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

TEST_CASE("Dictionary", "[containers][dictionary]") {
  unitnos_dictionary *dictionary =
      unitnos_dictionary_create(compare<int>, nullptr, nullptr, nullptr);

  SECTION("Value insertion") {
    std::map<int, double> values = {
        {1, 1.0},
        {2, 3.0},
        {3, 5.0},
    };
    REQUIRE(unitnos_dictionary_size(dictionary) == 0);

    size_t i = 0;
    for (auto &val : values) {
      unitnos_dictionary_insert(dictionary, const_cast<int *>(&val.first),
                                const_cast<double *>(&val.second));
      ++i;
      REQUIRE(unitnos_dictionary_size(dictionary) == i);
    }

    REQUIRE(unitnos_dictionary_size(dictionary) == values.size());
  }

  GIVEN("A tree filled with values") {
    std::vector<std::pair<int, double>> values = {
        {1, 1.0},
        {3, 5.0},
        {2, 3.0},
    };

    for (auto &val : values) {
      unitnos_dictionary_insert(dictionary, &val.first, &val.second);
    }

    SECTION("for each order") {
      std::vector<std::pair<int, double>> v;

      unitnos_dictionary_foreach(
          dictionary,
          [](void *key, void *value, void *v) {
            std::vector<std::pair<int, double>> *vec =
                reinterpret_cast<std::vector<std::pair<int, double>> *>(v);
            vec->push_back({*((int *)key), *((double *)value)});
            return false;
          },
          &v);

      REQUIRE(v[0].first == 1);
      REQUIRE(v[1].first == 2);
      REQUIRE(v[2].first == 3);
    }

    SECTION("lookup") {
      int n = 2;
      void *val = unitnos_dictionary_lookup(dictionary, &n);
      REQUIRE(val == &values[2].second);

      n = 5;
      val = unitnos_dictionary_lookup(dictionary, &n);
      REQUIRE(val == nullptr);
    }
    SECTION("contains") {
      int n = 2;
      bool contains = unitnos_dictionary_contains(dictionary, &n);
      REQUIRE(contains);
      n = 123;
      contains = unitnos_dictionary_contains(dictionary, &n);
      REQUIRE_FALSE(contains);
    }

    SECTION("Value removal") {
      unitnos_dictionary_remove(dictionary, &values[0]);
      REQUIRE(unitnos_dictionary_size(dictionary) == 2);
      unitnos_dictionary_remove(dictionary, &values[1]);
      REQUIRE(unitnos_dictionary_size(dictionary) == 1);
      unitnos_dictionary_remove(dictionary, &values[2]);
      REQUIRE(unitnos_dictionary_size(dictionary) == 0);
    }
  }

  unitnos_dictionary_destroy(dictionary);
}
