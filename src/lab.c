
#include "lab.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pwd.h>



char *get_prompt(const char *env){
    char* environVar = getenv(env);
    if (environVar == NULL && *env != NULL) {
        return strdup("shell>");
    }
    else{
        return strdup(environVar);
    }
}


int change_dir(char **dir){
    if (dir == NULL || *dir == NULL) {
        char* home = getenv("HOME");
        if (home == NULL) {
            // if home is not set, find home
            struct passwd *info = getpwuid(getuid())->pw_dir;
        }
        if (chdir(home) != 0) {
            perror("change_dir");
            return -1;
        }
    }
    else{
        if(chdir(*dir) != 0){
            fprintf(stderr, "chdir failed: failed to change to %s: %s\n", *dir, strerror(errno));
            return -1;
        }
    }

    return 0;
    
}

char **cmd_parse(char const *line){
    if (line == NULL) {
        return NULL;
    }

    // Allocate memory for the arguments
    
    char** args = malloc(sizeof(char*) * (sysconf(_SC_ARG_MAX) + 1));
    char* lineCopy = strdup(line);
    char* token = strtok(lineCopy, " ");
    int i = 0;
    while (token != NULL && i < sysconf(_SC_ARG_MAX-1)) {
        args[i] = strdup(token);
        token = strtok(NULL, " ");
        i++;
    }
    return args;
}

void cmd_free(char ** line){
    if (line != NULL) {
        for (int i = 0; i < sysconf(_SC_ARG_MAX); i++)
        {
            free(line[i]);
        }
        free(line);
    }
    
}

char *trim_white(char *line){
    if (line == NULL) {
        return NULL;
    }

    // Trim leading whitespace
    char *start = line;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // Trim trailing whitespace
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    // Null-terminate the trimmed string
    *(end + 1) = '\0';

    return start;
}

bool do_builtin(struct shell *sh, char **argv){
    if (argv == NULL || *argv == NULL) {
        return false;
    }

    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }

    else if (strcmp(argv[0], "cd") == 0) {
        change_dir(argv[1]);
    }

    else if (strcmp(argv[0], "history") == 0) {
        HIST_ENTRY **history_list = history_list();
        if (history_list == NULL) {
            return false;
        }
        for (int i = 0; history_list[i] != NULL; i++) {
            printf("%d: %s\n", i + 1, history_list[i]->line);
        }
    }
    else{
        return false;
    }
    return true;
    
}

void sh_init(struct shell *sh){
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_is_interactive = isatty(sh->shell_terminal);

    //shell prompt
    if (sh->shell_is_interactive){
        while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp()))
        {
            kill(-sh->shell_pgid, SIGTTIN);
        }
    }
    
    // shell id
    sh->shell_pgid = getpid();

    // process group id
    if(setpgid(sh->shell_pgid, sh->shell_pgid) < 0){
        perror("setpgid");
        exit(1);
    }
}

void sh_destroy(struct shell *sh){
    free(sh->prompt);
    //sh not malloc'd in main, so no need to free sh
}

void parse_args(int argc, char **argv){
    for (int i = 0; i < argc; i++){
        if(strcmp(argv[i],"-v") == 0){
            // print version then skedaddle
            printf("Version %d.%d\n",lab_VERSION_MAJOR, lab_VERSION_MINOR);
            exit(0);
        }
    }
}