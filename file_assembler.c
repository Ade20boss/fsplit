#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_SIZE 4096

void assemble(char *filename, int part_no)
{
    size_t bytes_left_in_part;
    size_t part_size;
    char restored_name[256];
    snprintf(restored_name, sizeof(restored_name), "restored_%s", filename);
    FILE *check_pointer = fopen(restored_name, "rb");
    if (check_pointer != NULL) 
    {
        fprintf(stderr, "Error: File '%s' already exists. Aborting to prevent overwrite.\n", restored_name);
        fclose(check_pointer);
        exit(-1);
    }

    FILE *output_pointer = fopen(restored_name, "wb");
    if (output_pointer == NULL)
    {
        fprintf(stderr, "Cannot open file\n");
        exit(-1);
    }

    void *buffer = malloc(READ_SIZE);
    if (buffer == NULL)
    {
        fprintf(stderr, "Cannot allocate memory to buffer\n");
        fclose(output_pointer);
        exit(-1);
    }

    for (int i = 0; i < part_no; i++)
    {   
        char partfilename[256];
        snprintf(partfilename, sizeof(partfilename), "%s_%d", filename, i+1);
        FILE *part_pointer = fopen(partfilename, "rb");

        if(part_pointer == NULL)
        {
            fprintf(stderr, "Cannot open file\n");
            fclose(output_pointer);
            free(buffer);
            exit(-1);
        }

        if (fseek(part_pointer, 0, SEEK_END) == 0)
        {
            part_size = ftell(part_pointer);
        }
        else
        {
            fprintf(stderr, "Unable to check file_size\n");
            fclose(output_pointer);
            fclose(part_pointer);
            free(buffer);
            exit(-1);
        }
        
        bytes_left_in_part = part_size;
        rewind(part_pointer);

        while(bytes_left_in_part > 0)
        {
            if (bytes_left_in_part > READ_SIZE)
            {
                fread(buffer, READ_SIZE, 1, part_pointer);
                fwrite(buffer, READ_SIZE, 1, output_pointer);
                bytes_left_in_part = bytes_left_in_part - READ_SIZE;     
            }
            else
            {
                fread(buffer, bytes_left_in_part, 1, part_pointer);
                fwrite(buffer, bytes_left_in_part, 1, output_pointer);
                bytes_left_in_part = 0;

            }
        }
        fclose(part_pointer);


    }
    free(buffer);
    fclose(output_pointer);
}

int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <filename> <number-of-parts>\n", argv[0]);
        exit(-1);
    }
    if (atoi(argv[2]) <= 0)
    {
        fprintf(stderr, "Number of splits must be more than zero\n");
        exit(-1);
    }
    if (strlen(argv[1]) > 245)
    {
        fprintf(stderr, "Filename cannot be more than 256 characters\n");
        exit(-1);
    }

    assemble(argv[1], atoi(argv[2]));
}