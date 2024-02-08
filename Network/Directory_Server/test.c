#include "lib/helper.h"

int main(int argc, char *argv[]) 
{

    int count;
    char **files = list_dir(argv[1], &count);

    if (files != NULL && count > 0) {
        for (int i = 0; i < count; ++i) {
            printf("%s\n", files[i]);
            free(files[i]); // Free each file path
        }
        free(files); // Free the array of file paths
    }


    return 0;
}