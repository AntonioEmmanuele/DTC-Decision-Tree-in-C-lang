#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../src/tree_conf.h"
#define FILENAME "statlog_rf5.bin"



int main() {
    // Open the binary file for reading
    FILE *file = fopen(FILENAME, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Read the structure from the file
    bin_trailer_t data;
    if (fread(&data, sizeof(bin_trailer_t), 1, file) != 1) {
        perror("Error reading file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Close the file
    fclose(file);

    // Print the results
    printf("Num Classes: %u\n", data.num_classes);
    printf("Num Features: %u\n", data.num_features);
    printf("Num Trees: %u\n", data.num_trees);

    return EXIT_SUCCESS;
}
