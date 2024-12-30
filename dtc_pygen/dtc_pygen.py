"""
@file dtc_pygen.py
@brief This file contains the utils to generate a binary configurator for the c lib given a PMML file in input.


@copyright Copyright (C) 2024 Antonio Emmanuele

This file is part of DTC Decision Tree in C-lang

DTC is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DTC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DTC. If not, see <https://www.gnu.org/licenses/>.
"""
import xml.etree.ElementTree as ET
import ctypes
import argparse
import joblib 
import numpy as np
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier
import pandas as pd
from jinja2 import Environment, FileSystemLoader

""" Map of operators to c code functions. If the array in C is altered then this map must be changed accordingly."""
operators_map = {
    "lessOrEqual": 0,
    "lessThan": 1,
    "greaterOrEqual": 2,
    "greatrThan": 3,
    "equal": 4,
    "notEqual": 5,
}

def find_feature_index_by_name(data, input_name):
    """
    Finds the index of the dictionary in the list where the 'name' field matches the input string.

    Parameters:
        data (list): A list of dictionaries, each containing a 'name' key.
        input_name (str): The name to search for.

    Returns:
        int: The index of the dictionary with the matching 'name', or -1 if not found.
    """
    for index, item in enumerate(data):
        if item.get('name') == input_name:
            return index
    assert 1 == 0, "Invalid PMML, feature provided by the node is not present in the model features."

# Define the tree node c structure
class TreeNode(ctypes.Structure):
    _fields_ = [
        ("operator", ctypes.c_uint16),
        ("feature_index", ctypes.c_uint16), 
        ("class_res", ctypes.c_int16),
        ("left_node", ctypes.c_int32 ), # For now, it is easier to use the int32_t for direct addressing
        ("right_node", ctypes.c_int32), # even if the C code has the support for pointers. This makes initialization a lot easier.
        ("threshold", ctypes.c_double),

    ]

# Internal representation of the node used during the parsing.
class NodeConfig:
    def __init__(self, tree_node_struct, id, left_node, right_node):
        self.tree_node_struct = tree_node_struct
        self.id = id
        self.left_node = left_node
        self.right_node = right_node

# Model Trailer
class ConfigTrailer(ctypes.Structure):
    _fields_ = [
        ("num_classes", ctypes.c_uint16), 
        ("num_features" , ctypes.c_uint16),
        ("num_trees", ctypes.c_uint16),
    ]

def print_tree(node_list):
    """
    Prints the entire tree, highlighting left and right children IDs for each node.

    Parameters:
        node_list (list): List of NodeConfig objects representing the tree.
    """
    for node in node_list:
        left_id = node.tree_node_struct.left_node
        right_id = node.tree_node_struct.right_node
        print(f"Node ID: {node.id}, Left Child ID: {left_id}, Right Child ID: {right_id}")

def get_xmlns_uri(elem):
    """ This function is directly taken from the pyALS-RF project. See https://github.com/SalvatoreBarone/pyALS-RF/tree/master for more info. """
    if elem.tag[0] == "{":
        uri, ignore, tag = elem.tag[1:].partition("}")
    else:
        uri = None
    return uri

def get_features_and_classes_from_pmml(root, pmml_namespace):
    """ This function is directly taken from the pyALS-RF project. See https://github.com/SalvatoreBarone/pyALS-RF/tree/master for more info. """
    model_features = []
    model_classes = []
    for child in root.find("pmml:DataDictionary", pmml_namespace).findall('pmml:DataField', pmml_namespace):
        if child.attrib["optype"] == "continuous":
            # the child is PROBABLY a feature
            model_features.append({
                "name": child.attrib['name'].replace('-', '_'),
                "type": "double" if child.attrib['dataType'] == "double" else "int"})
        elif child.attrib["optype"] == "categorical":
            # the child PROBABLY specifies model-classes
            for element in child.findall("pmml:Value",pmml_namespace):
                model_classes.append(element.attrib['value'].replace('-', '_'))
    return model_features, model_classes

def get_tree_model_from_pmml(namespaces, tree_model_root, features, id=0):
    tree = []
    get_tree_nodes_from_pmml_recursively(namespaces, tree_model_root, tree, features, id)
    return tree

