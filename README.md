# Tree Visiting

This repository contains the implementation of a decision tree and random forest classifier in C lang. The main functionalities include visiting a decision tree, visiting an ensemble of trees, and performing majority voting on the classification results.
## Folders
- `src`: Folder containing the src files in C lang.

## Files

- `src/tree_visit.h`: Header file containing type definitions and function declarations.
- `src/tree_visit.c`: Source file containing the implementation of the functions declared in the header file.

## C-lib Functions (tree_visit.c)

### visit_tree

Visits the decision tree classifier and provides the classification result.

**Parameters:**
- `root_node`: Pointer to the root node of the tree.
- `features`: Array of input feature values.
- `classification_result`: Pointer to store the classification result.

**Returns:**
- `CLASSIFICATION_OK`: Classification was successful.
- `CLASSIFICATION_PRUNED`:Node was pruned, therefore classification_results contains a negative value.

### visit_ensemble

Visits an ensemble of trees and classifies the given features.

**Parameters:**
- `trees`: Array of pointers to the root nodes of the trees.
- `number_trees`: Number of trees in the ensemble.
- `features`: Array of feature values.
- `class_per_tree`: Array to store the classification result of each tree.

**Returns:**
- `CLASSIFICATION_OK`: Classification was successful.
- `CLASSIFICATION_PRUNED`: At least one tree resulted in `CLASSIFICATION_PRUNED`.

### majority_voting

Determines the most popular classification result from an array of classifications.

**Parameters:**
- `classifications`: Array of classification results.
- `classification_elements`: Number of elements in the classifications array.
- `most_popular`: Pointer to store the most popular class.

**Returns:**
- Number of votes of the majority class.

### visit_rf_majority_voting

Performs majority voting on the classification results of an ensemble of trees.

**Parameters:**
- `trees`: Array of pointers to the root nodes of the trees.
- `number_trees`: Number of trees in the ensemble.
- `features`: Array of feature values.
- `classification_result`: Pointer to store the final classification result.
- `num_votes`: Pointer to store the number of votes for the majority class.

**Returns:**
- `CLASSIFICATION_OK`: Classification was successful.
- `CLASSIFICATION_PRUNED`: At least one tree resulted in `CLASSIFICATION_PRUNED`.
- `CLASSIFICATION_DRAW`: A draw condition occurred during majority voting.

## Usage
TODO
To use the functions provided in this repository, include the `tree_visit.h` header file in your project and link against the compiled `tree_visit.c` source file.



## License
This project is licensed under the GNU General Public License v3.0 (GPLv3) - see the [LICENSE](LICENSE) file for details.
