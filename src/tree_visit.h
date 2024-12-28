/*
 * This file is part of DTC: Decision Tree in C-lang project.
 *
 * DTC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DTC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DTC. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file tree_visit.h
 * @author Antonio Emmanuele (antony.35.ae@gmail.com)
 * @brief   Contains the function prototypes for visiting a decision tree and an ensemble of trees.
 * @version 0.1
 * @date 2024-12-27
 * 
 * @copyright Copyright (c) 2024 Antonio Emmanuele
 * 
 */
#ifndef TREE_VISIT_H
#define TREE_VISIT_H

#include "stdint.h"

#define COMPILE_PRUNED 0            /**< If set to 1, the code compile pruning handling */
#define USE_POINTERS   0            /**< If set to 1, the code explicitly use pointers to nodes. */   
#define CLASSIFICATION_DEFAULT 0    /**< Classification default return value. Theoretically, never employed. */
#define CLASSIFICATION_OK 1         /**< No draw or pruned conditions occurred during classification. */
#if COMPILE_PRUNED
#define CLASSIFICATION_PRUNED -1    /**< The tree was pruned, i.e. the current_node, during the execution of tree_visiting, is NULL and a leaf is not reached. */
#endif
#define CLASSIFICATION_DRAW -2      /**< Two or more classes share the majority voting condition. */

extern int num_classes; /**< Number of classes. Initialized in the .c file, can be externally inizialized from main.*/


#if !USE_POINTERS
/**
 * @typedef nodes_idx_t
 * @brief A type representing the index of a node in the tree.
 */
typedef int32_t nodes_idx_t;
#endif
/**
 * @typedef class_t
 * @brief A type representing the classification result.
 */
typedef int16_t class_t;

/**
 * @typedef feature_idx_t
 * @brief A type representing the index of a feature. This index is mantained in a node for classifications.
 */
typedef uint16_t feature_idx_t;

/**
 * @typedef feature_type_t
 * @brief A type representing the value of a feature.
 */
typedef double feature_type_t;

/**
 * @struct node_t
 * @brief A structure representing a node in the decision tree.
 */
typedef struct node_t {
    feature_idx_t feature_index; /**< Index of the feature used for splitting. */
    feature_type_t threshold;    /**< Threshold value for the feature. */
    class_t class;               /**< Classification result if the node is a leaf. */
#if USE_POINTERS
    struct node_t* left_child;   /**< Pointer to the left child node. */
    struct node_t* right_child;  /**< Pointer to the right child node. */
#else 
    nodes_idx_t left_node;          /**< Index of the left child node. */
    nodes_idx_t right_node;         /**< Index of the right child node. */
#endif
} node_t;

/**
 * @brief Visits the decision tree classifier providing in output the classification result.
 * 
 * @param[in] root_node Pointer to the root node of the tree.
 * @param[in] features Array of input feature values (i.e. the input sample).
 * @param[out] classification_result Pointer to store the classification result.
 * @return int Status of the visit operation.
 * @retval CLASSIFICATION_OK Classification was successful.
 * @retval CLASSIFICATION_PRUNED Node was pruned, therefore classification_results contains a negative value.
 */
int visit_tree(const node_t* const root_node, const feature_type_t* const features, class_t* const classification_result);

/**
 * @brief Visits an ensemble of trees and classifies the given features.
 * 
 * @param[in] trees Array of pointers to the root nodes of the trees.
 * @param[in] number_trees Number of trees in the ensemble.
 * @param[in] features Array of feature values.
 * @param[out] class_per_tree Array to store the classification result of each tree.
 * @return int Status of the visit operation.
 * @retval CLASSIFICATION_OK Classification was successful.
 * @retval CLASSIFICATION_PRUNED At least the classification of a single tree resulted in CLASSIFICATION_PRUNED.
 */
int visit_ensemble(node_t* const trees[],  const uint16_t number_trees, const feature_type_t* const features, class_t* const class_per_tree);

/**
 * @brief Performs majority voting on the classification results of an ensemble of trees.
 * 
 * @param[in] trees Array of pointers to the root nodes of the trees.
 * @param[in] number_trees Number of trees in the ensemble.
 * @param[in] features Array of feature values.
 * @param[out] classification_result Pointer to store the final classification result.
 * @return int Status of the visit operation.
 * @retval CLASSIFICATION_OK Classification was successful.
 * @retval CLASSIFICATION_PRUNED At least the classification of a single tree resulted in CLASSIFICATION_PRUNED.
 * @retval CLASSIFICATION_DRAW A draw condition during majority voting occured (not implemented).
 */
int visit_rf_majority_voting(node_t* const trees[],  const uint16_t number_trees, const feature_type_t* const features, class_t* const classification_result);

/**
 * @brief Determines the most popular classification result from an array of classifications.
 * 
 * @param[in] classifications Array of classification results.
 * @param[in] classification_elements Number of elements in the classifications array.
 * @param[out] most_popular Pointer to store the most popular class.
 * @return int Status of the majority voting operation.
 * @retval Number of votes of the majority class.
 */
int majority_voting(const class_t* const classifications, const uint16_t classification_elements, class_t* const most_popular);

#endif // TREE_VISIT_H