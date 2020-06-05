/**
 * \file base.h
 *
 * Common utilities used to implement containers
 */

#ifndef UNITNOS_CONTAINER_BASE_H_
#define UNITNOS_CONTAINER_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Specifies the type of a comparison function used to compare two values. The
 * function should return a negative integer if the first value comes before
 * the second, 0 if they are equal, or a positive integer if the first value
 * comes after the second.
 *
 * \param [in] lhs the left hand side value
 * \param [in] rhs the right hand side value
 * \param [in] user_data optional user data
 *
 * \returns negative value if \p lhs < \p rhs; zero if \p lhs = \p rhs ;
 * positive value if \p lhs > \p rhs
 */
typedef int (*unitnos_compare_func)(const void *lhs, const void *rhs,
                                    void *user_data);
/**
 * Specifies the type of function which is called when a data element is
 * destroyed. It is passed the pointer to the data element and should free any
 * memory and resources allocated for it.
 *
 * \param [in] data the data element the be destroyed
 * \param [in] user_data optional user data
 */
typedef void (*unitnos_destroy_nodify)(void *data, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* ifndef UNITNOS_CONTAINER_BASE_H_ */
