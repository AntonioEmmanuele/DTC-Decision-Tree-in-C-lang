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
 * @file tree_visit.c
 * @author Antonio Emmanuele (antony.35.ae@gmail.com)
 * @brief  Contains the function implementations for visiting a decision tree and an ensemble of trees.
 * @version 0.1
 * @date 2024-12-27
 * 
 * @copyright Copyright (c) 2024 Antonio Emmanuele
 * 
 */
#include "tree_visit.h"
#include "assert.h"
#include "string.h"
// #include "stdio.h"
// #include "stdlib.h"
#if USE_POINTERS
/**
 * @brief Macro used to check if a leaf node is reached, i.e. if the current node is a leaf node.
 * 
 */
#define IS_LEAF(current_node) ((NULL != current_node -> left_node) && (NULL != current_node -> right_node))
#else
/**
 * @brief Macro used to check if a leaf node is reached, i.e. if the current node is a leaf node.
 *          If no pointers are used then the macro checks whether the left and right nodes are different from -1.
 * 
 */
#define IS_LEAF(current_node) ((-1 == current_node -> left_node) && (-1 == current_node -> right_node))
#endif
#if COMPILE_PRUNED 
/**
 * @brief Macro used to check if the current node is pruned, i.e. if the current node is NULL.
 * 
 */
#define IS_PRUNED(current_node) (NULL == current_node)
#endif

int num_classes = 256;

/**
 * @typedef Operator function pointer definition
 * 
 */
typedef uint8_t (*operator_fun_t)(const feature_type_t, const feature_type_t);

/**
 * @brief Less or equal compare function
 * @param[in] a First value to compare
 * @param[in] b Second value to compare
 * @return uint8_t 1 if a <= b, 0 otherwise
 */
static uint8_t less_or_equal(const feature_type_t a, const feature_type_t b){
    return a <= b;
}

/**
 * @brief Less than compare function
 * @param[in] a First value to compare
 * @param[in] b Second value to compare
 * @return uint8_t 1 if a < b, 0 otherwise
 */
static uint8_t less_than(const feature_type_t a, const feature_type_t b){
    return a < b;
}

/**
 * @brief Greater or equal than compare function
 * @param[in] a First value to compare
 * @param[in] b Second value to compare
 * @return uint8_t 1 if a >= b, 0 otherwise
 */
static uint8_t greater_or_equal(const feature_type_t a, const feature_type_t b){
    return a >= b;
}

/**
 * @brief Greater than compare function
 * @param[in] a First value to compare
 * @param[in] b Second value to compare
 * @return uint8_t 1 if a > b, 0 otherwise
 */
static uint8_t greater_than(const feature_type_t a, const feature_type_t b){
    return a > b;
}

/**
 * @brief Equal compare function
 * @param[in] a First value to compare
 * @param[in] b Second value to compare
 * @return uint8_t 1 if a == b, 0 otherwise
 */
static uint8_t equal(const feature_type_t a, const feature_type_t b){
    return a == b;
}

/**
 * @brief Not equal compare function
 * @param[in] a First value to compare
 * @param[in] b Second value to compare
 * @return uint8_t 1 if a != b, 0 otherwise
 */
static uint8_t not_equal(const feature_type_t a, const feature_type_t b){
    return a != b;
}
static operator_fun_t operators[] = {less_or_equal, less_than, greater_or_equal, greater_than, equal, not_equal};

