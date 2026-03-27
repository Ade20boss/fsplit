#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_SIZE 4096

void split(char *filename, int split_nums)
{   
    if (split_nums <= 0)
    {
        fprintf(stderr, "Number of splits cannot be less than or equal to zero\n");
        printf("Aborting\n");
        exit(-1);
    }

    long filesize;
    long bytes_left_in_part;
    void *buffer = malloc(READ_SIZE);
    if (buffer == NULL)
    {
        fprintf(stderr, "Cannot allocate memory to buffer\n");
        printf("Aborting\n");
        free(buffer);
        exit(-1);
    }

    FILE *file_pointer = fopen(filename, "rb");
    if (file_pointer == NULL)
    {
        fprintf(stderr, "Cannot open file\n");
        printf("Aborting\n");
        free(buffer);
        exit(-1);
    }



    if (fseek(file_pointer, 0, SEEK_END) == 0)
    {
        filesize = ftell(file_pointer);
        if (filesize < 0)
        {
            fprintf(stderr, "Error checking file size\n");
            printf("Aborting\n");
            fclose(file_pointer);
            free(buffer);
            exit(-1);
        }

    }
    else
    {
        fprintf(stderr, "Unable to check file_size\n");
        printf("Aborting\n");
        fclose(file_pointer);
        free(buffer);
        exit(-1);
    }
    long partsize = filesize / split_nums;
    long remainder = filesize % split_nums;
    printf("File_size: %ld bytes\n", filesize);
    printf("Part_size: %ld bytes\n", partsize);
    printf("Remainder_size: %ld bytes\n", remainder);
    


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
        if (result < 0 || result >= sizeof(partfilename))
        {
            fprintf(stderr, "Filename too long\n");
            printf("Aborting\n");
            fclose(file_pointer);
            free(buffer);
            exit(-1);
        }
        printf("%s\n", partfilename);
        FILE *partfile_pointer = fopen(partfilename, "wb");
        if (partfile_pointer == NULL)
        {
            fprintf(stderr, "Cannot create file for this chunk\n");
            printf("Aborting\n");
            fclose(file_pointer);
            free(buffer);
            exit(-1);
        }

        while (bytes_left_in_part > 0)
        {
            if(bytes_left_in_part > READ_SIZE)
            {
                 fread(buffer, READ_SIZE, 1, file_pointer);
                 fwrite(buffer, READ_SIZE, 1, partfile_pointer);
                 bytes_left_in_part = bytes_left_in_part - READ_SIZE;
                

            }
            else
            {
                 fread(buffer, bytes_left_in_part, 1, file_pointer);
                 fwrite(buffer, bytes_left_in_part, 1, partfile_pointer);
                  bytes_left_in_part = 0;
            }
           
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
        printf("Usage: %s <filename> <number-of-splits>\n", argv[0]);
        printf("Aborting\n");
        exit(-1);
    }
    if (strlen(argv[1]) > 245)
    {
        fprintf(stderr, "Filename cannot be more than 256 characters\n");
        printf("Aborting\n");
        exit(-1);
    }

    split(argv[1], atoi(argv[2]));

    return 0;
}

