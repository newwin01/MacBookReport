#include "helper.h"

// reference Prof. Hong Shin's code
char ** list_dir(char* dirpath, int* count) {
    DIR* dir = opendir(dirpath);

    if (dir == NULL) {
        *count = 0;
        return NULL;
    }

    char** file_paths = NULL;
    *count = 0;

    for (struct dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
        if (entry->d_type != DT_DIR && entry->d_type != DT_REG)
            continue;

        char* filepath = (char*)malloc(strlen(dirpath) + 1 + strlen(entry->d_name) + 1);
        strcpy(filepath, dirpath);
        strcpy(filepath + strlen(dirpath), "/");
        strcpy(filepath + strlen(dirpath) + 1, entry->d_name);

        if (entry->d_type == DT_DIR) {

            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {

                int subdir_count;
                char** subdir_files = list_dir(filepath, &subdir_count);

                if (subdir_files != NULL && subdir_count > 0) {
                    file_paths = realloc(file_paths, (*count + subdir_count) * sizeof(char*));

                    for (int i = 0; i < subdir_count; ++i) {
                        file_paths[*count] = subdir_files[i];
                        (*count)++;
                    }

                    free(subdir_files);
                }
            }

        } else if (entry->d_type == DT_REG) {
            file_paths = realloc(file_paths, (*count + 1) * sizeof(char*));
            file_paths[*count] = filepath;
            (*count)++;
        }
    }

    closedir(dir);

    return file_paths;
}

char* extract_file_name(char *path) {
    char *fileName = strrchr(path, '/');
    
    if (fileName != NULL) {
        fileName++; 
    } else {
        fileName = path;
    }

    return fileName;
}

int fileExists(char *filename) {
    return access(filename, F_OK) == 0;
}



char * two_string_concat (char *string1, char *string2) {

    if (string1 == NULL) {
        return string2;
    }
    if (string2 == NULL) {
        return string1;
    }

    char *result_string = malloc(sizeof(char) * (strlen(string1) + strlen(string2) + 1));

    if (result_string == NULL) {
        fprintf(stderr, "%s", "Memory allocation failed\n");
        exit(-1);
    }

    strcpy(result_string, string1);

    strcat(result_string, string2);

    return result_string;
}

int remove_file_exists(char *file_path) {
    
    if (remove(file_path) == 0) {
        return 1; 
    } else {
        return 0; 
    }
}

int create_directory(char *file_path) {

    char *dir_name = strdup(file_path); 
    char *last_slash = strrchr(dir_name, '/'); 
    
    if (last_slash != NULL) {
        *last_slash = '\0'; 

        if (mkdir(dir_name, 0777) == 0) {

            #ifdef _DEUBG
                printf("Directory '%s' has been created.\n", dir_name);
            #endif

            free(dir_name); 
            return 1; 
        } else {
            free(dir_name); 
            return 0; 
        }
    } else {
        free(dir_name); 
        return 0; 
    }
}

char *make_space_string(int count) {

    char *space_string = (char *)malloc(count);

    for (int i = 0; i < count; i++) {
        space_string[i] = ' ';
    }

    space_string[count] = '\0';

    return space_string;

}   