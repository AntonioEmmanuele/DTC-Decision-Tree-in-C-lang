# DTC: Decision Tree in C lang

This repository contains the implementation of a decision tree and random forest classifier in C lang. The main functionalities include visiting a decision tree, visiting an ensemble of trees, and performing majority voting on the classification results.

## Folders
- `src`: Folder containing the src files in C lang.
- `dtc_pygen`: Folder containing the python source files used for the generation of the binary configuration configuration.
- `datasets`: Folder containing example datasets.
- `examples`: Folder containing example source code.

## Files

- `src/tree_visit.h`: Header file containing Decision Tree type definitions and function declarations.
- `src/tree_visit.c`: Source file containing the implementation of the functions declared in tree_visit.h header file.
- `src/tree_conf.h`:  Header file containing Configuration type definitions and configuration function declarations.
- `src/tree_conf.c`:  Source file containing the implementation of the functions declared in tree_conf.h header file.

## Binary Configuration
In this library a Tree Based model (Decision Tree or Random Forest) is transformed in a binary file by the dtc_pygen configurator.
Using this approach, the PMML or the Joblib files are inputted to the configurator via the parse command and the binary is generated.
Structure of this binary is described as follow.
# Structure
- `bin_trailer_t`: Header of the binary containing informations such as the number of trees, the number of classes and the number of features.

Then, iterated for each tree (whose number is specified in the trailer).

- `uint16_t number_of_nodes`: Number of nodes of the specific decision tree.
- `node_t nodes[number_of_nodes]`: The array of nodes of the parsed Decision Tree. Such nodes are already parsed and do not require any
                                    additional post-processing step. Such array of nodes can be directly inputted to the C-fun visit to 
                                    visit the decision tree classifier.
 
In addition, if the dataset is available, the dtc_pygen configurator, using the `gen_test_vec` command can parse the dataset 
and generate an header test file containing C-input vectors and correct classess in order to validate the accuracy of the parsed tree.
For more infos, please check the `examples/desktop/dtc_parse` and `examples/desktop/inference_accuracy` folders. 

## Configurator commands
- `feature_type`: C-type of the used features. It is mandatory for all commands.
# parse
This command is issued when a model (PMML or Joblib) is inputted and the corresponding configuration binary is generated.
Args:
- `input_model`: Path of the PMML or Joblib of the input model.
- `output_bin`:  Path of the output binary.
     

# gen_test_vec
This command takes as input either the input model as well as the input datatet. It generates in output the C-test 
vectors used to validate the accuracy of the final classifier.
Args:
- `input_model`: Joblib (can not be done with PMML) of the model used to generate the correctly classified output of the model.
                    This function however is currently not implemented.
- `input_dataset`: Dataset on which the model is trained. It should be equal to the CSV of the test set as the generated test-vectors are
                     equal to the ones of the classified output.
- `target_column`: Column of the dataset mantaining the classification results.
- `output_test_vec`: Name,-- i.e. the path--, of the output header file containing the test vectors and classification results.

## C-lib Compilation Flags
Here are reported the compilation flags of the implemented functionalities. Not tested ones, are not reported as they are not meant to be used.
- `USE_FLOAT`: If use float is set to 1 then the library used float for the feature representation. Otherwise double is used.

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


## License
This project is licensed under the GNU General Public License v3.0 (GPLv3) - see the [LICENSE](LICENSE) file for details.
