#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

char error_message[30] = "An error has occurred\n";
int has_ran = 0;

void myPrint(char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}
void remove_spaces(char* input, char* output) {
    int j = 0;
    for(int i = 0; i <= strlen(input); i++) {
        if (input[i] != ' ' && input[i] != '\t') {
                output[j] = input[i];
                j++;          
            }
    }
}

void run_not_built_in(char* input_str, int* rd_flag, int *ard_flag, char* file_name) {
    char* token_list[7];
    char* token;
    char* rest = input_str;
    int i = 0;
    int status;
    while ((token = strtok_r(rest, " ", &rest))) {
        char no_spaces[200];
        remove_spaces(token, no_spaces);
        if(strcmp(no_spaces, "\n") != 0 && strcmp(no_spaces, "") != 0) {
            token_list[i] = malloc(sizeof(char) * 100);
            strcpy(token_list[i], no_spaces);
            char *curr_word = token_list[i];
            if (curr_word[strlen(curr_word)-1] == '\n') {
                curr_word[strlen(curr_word)-1] = '\0';
            }
        i++;
        }
    } 
    token_list[i] = NULL;
    int ret = fork();
    if (ret == 0) {
        if(*rd_flag == 1) {
            int temp = open(file_name, O_RDONLY);
            if(temp != -1) {
                write(STDOUT_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            int rdfd = open(file_name, O_WRONLY | O_CREAT, 0777);
            if (rdfd == -1) {
                write(STDOUT_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            dup2(rdfd,STDOUT_FILENO);
        }
        int srcfile = open(file_name, O_RDWR, 0777);
        int tempfile = open("myTempFile", O_RDWR | O_CREAT, 0777);
        char curr_char;
        if (*ard_flag == 1) {
            if(srcfile != -1) {
                while (read(srcfile, &curr_char, 1) != 0) {
                    write(tempfile, &curr_char, 1);
                }
                close(srcfile);
                remove(file_name);
                int new_srcfile = open(file_name, O_RDWR | O_CREAT, 0777);
                dup2(new_srcfile,STDOUT_FILENO);
            }
            else {
                int ardfd = open(file_name, O_WRONLY | O_CREAT, 0777);
                if (ardfd == -1) {
                    write(STDOUT_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
                dup2(ardfd,STDOUT_FILENO);
            }
        }
        if(execvp(token_list[0], token_list) == -1) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
            exit(0);
        }
        exit(0);
    }
    else {
        waitpid(ret, &status, 0);
        if (*ard_flag == 1) {
            int new_srcfile = open(file_name, O_RDWR | O_APPEND, 0777);
            char curr_char;
            int tempfile = open("myTempFile", O_RDWR, 0777);
            while((read(tempfile, &curr_char, 1)) != 0) {
                write(new_srcfile, &curr_char, 1);
            }
            close(new_srcfile);
            close(tempfile);
            remove("myTempFile");
        }
        return;
    }
}

void run_if_built_in(char* input_str, int rd_flag) {
    char nl = '\n';
    char* rest = input_str;
    char no_spaces[10];
    char* token;
    if (!input_str) {
            exit(0);
        }
    remove_spaces(input_str, no_spaces);
    if (strcmp(no_spaces, "exit\n") == 0) {
        if(rd_flag == 1) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
            has_ran = 1;
            return;
        }
        exit(0);
    }
    else if (strncmp(no_spaces, "exit", 4) == 0) {
        write(STDOUT_FILENO, error_message, strlen(error_message));
        has_ran = 1;
        return;
    }
    else if (strcmp(no_spaces, "pwd\n") == 0 || strcmp(no_spaces, "pwd") == 0) {
        if(rd_flag == 1) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
            has_ran = 1;
            return;
        }
        char curr_directory[4096];
        getcwd(curr_directory, 4096);
        strncat(curr_directory, &nl, 1);
        myPrint(curr_directory);
        has_ran = 1;
        return;
    }
    else if (strncmp(no_spaces, "pwd", 3) == 0) {
        write(STDOUT_FILENO, error_message, strlen(error_message));
        has_ran = 1;
        return;
    }
        int cdFlag = 0;
        char* dir;
        while ((token = strtok_r(rest, " ", &rest))) {
            if (cdFlag == 1 && strcmp(token, "\n") != 0) {
                dir = token;
                cdFlag = 2;
            }
            else if (cdFlag == 2 && strcmp(token, "\n") != 0) {
                cdFlag = 3;
            }
            else if(strcmp(token, "cd\n") == 0 || strcmp(token, "cd") == 0) {
                cdFlag = 1;
            }
        }
        if (cdFlag == 1) {
            if(rd_flag == 1) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
            has_ran = 1;
            return;
        }
            chdir(getenv("HOME"));
            has_ran = 1;
        }
        else if (cdFlag == 2) {
            has_ran = 1;
            if(rd_flag == 1) {
                write(STDOUT_FILENO, error_message, strlen(error_message));
                return;
            }
            if( dir[strlen(dir) - 1] == '\n') {
                dir[strlen(dir)-1] = '\0';
            }
            if (chdir(dir) == -1) {
                write(STDOUT_FILENO, error_message, strlen(error_message));
            }
        }
        else if (cdFlag == 3) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
            has_ran = 1;
        }
    }
    
void check_redirect(char* input, int* rd_flag, char* file_name, char* rest, int* has_ran) {
    char* redirect_token;
    char* redirect_rest = malloc(sizeof(char) * 512);
    strcpy(redirect_rest, input);
    int rd_counter = 0;
    while ((redirect_token = strtok_r(redirect_rest, ">", &redirect_rest))) {
        if (rd_counter == 0) {
            strcpy(rest, redirect_token);
        }
        if(rd_counter == 1) {
            *rd_flag = 1;
            remove_spaces(redirect_token, file_name);
            if (file_name[strlen(file_name) - 1] == '\n') {
                file_name[strlen(file_name)-1] = '\0';
            }
        }
        if(rd_counter > 1) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
            rest = "";
            *has_ran = 1;
            return;
        }
        rd_counter++;
    }
}

