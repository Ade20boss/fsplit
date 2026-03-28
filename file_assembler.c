#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_SIZE 4096

void assemble(char *filename, int part_no)
{
    size_t bytes_left_in_part;
    size_t part_size;
    char restored_name[256];

    // 1. Generate the restored filename
    int name_result = snprintf(restored_name, sizeof(restored_name), "restored_%s", filename);
    if (name_result < 0 || name_result >= (int)sizeof(restored_name)) {
        fprintf(stderr, "Error: Restored filename is too long.\n");
        exit(-1);
    }

    // 2. Overwrite Protection: Check if the file already exists
    FILE *check_pointer = fopen(restored_name, "rb");
    if (check_pointer != NULL) 
    {
        fprintf(stderr, "Error: File '%s' already exists. Aborting to prevent overwrite.\n", restored_name);
        fclose(check_pointer);
        exit(-1);
    }

    // 3. Open output file and allocate buffer
    FILE *output_pointer = fopen(restored_name, "wb");
    if (output_pointer == NULL)
    {
        fprintf(stderr, "Error: Cannot create output file '%s'\n", restored_name);
        exit(-1);
    }

    void *buffer = malloc(READ_SIZE);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error: Cannot allocate memory to buffer\n");
        fclose(output_pointer);
        exit(-1);
    }

    // --- PHASE 1: REASSEMBLY ---
    printf("Starting reassembly of %d parts...\n", part_no);
    for (int i = 0; i < part_no; i++)
    {   
        char partfilename[256];
        snprintf(partfilename, sizeof(partfilename), "%s_%d", filename, i+1);
        
        FILE *part_pointer = fopen(partfilename, "rb");
        if(part_pointer == NULL)
        {
            fprintf(stderr, "Error: Cannot open part file '%s'\n", partfilename);
            fclose(output_pointer);
            free(buffer);
            exit(-1);
        }

        // Determine part size
        if (fseek(part_pointer, 0, SEEK_END) != 0) {
            fprintf(stderr, "Error: Seek failed on %s\n", partfilename);
            fclose(part_pointer);
            fclose(output_pointer);
            free(buffer);
            exit(-1);
        }
        else
        {
            part_size = (size_t)ftell(part_pointer);
            rewind(part_pointer);
        }

        
        bytes_left_in_part = part_size;

        // Transfer bytes from part to restored file
        while(bytes_left_in_part > 0)
        {
            size_t to_read = (bytes_left_in_part > READ_SIZE) ? READ_SIZE : bytes_left_in_part;
            
            if (fread(buffer, 1, to_read, part_pointer) != to_read) {
                fprintf(stderr, "Error: Read failure on %s\n", partfilename);
                fclose(part_pointer);
                fclose(output_pointer);
                free(buffer);
                exit(-1);
            }

            if (fwrite(buffer, 1, to_read, output_pointer) != to_read) {
                fprintf(stderr, "Error: Write failure on output file\n");
                fclose(part_pointer);
                fclose(output_pointer);
                free(buffer);
                exit(-1);
            }
            bytes_left_in_part -= to_read;
        }
        
        fclose(part_pointer);
        printf("Merged %s successfully.\n", partfilename);
    }

    // Close reassembly resources BEFORE cleanup
    free(buffer);
    fclose(output_pointer);

    // --- PHASE 2: CLEANUP ---
    // We only reach this point if the entire reassembly succeeded
    printf("Reassembly complete. Cleaning up part files...\n");
    for (int i = 0; i < part_no; i++)
    {
        char part_to_delete[256];
        snprintf(part_to_delete, sizeof(part_to_delete), "%s_%d", filename, i + 1);
        
        if (remove(part_to_delete) != 0) {
            fprintf(stderr, "Warning: Could not delete part file %s\n", part_to_delete);
        }
    }

    printf("Done. Final file: %s\n", restored_name);
}

int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <original_filename> <number_of_parts>\n", argv[0]);
        exit(-1);
    }

    int part_count = atoi(argv[2]);
    if (part_count <= 0)
    {
        fprintf(stderr, "Error: Number of parts must be a positive integer.\n");
        exit(-1);
    }

    if (strlen(argv[1]) > 245)
    {
        fprintf(stderr, "Error: Filename is too long.\n");
        exit(-1);
    }

    assemble(argv[1], part_count);

    return 0;
}