#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "cjson\cJSON.h" // git clone https://github.com/DaveGamble/cJSON

// read file function thx chatGPT
char *read_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    rewind(fp);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    fread(buffer, 1, length, fp);
    buffer[length] = '\0';

    fclose(fp);
    return buffer;
}

cJSON *read_fileTocJSON(const char *filename) {
    char *data = read_file(filename);
    if (!data) {
        printf("read error\n");
        return NULL;
    }

    cJSON *dataP = cJSON_Parse(data); // P for parsed
    if (!dataP) {
        printf("parse error\n");
        free(data);
        return NULL;
    }
    return dataP;
}

// My Lambdas

#define arrlen(x) (sizeof(x)/sizeof(x[0]))

// My functions

void trim(char *str) {
    int start = 0;
    int end = strlen(str) - 1;

    while (isspace(str[start])) {
        start++;   
    }

    while (end > start && isspace(str[end])) {
        end--;
    }
    
    if (start > 0 || end < (strlen(str) - 1)) {
        memmove(str, str + start, end - start + 1);
        str[end - start + 1] = '\0';
    }
}

void all_tolower(char *str) {
    for (int i = 0; str[i]; i++) {
    str[i] = tolower(str[i]); // Convert each character to lowercase
    }
}

void trtolower(char *str) {
    trim(str);
    all_tolower(str);
}

char *join(char **array, const size_t arrSize , const char *sep) {
    if (arrSize) {
        int total_length = 1;
        for (size_t i = 0; i < arrSize; i++) {
            if (array[i]) {
                total_length += strlen(array[i]);
            }
            if (i < arrSize-1) {
                total_length += strlen(sep);
            }
        }

        char *concat = malloc(total_length);
        memset(concat, 0, total_length);
        for (size_t i = 0; i < arrSize; i++) {
            printf("concat : %s\n", concat);
            if (i) {
                strcat(concat, sep);
            }            
            strcat(concat, array[i]);
        }
        printf("result : %s\n", concat);
    }
    return NULL;
}

char *CJSON_join(cJSON *array, const size_t arrSize , const char *sep) {
    if (cJSON_IsArray(array) && arrSize) {
        cJSON *item = NULL;
        int total_length = 1;
        for (size_t i = 0; i < arrSize; i++) {
            item = cJSON_GetArrayItem(array, i);
            if (cJSON_IsString(item)) {
                total_length += strlen(item->valuestring);
            }
            if (i < arrSize-1) {
                total_length += strlen(sep);
            }
        }
        
        char *concat = malloc(total_length);
        memset(concat, 0, total_length);
        for (size_t i = 0; i < arrSize; i++) {
            item = cJSON_GetArrayItem(array, i);
            if (cJSON_IsString(item)) {
                if (i) {
                    strcat(concat, sep);
                }
                strcat(concat, item->valuestring);
            }
        }   
        return concat;
    }
    return NULL;
}

void chooser() {}

void addLogin() {}

void removeLogin() {}

void changeLogin() {}

// classes

struct DictOS {
    cJSON *data;
    cJSON *users;
    cJSON *commands;

    char *fileName;
};

struct DictDB {
    cJSON *data;
    int uLength; //username max length
    int pLength; //password max length

    char *fileName;
};


// DictDB methods

// init
void init_DictDB(struct DictDB *p) {

    p->fileName = "data/db.json";
    cJSON *datafile = read_fileTocJSON(p->fileName);
    if (datafile) {
        p->data = datafile;

        int duL = 40; // default username max length value
        int dpL = 40; // default password ..

        cJSON *length = cJSON_GetObjectItem(p->data, "length");
        if (length) {

            cJSON *uL = cJSON_GetObjectItem(length, "username");
            if (cJSON_IsNumber(uL)) {
                p->uLength = uL->valueint;
            }
            else {
                printf("default value used (username : %i)\n", duL);
                p->uLength = duL;
            }
            
            cJSON *pL = cJSON_GetObjectItem(length, "password");
            if (cJSON_IsNumber(pL)) {
                p->pLength = pL->valueint;
            }
            else{
                printf("default value used (password : %i)\n", dpL);
                p->pLength = duL;
            }
        }
        else {
            printf("length key missing in %s", p->fileName);
        }
    }
}

// DictOS methods

// init
void init_DictOS(struct DictOS *p) {
    p->fileName = "data/users.json";
    cJSON *datafile = read_fileTocJSON(p->fileName);
    if (datafile) {
        p->data = datafile;
        p->users = cJSON_GetObjectItem(p->data, "users");
        p->commands = cJSON_GetObjectItem(p->data, "commands");

    }
}

void userPromptOS(struct DictOS *p) {
    char userInput[100];
    bool running = true;
    char *sep = "\n  ";
    while (running) {
        printf(":");
        fgets(userInput, sizeof(userInput), stdin);
        trtolower(userInput);

        if (strcmp(userInput, "") == 0 || strcmp(userInput, "help") == 0) {
            printf("Help Menu / commands [%s%s\n]\n", sep, CJSON_join(p->commands, cJSON_GetArraySize(p->commands), sep));
        }
    }
}

char *getPswd(struct DictOS *p) {
    char userInput[50];
    printf("username : ");
    fgets(userInput, sizeof(userInput), stdin);
    trim(userInput);
    if ("size : %i",cJSON_GetArraySize(p->users)) {
        cJSON *dict = NULL;
        char *user;
        char *password;
        cJSON_ArrayForEach(dict, p->users) {
            user = cJSON_GetObjectItem(dict, "name")->valuestring;
            if (strcmp(userInput, user) == 0) {
                password = cJSON_GetObjectItem(dict, "password")->valuestring;
                return password;
            }
        }   
    }
    return NULL;
}

int main() {

    //char *t[] = {"hello", "hi"};

    struct DictOS Windows;
    init_DictOS(&Windows);

    struct DictDB database;
    init_DictDB(&database);

    userPromptOS(&Windows);

    return 0;
}

//gcc .\dbCommands.c .\cjson\cJSON.c -o .\dbCommands