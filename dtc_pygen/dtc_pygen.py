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
import struct
import ctypes

# Define the tree node c structure
class TreeNode(ctypes.Structure):
    _fields_ = [
        ("feature_index", ctypes.c_uint16), 
        ("threshold", ctypes.c_int32),
        ("class", ctypes.c_int16),
        ("left_node", ctypes.c_int32 ), # For now, it is easier to use the int32_t for direct addressing
        ("right_node", ctypes.c_int32), # even if the C code has the support for pointers. This makes initialization a lot easier.
    ]

# Model Trailer
class ConfigTrailer(ctypes.Structure):
    _fields_ = [
        ("num_classes", ctypes.c_uint16), 
        ("num_features" , ctypes.c_uint16),
        ("num_trees", ctypes.c_uint16),
    ]

def pmml_parser(file_path):
    tree = ET.parse(file_path)
    root = tree.getroot()
    trailer = ConfigTrailer()
    namespaces = {}
    namespaces["pmml"] = get_xmlns_uri(root)
    if namespaces is None:
        raise ValueError("The PMML file does not have the correct namespace")
    else:
        print("The PMML file has the correct namespace: ", namespaces)
    model_features, model_classes = get_features_and_classes_from_pmml(root, namespaces)
    print("Model features: ", model_features)
    print("Model classes: ", model_classes)
    trailer._fields_["num_classes"] = len(model_classes)
    trailer._fields_["num_features"] = len(model_features)
    trailer._fields_["num_trees"] = 0

def joblib_parser(file_path):
    print("Not implemented yet")
    exit(1)

@staticmethod
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

def parse(model_source : str):
        if model_source.endswith(".pmml"):
            pmml_parser(model_source)
        elif model_source.endswith(".joblib"):
            joblib_parser(model_source)

if __name__ == "__main__":
    model = "../example_dataset/statlog_segment/rf_5/rf_5.pmml"
    parse(model)
    print("Hello World")