int visit_tree(const node_t* const root_node, const feature_type_t * const features, class_t* const classification_result){
    const node_t* current_node = root_node; 
    int to_ret = CLASSIFICATION_DEFAULT;
    uint8_t is_leaf = IS_LEAF(current_node);
#if COMPILE_PRUNED
    uint8_t is_pruned = IS_PRUNED(current_node);
    while( !is_leaf && !is_pruned){
#else
    while(!is_leaf){
#endif

#if USE_POINTERS
        if(operators[current_node->operator](features[current_node->feature_index], current_node->threshold)){
            current_node = current_node -> left_child;
        }
        else{
            current_node = current_node -> right_child;
        }
#else
        if(operators[current_node->operator](features[current_node->feature_index], current_node->threshold)){
            current_node = &root_node[current_node -> left_node];
        }
        else{
            current_node = &root_node[current_node -> right_node];
        }
#endif

#if COMPILE_PRUNED
        is_pruned = IS_PRUNED(current_node);
        // Avoid NULL memory access
        if (!is_pruned)
            is_leaf = IS_LEAF(current_node);
#else
        is_leaf = IS_LEAF(current_node);
#endif
    }
#if COMPILE_PRUNED
    if(is_pruned){
        to_ret = CLASSIFICATION_PRUNED;
        *classification_result = to_ret;
    }
    else if (is_leaf){
#else
    if(is_leaf){
#endif
        to_ret = CLASSIFICATION_OK;
        *classification_result = current_node -> class;
    }
    else{
        assert(1 == 0);
    }
    return to_ret;
}

int visit_ensemble(node_t* const trees[],  const uint16_t number_trees, const feature_type_t* const features, class_t* const class_per_tree){
    int ret_helper = CLASSIFICATION_OK;
    uint16_t tree_idx = 0U;
    for(tree_idx = 0U; tree_idx < number_trees; tree_idx ++){
        // If it is CLASSIFICATION_OK then it remains classification ok.
        // Otherwise, if just once it becames classification pruned, then it returns.
        ret_helper = ret_helper && (CLASSIFICATION_OK == visit_tree(trees[tree_idx], features, &class_per_tree[tree_idx]));
    }
    return ret_helper;
}

int majority_voting(const class_t* const classifications, 
                    const uint16_t classification_elements, 
                    class_t* const most_popular) {
    
    /**
     * This function do not handle Draw Conditions. 
     * Honestly, it makes no sense to handle them, except if you wanna waste CPU cycles.
     * A Draw Condition is inherently a situation in which two or more classess are the most popular.
     * This may result in a misclassification.
     * This function simply returns the ""fist"" most popular class. If it is the correct class then 
     * we're lucky. If it is not, then it is a missclassification.
     */

    // Cardinality of class counts can be externally redefined
    uint16_t class_counts[num_classes];
    memset(class_counts, 0, num_classes * sizeof(uint16_t)); 
    // Max_class is the most popular class.
    class_t max_class = -1;
    // Count of occurrences of the most popular class
    uint16_t max_count = 0;  

    // Count valid classifications
    for (uint16_t i = 0; i < classification_elements; i++) {
#if COMPILED_PRUNING
        if (0 <= classifications[i] ) { // Ignore classifications < 0
#endif
            class_counts[classifications[i]]++;
            // Update the most popular class
            if (max_count < class_counts[classifications[i]] ) {
                max_count = class_counts[classifications[i]];
                max_class = classifications[i];
            }
#if COMPILED_PRUNING
        }
#endif
    }
    // Update only if needed.
    if(max_count > 0)
        *most_popular = max_class;
    return max_count; 
}

int visit_rf_majority_voting(   node_t* const trees[],
                                const uint16_t number_trees, 
                                const feature_type_t* const features,
                                class_t* const classification_result,
                                uint16_t * const num_votes){
    
    int to_ret = CLASSIFICATION_DEFAULT;
    // Classess per each different tree
    class_t class_per_tree[number_trees];
    memset(class_per_tree, -1, number_trees * sizeof(class_t));
    to_ret = visit_ensemble(trees, number_trees, features, class_per_tree);
    // for(int i = 0; i < number_trees; i++){
    //     printf("Classification result for tree %d : %d\n", i, class_per_tree[i]);
    // }
    // exit(1);
    // Do the majority voting.
    *num_votes = majority_voting(class_per_tree, number_trees, classification_result);
    return to_ret; 
}