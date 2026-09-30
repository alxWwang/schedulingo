#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void run_funcs(char* title, char* time_limit, int argc, char *argv[]){
    printf("Job Title: %s\n", title);
    printf("Time limit: %s\n", time_limit);
    printf("Command: ");
    for (int i = 0; i< argc; i++){
        printf("%s ", argv[i]);
    }
    printf("\n\n");
}


int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if(!file){
        // Handle the error if the file could not be opened
        perror("Failed to open file");
        return 1;
    }

    char buffer[512];
    int line_number = 0;
    while(fgets(buffer, sizeof(buffer), file)){
        line_number++;
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n'){
            buffer[len-1] = '\0';
        }
        if (buffer[0] == '#'|| strlen(buffer) == 0) { continue; }

        char **words = NULL;
        int count = 0;
        int cap = 0;

        printf("Line %d: %s ->", line_number, buffer);
        char *tokens = strtok(buffer, " \t");
        while (tokens != NULL){
            if (count == cap){
                cap = cap ? cap * 2 : 4;
                char **tmp = realloc(words, cap * sizeof(char *));
                if (!tmp){
                    fprintf(stderr, "Reallocation failed");
                    break;
                }
                words = tmp;
            }
            words[count] = strdup(tokens);
            count ++;
            printf("[%s]", tokens);
            tokens = strtok(NULL, " \t");
        }
        printf("\n");
        if (count > 2){
            run_funcs(words[0], words[1], count-2, &words[2]);
        }
        for (int i = 0; i< count; i++){
            free(words[i]);
        }
        free(words);
    }

    fclose(file);
    return 0;
}

