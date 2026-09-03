#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "cjson\cJSON.h" // git clone https://github.com/DaveGamble/cJSON

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

// classes

struct DictOS {
    cJSON *data;
    cJSON *users;
    cJSON *commands;

    char *fileName;

    char *currentUser;
    int privilege; 


};

struct DictDB {
    cJSON *data;
    int uLength; //username max length
    int pLength; //password max length

    char *fileName;
};

#define NAME_MAX_SIZE_DICTOS 20
#define PASSWORD_MAX_SIZE_DICTOS 20
#define NAME_MAX_SIZE_DICTDB 20
#define PASSWORD_MAX_SIZE_DICDB 20

#define arrlen(x) (sizeof(x)/sizeof(x[0]))
#define test (printf("test\n"))
#define IntToStringSizeof(n) (int)((ceil(log10(n))+1)*sizeof(char))

char *getHostName() {
    static char hostname[256];
    size_t size = sizeof(hostname);

    #ifdef _WIN32
        DWORD dsize = size;
        if (!GetComputerNameA(hostname, &dsize)) {
            return NULL;
        }
    #else
        if (!gethostname(hostname, size)) {
            return NULL;
        }
    #endif
    hostname[sizeof(hostname) - 1] = '\0';
    return (char *)hostname;

}

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

char *hash(const char *str) {
    int size = strlen(str);

    int power = 6761;
    int modulo = 1e9+9;
    long long hash_value = 0;
    long long p_power = 1;
    for (int i = 0; i < size; i++) {
        hash_value = (hash_value + (str[i] - 'a' + 1) * p_power) % modulo;
        p_power = (p_power * power) % modulo;
    }

    int hash_string_size = IntToStringSizeof(hash_value);
    char *hash_string = malloc(sizeof(char) * hash_string_size);
    sprintf(hash_string, "%d", hash_value);
    return hash_string;

    return hash_string;
    
}

void noNewline(char *str) {
    str[strcspn(str, "\n")] = '\0';
}

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

