/**
 * \file set.h
 *
 * \brief Sorted set - implementation
 */
#include "set.h"

#include "tree.h"

struct unitnos_set {
  unitnos_tree *tree;
};

unitnos_set *unitnos_set_create(unitnos_compare_func compare_func,
                                unitnos_destroy_nodify value_destroy_func,
                                void *user_data) {
  return (unitnos_set *)unitnos_tree_create(compare_func, value_destroy_func,
                                            user_data);
}
void unitnos_set_destroy(unitnos_set *set) {
  unitnos_tree_destroy((unitnos_tree *)set);
}
void unitnos_set_insert(unitnos_set *set, void *value) {
  unitnos_tree_insert((unitnos_tree *)set, value);
}
void unitnos_set_remove(unitnos_set *set, const void *value) {
  unitnos_tree_remove((unitnos_tree *)set, value);
}
void unitnos_set_foreach(unitnos_set *set, unitnos_set_transverse_func func,
                         void *user_data) {
  unitnos_tree_foreach((unitnos_tree *)set, func, user_data);
}
size_t unitnos_set_size(unitnos_set *set) {
  return unitnos_tree_size((unitnos_tree *)set);
}
bool unitnos_set_contains(unitnos_set *set, const void *value) {
  return unitnos_tree_contains((unitnos_tree *)set, value);
}
void *unitnos_set_lookup(unitnos_set *set, const void *value) {
  return unitnos_tree_lookup((unitnos_tree *)set, value);
}
void *unitnos_set_first(unitnos_set *set) {
  return unitnos_tree_first((unitnos_tree *)set);
}
void *unitnos_set_max(unitnos_set *set) {
  return unitnos_tree_max((unitnos_tree *)set);
}
void *unitnos_set_min(unitnos_set *set) {
  return unitnos_tree_min((unitnos_tree *)set);
}
