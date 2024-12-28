#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define FILENAME "statlog_rf5.bin"

// Define the C structure
typedef struct {
    uint16_t field1;
    uint16_t field2;
    uint16_t field3;
} bin_trailer_t;

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
    printf("Field1: %u\n", data.field1);
    printf("Field2: %u\n", data.field2);
    printf("Field3: %u\n", data.field3);

    return EXIT_SUCCESS;
}