void check_adv_redirect(char* input, int* ard_flag, char* file_name, char* rest, int* has_ran) {
    char* redirect_token;
    char* redirect_rest = malloc(sizeof(char) * 512);
    strcpy(redirect_rest, input);
    int ard_counter = 0;
    while ((redirect_token = strtok_r(redirect_rest, "+", &redirect_rest))) {
        if (ard_counter == 0) {
            strcpy(rest, redirect_token);
            rest[strlen(rest)- 1] = '\0';
        }
        if(ard_counter == 1) {
            *ard_flag = 1;
            remove_spaces(redirect_token, file_name);
            if (file_name[strlen(file_name) - 1] == '\n') {
                file_name[strlen(file_name)-1] = '\0';
            }
        }
        if(ard_counter > 1) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
            rest = "";
            *has_ran = 1;
            return;
        }
        ard_counter++;
    }
}

int main(int argc, char *argv[]) {
    FILE* fd;
    char cmd_buff[1500];
    char *pinput;
    char* token;
    char token2[512];
    char* command_segment = malloc(sizeof(char) * 512);
    char* rest = malloc(sizeof(char) * 512);
    int rd_flag = 0;
    int ard_flag = 0;
    int batch_mode = 0;
    char* file_name = malloc(sizeof(char) * 200);
    
    if(argc > 2) {
        write(STDOUT_FILENO, error_message, strlen(error_message));
        exit(0);
    }
    if(argc == 2) {
        fd = fopen(argv[1],"r");
        if(fd == NULL) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
            exit(0);
        }
        else {
            batch_mode = 1;
        }
    }
    while (1) {
        rd_flag = 0;
        if(batch_mode == 0) {
            has_ran = 0;
            rd_flag = 0;
            ard_flag = 0;
            myPrint("myshell> ");
            pinput = fgets(cmd_buff, 1500, stdin);
            if(strlen(pinput) > 512) {
                write(STDOUT_FILENO, error_message, strlen(error_message));
                continue;
            }
            rest = pinput;
            while ((token = strtok_r(rest, ";", &rest))) {
                has_ran = 0;
                check_adv_redirect(token, &ard_flag, file_name, command_segment, &has_ran);
                if (ard_flag == 0) {
                    check_redirect(token, &rd_flag, file_name, command_segment, &has_ran);
                }
                strcpy(token2, command_segment);
                run_if_built_in(command_segment, rd_flag);
                if(has_ran == 0) {
                    run_not_built_in(token2, &rd_flag, &ard_flag, file_name);
                }
                rd_flag = 0;
            }
        }
        else {
            while ((pinput = fgets(cmd_buff, 1500, fd))) {
                if(strlen(pinput) > 512) {
                myPrint(pinput);
                write(STDOUT_FILENO, error_message, strlen(error_message));
                continue;
            }
                has_ran = 0;
                rd_flag = 0;
                ard_flag = 0;
                char no_spaces[512];
                remove_spaces(pinput, no_spaces);
                if (strcmp(no_spaces,"\n") != 0) {
                    myPrint(pinput);
                    if(strcmp(no_spaces, ">") == 0 || strcmp(no_spaces, ">\n") == 0) {
                        write(STDOUT_FILENO, error_message, strlen(error_message));
                        continue;
                    }
                    rest = pinput;
                    while ((token = strtok_r(rest, ";", &rest))) {
                        has_ran = 0;
                        check_adv_redirect(token, &ard_flag, file_name, command_segment, &has_ran);
                        if(ard_flag == 0) {
                            check_redirect(token, &rd_flag, file_name, command_segment, &has_ran);
                        }
                        strcpy(token2, command_segment);
                        run_if_built_in(command_segment, rd_flag);
                        if(has_ran == 0) {
                            run_not_built_in(token2, &rd_flag, &ard_flag, file_name);
                        }
                        rd_flag = 0;
                    }
                }  
            }
            exit(0);
        }
    }
}
