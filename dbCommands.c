#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cjson\cJSON.h" // have cjon in the include folder


// global vars
//char *databaseFileName = "data/db_default.json";
char *databaseFileName = "data/db.json";

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

void chooser() {}

void addLogin() {}

void removeLogin() {}

void changeLogin() {}

void t() {}; // test

// classes
struct DictDB {
    cJSON *data;
    int uLength; //username max length
    int pLength; //password max length
};



// DictDB methods
void init_DictDB(struct DictDB *p, cJSON *data) {
    if (p) {
        p->data = data;

        int duL = 40; // default username max length value
        int dpL = 40; // default password ..

        cJSON *length = cJSON_GetObjectItem(data, "length");
        if (length) {

            cJSON *uL = cJSON_GetObjectItem(length, "username");
            printf("%s", uL->valuestring);
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
            printf("length key missing in %s", databaseFileName);
        }
    }
}

int main() {
    
    char *data = read_file(databaseFileName);
    if (!data) {
        printf("read error\n");
        return 1;
    }

    cJSON *dataP = cJSON_Parse(data);
    if (!dataP) {
        printf("parse error\n");
        free(data);
        return 1;
    }

    struct DictDB d1;
    init_DictDB(&d1, dataP);


    return 0;
}

//gcc .\dbCommands.c .\cjson\cJSON.c -o .\dbCommands