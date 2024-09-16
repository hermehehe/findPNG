#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>  /* for printf().  man 3 printf */
#include <stdlib.h> /* for exit().    man 3 exit   */
#include <string.h> /* for strcat().  man strcat   */
#include "png_def.h" 

void readpath(const char *str_dir, int *found_png)
{
    DIR *p_dir; 
    struct dirent *p_dirent;

    if ((p_dir = opendir(str_dir)) == NULL)
    {
        perror("opendir");
        return;  //if the direcory doesn't exist treating it as though there are no pngs and return to main
    }

    struct stat filestat;

    while ((p_dirent = readdir(p_dir)) != NULL)
    {
        char *str_path = p_dirent->d_name; /* relative path name! */
        // construct a full path
        char path[200] = {0};
        strcat(path, str_dir);
        strcat(path, "/");
        strcat(path, str_path);

        stat(path, &filestat); 

        if (strcmp(p_dirent->d_name, ".") == 0 || strcmp(p_dirent->d_name, "..") == 0) //don't want to print these so skip
        {
            continue;
        }

        if (S_ISDIR(filestat.st_mode)) //check if directory entry is a sub-directory
        {
            readpath(path, found_png); //recursive call to check sub-directory
        }

        else
        {
            FILE *f = fopen(path, "rb"); //open current file
            if (f == NULL){
                perror("fopen");
                break;
            }
            U8 *buf = (unsigned char *)malloc(MAX_INPUT_SIZE); 
            fread(buf, 1, PNG_SIG_SIZE, f); //load buffer with first 8 bytes

            
            if (is_png(buf) == 1) //check if image is a png
            {
              if(check_crc(buf, f) == 0) 
              {
                printf("%s\n", path); //print png file path
                *found_png = 1;
               }

            }
            free(buf);
            fclose(f);
        }
    }
    
    closedir(p_dir); 
}

int main(int argc, char **argv)
{
    int found_png = 0; //set to false

    if(argc != 2){
        printf("findpng: No PNG file found\n");
        return 0;
    }
    readpath(argv[1], &found_png); //reading direcotries and printing in readpath()

    if (found_png == 0) { //if no pngs were found in readpath()
        printf("findpng: No PNG file found\n");
    }
    
    return 0;
}
