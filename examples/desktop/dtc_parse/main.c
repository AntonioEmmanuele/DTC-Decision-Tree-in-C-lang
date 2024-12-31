#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../src/tree_conf.h"
#include "../../../src/tree_visit.h"
#define FILENAME "statlog_rf5.bin"


int main() {
    // Open the binary file for reading
    FILE *file = fopen(FILENAME, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Read the structure from the file
    bin_trailer_t tree_trailer;
    if (fread(&tree_trailer, sizeof(bin_trailer_t), 1, file) != 1) {
        perror("Error reading file");
        fclose(file);
        return EXIT_FAILURE;
    }
    printf("Num Classes: %u\n", tree_trailer.num_classes);
    printf("Num Features: %u\n", tree_trailer.num_features);
    printf("Num Trees: %u\n", tree_trailer.num_trees);
    // Allocate a pointer to a pointer to a node_t ( each node is the first element of a tree)
    node_t ** trees = (node_t **) malloc(tree_trailer.num_trees * sizeof(node_t *));
    uint16_t num_nodes = 0;
    for(int t = 0; t < tree_trailer.num_trees; t++){
        // Read the number of nodes in the tree
        fread(&num_nodes, sizeof(num_nodes), 1, file);
        // Allocate the nodes of the tree
        trees[t] = (node_t *) malloc(num_nodes * sizeof(node_t));
        if (fread(trees[t], sizeof(node_t), num_nodes, file) != num_nodes) {
            perror("Error reading file");
            fclose(file);
            free(trees);
            return EXIT_FAILURE;
        }    
        printf("Tree: %u Num nodes %u\n", t, num_nodes);
        for(int i = 0; i < num_nodes; i++){
            printf("Node %u, Feature Idx: %u , Operator: %u, Thd: %f, RightIdx: %d, LeftIdx: %d, Class: %d \n", i, trees[t][i].feature_index, trees[t][i].operator, trees[t][i].threshold, trees[t][i].right_node, trees[t][i].left_node, trees[t][i].class);
        }
    }
    
    // Free different trees mem areas
    for(int i = 0; i < tree_trailer.num_trees; i++){
        free(trees[i]);
    }
    // Free the initial trees pointer
    free(trees);
    // Close the file
    fclose(file);
    
    return EXIT_SUCCESS;
}