def get_tree_nodes_from_pmml_recursively(namespaces, element_tree_node, nodes_list, features, id=0):
    # Find the children of the current node.
    children = element_tree_node.findall("pmml:Node", namespaces)
    # The children in PMML are not corresponding to a maximum of 2 children in the tree.
    # Indeed in a PMML file, two children represent a single node of the tree.
    # For example Node_0 is always true, while Node_1 correspond to the first node ( with the <=) condition while Node_2 to the second node (with the > condition).
    # These two node corresponds to a single node Node_x.
    assert len(children) == 2, f"Only binary trees are supported. Aborting. {children}" 
    children_list = []
    # For each children.
    for child in children:
        predicate = None
        if compound_predicate := child.find("pmml:CompoundPredicate", namespaces):
            predicate = next(item for item in compound_predicate.findall("pmml:SimplePredicate", namespaces) if item.attrib["operator"] != "isMissing")
        else:
            predicate = child.find("pmml:SimplePredicate", namespaces)
        if predicate is not None:
            feature = predicate.attrib['field'].replace('-', '_')
            operator = predicate.attrib['operator']
            threshold_value = predicate.attrib['value']   
        children_list.append({"children": child, "feature": feature, "operator": operator, "threshold_value": threshold_value})
    
    #Now append always the left 
    tree_node_feature = find_feature_index_by_name(features, children_list[0]["feature"])
    tree_node_operator = operators_map[children_list[0]["operator"]]
    tree_node_threshold = float(children_list[0]["threshold_value"]) # For now only double are supported
    # Todo : Fix Right and left correct indexes
    tree_node_right = 0
    tree_node_left = 0
    class_res = -1
    #nodes_list.append(TreeNode(tree_node_operator, tree_node_feature, tree_node_threshold, class_res, tree_node_left, tree_node_right))
    tree_node_c_struct = TreeNode(tree_node_operator, tree_node_feature, class_res, tree_node_left, tree_node_right, tree_node_threshold)
    parent_node = NodeConfig(tree_node_c_struct, id, None, None)
    nodes_list.append(parent_node)

    #print("Node : Feature: ", tree_node_feature, "Operator: ", tree_node_operator, "Threshold: ", tree_node_threshold)
    # Set the left node index.
    parent_node.tree_node_struct.left_node = len(nodes_list)
    if children_list[0]["children"].find("pmml:Node", namespaces) is not None:
        get_tree_nodes_from_pmml_recursively(namespaces, children_list[0]["children"], nodes_list, features, len(nodes_list))
    else:
        operator = 0 
        feature_index = 0
        threshold = 0
        class_res =  int(children_list[0]["children"].attrib['score'].replace('-', '_'))
        left_node = -1
        right_node = -1
        tree_node_c_struct = TreeNode(operator,feature_index, class_res, left_node, right_node, threshold)
        left_node = NodeConfig(tree_node_c_struct, len(nodes_list), None, None)
        nodes_list.append(left_node)
    
    # Set the right node index.
    parent_node.tree_node_struct.right_node = len(nodes_list)
    if children_list[1]["children"].find("pmml:Node", namespaces) is not None:
        get_tree_nodes_from_pmml_recursively(namespaces, children_list[1]["children"], nodes_list, features, len(nodes_list))
    else:
        operator = 0 
        feature_index = 0
        threshold = 0
        class_res =  int(children_list[1]["children"].attrib['score'].replace('-', '_'))
        left_node = -1
        right_node = -1
        tree_node_c_struct = TreeNode(operator, feature_index, class_res, left_node, right_node, threshold)
        left_node = NodeConfig(tree_node_c_struct, len(nodes_list), None, None)
        nodes_list.append(left_node)
        #print("Right Leaf : Feature: ", tree_node_feature, "Operator: ", tree_node_operator, "Threshold: ", children_list[1]["threshold_value"], "Score: ", children_list[1]["children"].attrib['score'].replace('-', '_'))

def pmml_parser(file_path, out_path):
    tree = ET.parse(file_path)
    root = tree.getroot()
    namespaces = {}
    namespaces["pmml"] = get_xmlns_uri(root)
    if namespaces is None:
        raise ValueError("The PMML file does not have the correct namespace")
    else:
        print("The PMML file has the correct namespace: ", namespaces)
    model_features, model_classes = get_features_and_classes_from_pmml(root, namespaces)
    print("Model features: ", model_features)
    print("Model classes: ", model_classes)
    trailer = ConfigTrailer(len(model_classes), len(model_features), 0)
    trees = []
    segmentation = root.find("pmml:MiningModel/pmml:Segmentation", namespaces)
    if segmentation is not None:  # If the model has a segmentation it consists of multiple trees  
        print("Segmentation found")
        for tree_id, segment in enumerate(segmentation.findall("pmml:Segment", namespaces)):
            tree_model_root = segment.find("pmml:TreeModel", namespaces).find("pmml:Node", namespaces)
            trailer.num_trees += 1
            print(f"Tree found Id {tree_id}")
            tree = get_tree_model_from_pmml(namespaces, tree_model_root, model_features, 0)
            trees.append(tree)
            #print_tree(tree)
    else:
        print("No segmentation found")
        trailer.num_trees += 1
        trees.append(get_tree_model_from_pmml(namespaces, tree_model_root, model_features, 0))
    
    write_bin(trailer, trees, out_path)
    # trailer_bytes = bytearray(trailer)
    # with open(out_path, "wb") as out_file:
    #     out_file.write(trailer_bytes)
    #     print("Trailer written")


def joblib_parser(file_path):
    print("Not implemented yet")
    exit(1)

