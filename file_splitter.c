#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_SIZE 4096

void split(char *filename, int split_nums)
{   
    if (split_nums <= 0)
    {
        fprintf(stderr, "Number of splits cannot be less than or equal to zero\n");
        exit(-1);
    }

    size_t filesize;
    size_t bytes_left_in_part;
    void *buffer = malloc(READ_SIZE);
    if (buffer == NULL)
    {
        fprintf(stderr, "Cannot allocate memory to buffer\n");
        free(buffer);
        exit(-1);
    }

    FILE *file_pointer = fopen(filename, "rb");
    if (file_pointer == NULL)
    {
        fprintf(stderr, "Cannot open file\n");
        free(buffer);
        exit(-1);
    }



    if (fseek(file_pointer, 0, SEEK_END) == 0)
    {
        long ftell_check = ftell(file_pointer);
        if (ftell_check < 0){
            fprintf(stderr, "Error checking file size\n");
            fclose(file_pointer);
            free(buffer);
            exit(-1);
        }
        filesize = ftell_check;
    }
    else
    {
        fprintf(stderr, "Unable to check file_size\n");
        fclose(file_pointer);
        free(buffer);
        exit(-1);
    }
    size_t partsize = filesize / split_nums;
    size_t remainder = filesize % split_nums;
    printf("File_size: %zu bytes\n", filesize);
    printf("Part_size: %zu bytes\n", partsize);
    printf("Remainder_size: %zu bytes\n", remainder);
    


    rewind(file_pointer);

    for (int i = 0; i < split_nums; i++)
    {   
        if (i == split_nums - 1)
        {
            bytes_left_in_part = partsize + remainder;
        }
        
        else
        {
            bytes_left_in_part = partsize;
        }
       
        char partfilename[256];
        int result = snprintf(partfilename, sizeof(partfilename), "%s_%d", filename, i+1);
        if (result < 0 || result >= (int)sizeof(partfilename))
        {
            fprintf(stderr, "Filename too long\n");
            fclose(file_pointer);
            free(buffer);
            exit(-1);
        }
        printf("%s\n", partfilename);
        FILE *partfile_pointer = fopen(partfilename, "wb");
        if (partfile_pointer == NULL)
        {
            fprintf(stderr, "Cannot create file for this chunk\n");
            fclose(file_pointer);
            free(buffer);
            exit(-1);
        }

        while(bytes_left_in_part > 0)
        {
            size_t to_read = (bytes_left_in_part > READ_SIZE) ? READ_SIZE : bytes_left_in_part;
            
            // Read from the main file_pointer
            if (fread(buffer, 1, to_read, file_pointer) != to_read) {
                fprintf(stderr, "Error: Read failure on source file\n");
                fclose(partfile_pointer);
                fclose(file_pointer);
                free(buffer);
                exit(-1);
            }

            // Write to the current partfile_pointer
            if (fwrite(buffer, 1, to_read, partfile_pointer) != to_read) {
                fprintf(stderr, "Error: Write failure on %s\n", partfilename);
                fclose(partfile_pointer);
                fclose(file_pointer);
                free(buffer);
                exit(-1);
            }
            bytes_left_in_part -= to_read;
        }
        fclose(partfile_pointer);
        
       

    }
   
    fclose(file_pointer);
    free(buffer);

}


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <filename> <number-of-splits>\n", argv[0]);
        exit(-1);
    }
    if (strlen(argv[1]) > 245)
    {
        fprintf(stderr, "Filename cannot be more than 256 characters\n");
        exit(-1);
    }

    split(argv[1], atoi(argv[2]));

    return 0;
}