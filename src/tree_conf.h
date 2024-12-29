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
 * @file tree_conf.h
 * @author Antonio Emmanuele (antony.35.ae@gmail.com)
 * @brief  Contains the functions and datastructures to load the classifier from a binary file.
 * @version 0.1
 * @date 2024-12-29
 * 
 * @copyright Copyright (c) 2024 Antonio Emmanuele
 * 
 */
#ifndef TREE_CONF_H
#define TREE_CONF_H
#include <stdint.h>

/**
 * @typedef bin_trailer_t
 * @brief   Trailer of the serialized binary configuration of the classifier. 
 * 
 */
typedef struct{
    uint16_t num_classes;   /**< Number of classes of the classifier. */
    uint16_t num_features;  /**< Number of input features. */
    uint16_t num_trees;     /**< Number of trees in the ensemble. */
} bin_trailer_t;


#endif // TREE_CONF_H