char *join(char **array, const size_t arrSize, const char *sep) {
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

char *slice(const char *src, int start, int length) {
    int src_length = strlen(src);

    char *dst = malloc(length + 1);
    memcpy(dst, &src[start], length);
    dst[length] = '\0';
    
    return dst;
}

int count_occ(char *str, char *occ) {
    int str_l = strlen(str);
    if (str_l) {
        int occ_l = strlen(occ);
        if (!occ_l) {
            occ = " ";
            occ_l = 1;
        }

        int count = 0;
        int i = 0;
        while (i < str_l - occ_l + 1) {
            if (!strcmp(slice(str, i, occ_l), occ)) {
                count++;
                i += occ_l;
            } else {
            i++;
            }
        }
        return count;
    }
    return 0;
}

char **split(char *str, char* sep, int *size) {
    char *copy = malloc(strlen(str)+1);
    strcpy(copy, str);
    *size = 0;
    if (copy) {

        if (!sep) {
            sep = " ";
        }

        int count = count_occ(copy, sep);
        
        if (count) {
            //*size = count+1;
            char *token = strtok(copy, sep);
            char **array = malloc((count+1)*sizeof *array);
            int i = 0;
            while (token != NULL) {
                // printf("token : %s\n", token);
                array[i] = malloc(strlen(token));
                strcpy(array[i], token);
                token = strtok(NULL, sep);
                *size = ++i;
            }   
            
            return array;
        }
        *size = 1;
        char **array = malloc(2*sizeof *array);
        array[0] = malloc(strlen(str));
        strcpy(array[0], str);
        return array;
    } 
    *size = 0;
    char **array = malloc(2*sizeof *array);
    return NULL;
}

int arrlen2(char **arr) {
    int i;
    for (i = 0; arr[i] != NULL; i++) {
    }
    return i;
    
}

/* ========================
        DictOS Methods
   ======================== */

void init_DictOS(struct DictOS *p) {
    p->fileName = "data/users.json";
    cJSON *datafile = read_fileTocJSON(p->fileName);
    if (datafile) {
        p->data = datafile;
        p->users = cJSON_GetObjectItem(p->data, "users");
        p->commands = cJSON_GetObjectItem(p->data, "commands");

        p->currentUser = NULL;


    }
}

void save_usersOS(struct DictOS *p) {
    char *file_str = cJSON_Print(p->data);
    

    FILE *fp = fopen(p->fileName, "w");
    if (fp) {
        fputs(file_str, fp);
        fclose(fp);
    }

    free(file_str);
}

bool userInOS(struct DictOS *p, char *name) {
    cJSON *dict = NULL;
    cJSON_ArrayForEach(dict, p->users) {
        if (!strcmp(cJSON_GetObjectItem(dict, "name")->valuestring, name)) {
            return true;
        }
    }
    return false;
}

char *getLastUser(struct DictOS *p) {
    return cJSON_GetObjectItem(p->data, "last")->valuestring;
}

char *getPasswordInputOfUser(char *user){
    static char pswdInput[PASSWORD_MAX_SIZE_DICTOS];
    printf("type password of %s : ", user);
    fgets(pswdInput, sizeof(pswdInput), stdin);
    noNewline(pswdInput);
    return pswdInput;
}
bool verifyPassword(struct DictOS *p, char *username, char *password) {
    cJSON *item = NULL;

    cJSON_ArrayForEach(item, p->users) {
        char *n = cJSON_GetObjectItem(item, "name")->valuestring;
        char *p = cJSON_GetObjectItem(item, "password")->valuestring;
        if (!strcmp(n, username)) {
            if (!strcmp(p, password)) {
                return true;
            }
        }
    }
    return false;
}

bool atLeastXUserInOS(struct DictOS *p, int x) {
    return (cJSON_GetArraySize(p->users) ?: x-1) >= x;
}



void addUserOS(struct DictOS *p, char *name, char *password) {
    
    cJSON *user = cJSON_CreateObject();
    cJSON_AddStringToObject(user, "name", name);
    cJSON_AddStringToObject(user, "password", password);

    cJSON_AddItemToArray(p->users, user);

    save_usersOS(p);
    printf("user %s added\n", name);
}

void removeUserOS(struct DictOS *p, char *name) {
    
    int count = cJSON_GetArraySize(p->users);

    for (int i = 0; i < count; i++) {
        cJSON *user = cJSON_GetArrayItem(p->users, i);

        if (!strcmp(cJSON_GetObjectItem(user, "name")->valuestring, name)) {

            cJSON_DeleteItemFromArray(p->users, i);
            break;
        }
    }

    save_usersOS(p);
    printf("user %s removed\n", name);
}

void changeCurrentUser(struct DictOS *p, char *name) {
    cJSON *old = cJSON_GetObjectItem(p->data, "last");
        if (strcmp(old->valuestring, name)) {
            cJSON_SetValuestring(old, name);
            save_usersOS(p);
        }
    p->currentUser = name;
}

void getlocaluser(struct DictOS *p) {
    cJSON *dict = NULL;
    printf("Name\n----\n");
    cJSON_ArrayForEach(dict, p->users) {
        printf("%s \n", cJSON_GetObjectItem(dict, "name")->valuestring);
    }
    printf("\n");
}

void newlocaluser(struct DictOS *p, char *input, char **inputArr, int inputArrSize) {
    if (inputArrSize == 5) {
        if (!strcmp(inputArr[1], "-name") || !strcmp(inputArr[3], "-password")) {
            int inputArrStringSize;
            char **inputArrStrings = split(input, "\"", &inputArrStringSize);
            if (inputArrStringSize == 4) {
                char *name = inputArrStrings[1];
                trim(name);
                if (!userInOS(p, name)) {
                    char *password = inputArrStrings[3];
                    addUserOS(p, name, password);
                } else {
                    printf("user '%s' already exist\n", name);
                }
                return;
            }
        }
    }
    printf("Invalid synthax\n");

}

void removelocaluser(struct DictOS *p, char *input, char **inputArr, int inputArrSize) {
    if (inputArrSize == 3) {
        if (!strcmp(inputArr[1], "-name")) {
            int inputArrStringSize;
            char **inputArrStrings = split(input, "\"", &inputArrStringSize);
            if (inputArrStringSize == 2) {
                char *name = inputArrStrings[1];
                if (!userInOS(p, name)) {
                    printf("user '%s' does not exist\n", name);                    
                } else if (!atLeastXUserInOS(p, 1)) {
                    printf("There need to be at least one user\n");
                } else {
                    char *password = inputArrStrings[3];
                    removeUserOS(p, name);
                }
                return;
            }
        }
    }
    printf("Invalid synthax\n");

}

void whoami(struct DictOS *p) {
    char *machineName = getHostName() ?: "?";
    char *user = p->currentUser ?: "?";
    printf("%s/%s\n\n", machineName, user);
}

void runas(struct DictOS *p, char *input, char **inputArr, int inputArrSize) {
    int userStringLength = 6; // strlen("/user:") 
    if (inputArrSize == 3) {
        int userArgSize = strlen(inputArr[1]);
        if (userArgSize > userStringLength) {
            char *userArg = inputArr[1];
            if (!strcmp(slice(userArg, 0, 6), "/user:") && count_occ(userArg, "\"") == 2) {
                int size = 0;
                char **userArr = split(userArg, "\"", &size);
                if (size == 2 && userArr[1]) {
                    char *username = userArr[1];
                    if (userInOS) {
                        char *password = getPasswordInputOfUser(username);
                        if (verifyPassword(p, username, password)) {
                            char *app = inputArr[2];
                            if (!strcmp(app, "powershell") || !strcmp(app, "powershell.exe")) {
                                changeCurrentUser(p, username);
                                return;
                            }
                        }
                    }
                    printf("1326 : Incorrect username or password.\n");
                    return;
                }                 
            }
        }
    }
    printf("Invalid Synthax\n");
}

void procedureNoUser(struct DictOS *p) {
    if (!atLeastXUserInOS(p, 1)) {
        char name[NAME_MAX_SIZE_DICTOS];
        char password[PASSWORD_MAX_SIZE_DICTOS];

        printf("No users found, please create a new user.\n");
        printf("name length max : %i\npassword length max : %i\n\n", NAME_MAX_SIZE_DICTOS, PASSWORD_MAX_SIZE_DICTOS);

        printf("name : ");
        fgets(name, sizeof(name), stdin);
        noNewline(name);

        printf("password : ");
        fgets(password, sizeof(password), stdin);
        noNewline(password);

        printf("\n");
        addUserOS(p, name, password);
        procedureNoUser(p);
    }
}



void procedureConnectAsLastUser(struct DictOS *p) {
    char *name = getLastUser(p);
    if (name && userInOS(p, name)) {
        printf("Last user was %s\n", name);
        char *password = getPasswordInputOfUser(name);
        if (verifyPassword(p, name, password)) {
            changeCurrentUser(p, name);
            return;
        } else {
            printf("1326 : Incorrect username or password.\n");
        }
    }
    
}

void userPromptOS(struct DictOS *p) {
    procedureNoUser(p);
    procedureConnectAsLastUser(p);
    char userInput[500];
    char **userInputPARSED;
    bool running = true;
    int arrsize;
    while (running) {
        printf("PS %s> ", p->currentUser ?: "?");
        fgets(userInput, sizeof(userInput), stdin);
        trtolower(userInput);

        if (strcmp(userInput, "") == 0 || strcmp(userInput, "help") == 0) {
            printf("Help Menu / commands [%s%s\n]\n", "\n  ", CJSON_join(p->commands, cJSON_GetArraySize(p->commands), "\n  "));
        } else if (!strcmp(userInput, "exit")){
            running = false;
        } else {
            userInputPARSED = split(userInput, " ", &arrsize);

            if (!strcmp(userInputPARSED[0], "get-localuser") || !strcmp(userInputPARSED[0], "glu")) {
                getlocaluser(p);
            }
            else if (!strcmp(userInputPARSED[0], "new-localuser") || !strcmp(userInputPARSED[0], "nlu")) {
                newlocaluser(p, userInput, userInputPARSED, arrsize);
            }
            else if (!strcmp(userInputPARSED[0], "remove-localuser") || !strcmp(userInputPARSED[0], "rlu")) {
                removelocaluser(p, userInput, userInputPARSED, arrsize);
            }
            else if (!strcmp(userInputPARSED[0], "whoami")) {
                whoami(p);
            }
            else if (!strcmp(userInputPARSED[0], "runas")) {
                runas(p, userInput, userInputPARSED, arrsize);

            } else {
                printf("please type \"help\"\n");
            }
        }
    }
}

/* ========================
        DictDB Methods
   ======================== */

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



int main() {
    
    struct DictOS Windows;
    init_DictOS(&Windows);

    struct DictDB database;
    init_DictDB(&database);

    userPromptOS(&Windows);

    return 0;
}


//gcc .\dbCommands.c .\cjson\cJSON.c -o .\dbCommands