def write_bin(trailer, trees, out_file):
    final_bin = bytearray(trailer)
    print("Written trailer of size: ", len(final_bin))
    for idx, tree in enumerate(trees):
        print(f"[Tree-ID: {idx}] Writing with number of nodes: ", len(tree))
        # Append the tree node size.
        final_bin += (bytearray(ctypes.c_uint16(len(tree))))
        print(f"[Tree-ID: {idx}] Size of the trailer with Number-Nodes: ", len(final_bin))
        for node in tree:
            final_bin += bytearray(node.tree_node_struct)
        print(f"[Tree-ID: {idx}] Size of the trailer with Tree: ", len(final_bin))
    
    with open(out_file, "wb") as out_file:
        out_file.write(final_bin)
        print(f"Binary file written, written size {len(final_bin)}")

def parse(model_source : str, out_path: str):
        if model_source.endswith(".pmml"):
            pmml_parser(model_source, out_path)
        elif model_source.endswith(".joblib"):
            joblib_parser(model_source)

""" Generate a c module that contains a number_of_inputs, taken to X_test to the module.
    X_test : Set of possible inputs.
                npy array 
    dataset_outcomes: Outcomes of the dataset for each input.
    file_path : path in which the file is saved.
"""
def render_module_test_header(X_test, dataset_outcomes, file_path):
    file_loader = FileSystemLoader(searchpath = "./")
    env = Environment(loader = file_loader)
    module_template = env.get_template("model_test.h.template")
    module_test = module_template.render(
                                    input_size      = len(X_test),
                                    feature_size    = len(X_test[0]),
                                    inputs          = X_test,
                                    ds_outs         = dataset_outcomes,
                                    )
    with open(f"{file_path}", "w") as out_file:
        out_file.write(module_test)

def gen_test_vec(model_source, dataset_source, target_column, out_path):
    if model_source.endswith(".pmml"):
        print("Can not reconvert the model from PMML to sklearn")
        print("Using the entire dataset to generate the test vectors")
        print("This consists in a simple accuracy comparison..")
        model = None
    elif model_source.endswith(".joblib"):
        print("Loaded joblib model")
        model = joblib.load(model_source)
    
    if model is not None:
        print("Loaded model")
        # For now not supported the joblib
    
    # Load the dataset
    print("Loading dataset")
    df = pd.read_csv(dataset_source, sep=";")
    dataset_outs = df[target_column].to_numpy()
    inputs = df.drop(columns=[target_column]).values
    print("Rendering dataset")
    render_module_test_header(inputs, dataset_outs, out_path)   
    print(f"Test vectors generated in: {out_path}")
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process and print NodeConfig data from a file.")
    parser.add_argument("command", type=str, help="The command to execute.")
    parser.add_argument("--input_model",  type=str, help="Path to the PMML or joblib files containing the model to serialize.", default =  "../datasets/statlog_segment/rf_5/rf_5.pmml")
    parser.add_argument("--output_bin",   type=str, help="Path to the output file to write the binary file.", default = "../examples/desktop/dtc_parse/statlog_rf5.bin")
    parser.add_argument("--input_dataset",  type=str, help="Path of the input dataset to parse", default =  "../datasets/statlog_segment/rf_5/test_dataset.csv")
    parser.add_argument("--output_test_vec",  type=str, help="Path to the output header containing the classification inputs and their outcomes", default = "../examples/desktop/inference_accuracy/model_test.h")
    parser.add_argument("--target_column",  type=str, help="Name of the target column of the input dataset", default = "Outcome")
    parser.add_argument("--csv_separator",  type=str, help="Separator of the csv file of the dataset", default = ";")
    args = parser.parse_args()

    if args.command == "parse":
        if args.input_model is None:
            print("The input model file is required.")
            exit(1)
        if args.output_bin is None or not args.output_bin.endswith(".bin"):
            print("Invalid output file. The output file must be a binary file.")
            exit(1)
        parse(args.input_model, args.output_bin)
    elif args.command == "gen_test_vec":
        if args.input_model is None:
            print("The input model file is required for the generation of the C-test vectors.")
            exit(1)
        if args.input_dataset is None:
            print("The input dataset file is required for the generation of C-test vectors.")
            exit(1)
        if args.output_test_vec is None or not args.output_test_vec.endswith(".h"):    
            print("Invalid output file. The output file must be C header file.")
            exit(1) 
        if args.target_column is None:
            print("The target column is required for the generation of the C-test vectors.")
            exit(1)
        gen_test_vec(args.input_model, args.input_dataset, args.target_column, args.output_test_vec)    
    else:
        print("Invalid command.")
        exit(1)    
    # model = "../datasets/statlog_segment/rf_5/rf_5.pmml"
    # out_bin = "../examples/desktop/dtc_parse/statlog_rf5.bin"
    # # print(ctypes.sizeof(TreeNode))
    # # exit(1)
    # parse(model, out_bin)
