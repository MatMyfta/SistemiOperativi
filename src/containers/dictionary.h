/**
 * \file dictionary.h
 *
 * \brief Generic ordered dictionary - interface
 *
 * API inspired by GLib
 */
#ifndef UNITNOS_DICTIONARY_H_
#define UNITNOS_DICTIONARY_H_

#include "../bool.h"
#include "base.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opqaue type representing a dictionary
 */
typedef struct unitnos_dictionary unitnos_dictionary;
/**
 * Specifies the type of function passed to #unitnos_dictionary_foreach(). It
 * is passed the key and the value of each key-value pair, together with the
 * optional user_data parameter passed to #unitnos_dictionary_foreach(). If the
 * function returns true, the traversal is stopped.
 */
typedef bool (*unitnos_dictionary_transverse_func)(void *key, void *value,
                                                   void *user_data);

/**
 * Creates a new dictionary like and allows to specify functions to free
 * the memory allocated for the key and value that get called when removing the
 * entry from the dictionary.
 *
 *
 * \param [in] key_compare_func qsort()-style comparison function
 * \param [in] key_destroy_func a function to free the memory allocated for the
 * key used when removing the entry from the dictionary or NULL if you don't
 * want to supply such a function
 * \param [in] value_destroy_func a function to free the memory allocated for
 * the value used when removing the entry from the dictionary or NULL if you
 * don't want to supply such a function
 *
 * \returns a newly allocated dictionary
 */
unitnos_dictionary *unitnos_dictionary_create(
    unitnos_compare_func compare_func, unitnos_destroy_nodify key_destroy_func,
    unitnos_destroy_nodify value_destroy_func, void *user_data);

/**
 * Destroy a dictionary
 *
 * \param [in] dictionary the dictionary to be destroyed
 *
 * Removes all keys and values from the dictionary and decreases its If keys
 * and/or values are dynamically allocated, you should either free them first or
 * pass key_destroy_func and value_destroy_func during the creation of the
 * dictionary. In the latter case the destroy functions you supplied will be
 * called on all keys and values before destroying the dictionary.
 */
void unitnos_dictionary_destroy(unitnos_dictionary *dictionary);

/**
 * Inserts a key/value pair into a dictionary.
 *
 * \param [in] dictionary a dictionary
 * \param [in] key the key to insert
 * \param [in] value the value corresponding to the key
 *
 * If the given key already exists in the dictionary its corresponding value is
 * set to the new value. If you supplied a value_destroy_func when creating the
 * dictionary, the old value is freed using that function. If you supplied a
 * key_destroy_func when creating the dictionary, the passed key is freed using
 * that function.
 */
void unitnos_dictionary_insert(unitnos_dictionary *dictionary, void *key,
                               void *value);

/**
 * Gets the value corresponding to the given key.
 *
 * \param [in] dictionary a dictionary
 * \param [in] the key to look up
 *
 * \returns the value corresponding to the key, or NULL if the key was not found
 */
void *unitnos_dictionary_lookup(unitnos_dictionary *dictionary,
                                const void *key);
/**
 * Check whether the given key is present
 */
bool unitnos_dictionary_contains(unitnos_dictionary *dictionary,
                                 const void *key);

/**
 * Gets the key in the dictionary that matches with the given key.
 *
 * \param [in] dictionary a dictionary
 * \param [in] the key to look up
 *
 * \returns the matching key, or NULL if the key was not found
 */
void *unitnos_dictionary_key_lookup(unitnos_dictionary *dictionary,
                                    const void *key);

/**
 * Removes a key/value pair from a dictionary.
 *
 * \param [in] dictionary a dictionary
 * \param [in] key the key to remove
 *
 * If during the dictionary creation key_destroy_func and/or value_destroy_func
 * were passed, the key and value are freed using the supplied destroy
 * functions, otherwise you have to make sure that any dynamically allocated
 * values are freed yourself. If the key does not exist in the dictionary, the
 * function does nothing.
 */
void unitnos_dictionary_remove(unitnos_dictionary *dictionary, const void *key);

/**
 * Calls the given function for each of the key&value pair in the dictionary.
 * The function is passed the key and the value of key&value pair, and the
 * given \p user_data parameter. The dictionary is traversed in sorted order.
 *
 * The dictionary may not be modified while iterating over it (you can't
 * add/remove items). To remove all items matching a predicate, you need to add
 * each item to a list in your unitnos_dictionary_transverse_func as you walk
 * over the dictionary, then walk the list and remove each item.
 *
 * \param [in] dictionary a dictionary
 * \param [in] func the function to call for each key&value pair visited. If
 * this function returns true, the traversal is stopped.
 * \param [in] user_data user data to pass to the function
 */
void unitnos_dictionary_foreach(unitnos_dictionary *dictionary,
                                unitnos_dictionary_transverse_func func,
                                void *user_data);
/**
 * Get the number of key&value pairs in the dictionary
 */
size_t unitnos_dictionary_size(unitnos_dictionary *dictionary);

/**
 * Get the first key in the dictionary
 *
 * \returns the first key, if any
 */
void *unitnos_dictionary_first(unitnos_dictionary *dictionary);
/**
 * Get the key with the highest value in the dictionary
 *
 * \returns the highest key, if any
 */
void *unitnos_dictionary_max(unitnos_dictionary *dictionary);
/**
 * Get the key with the lowest value in the dictionary
 *
 * \returns the lowest value, if any
 */
void *unitnos_dictionary_min(unitnos_dictionary *dictionary);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_DICTIONARY_H_ */
