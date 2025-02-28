
#include "lab.h"
#include <stdio.h>
#include <readline/readline.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <readline/history.h>



char *get_prompt(const char *env){
    char* environVar = getenv(env);
    if (environVar == NULL) {
        return strdup("shell>");
    }
    else{
        return strdup(environVar);
    }
}


int change_dir(char **dir){
    if (dir == NULL || dir[1] == NULL) {
        char* home = getenv("HOME");
        if (home == NULL) {
            // if home is not set, find home
            struct passwd *info = getpwuid(getuid());
            home = info->pw_dir;
        }
        if (chdir(home) != 0) {
            perror("change_dir");
            return -1;
        }
    }
    else{
        if(chdir(dir[1]) != 0){
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
    long maxArgs = sysconf(_SC_ARG_MAX);
    char** args = malloc(sizeof(char*) * (maxArgs + 1));
    char* lineCopy = strdup(line);
    char* trimmedLine = trim_white(lineCopy);
    free(lineCopy);

    char* token = strtok(trimmedLine, " ");

    int i = 0;
    while (token != NULL && i < maxArgs) {
        args[i] = strdup(token);
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = NULL;
    free(trimmedLine);
    return args;
}

void cmd_free(char ** line){
    if (line != NULL) {
        long maxArgs = sysconf(_SC_ARG_MAX);
        for (int i = 0; i < maxArgs; i++)
        {
            if (line[i] != NULL) {
                free(line[i]);
            }
            
        }
        free(line);
    }
    
}

char *trim_white(char *line){
    if (line == NULL) {
        return NULL;
    }

    char* lineCopy = strdup(line);

    // Trim leading whitespace
    char *start = lineCopy;
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
    char* out = strdup(start);
    free(lineCopy);

    return out;
}

bool do_builtin(struct shell *sh, char **argv){
    if (argv == NULL || *argv == NULL) {
        return false;
    }

    if (strcmp(argv[0], "exit") == 0) {
        sh_destroy(sh);
        exit(1);
    }

    else if (strcmp(argv[0], "cd") == 0) {
        char *path = argv[1];
        if (change_dir(&path) != 0) {
            return false;
        }
    }

    else if (strcmp(argv[0], "history") == 0) {
        HIST_ENTRY **history = history_list();
        if (history) {
            for (int i = 0; history[i] != NULL; i++) {
                printf("%d: %s\n", i + 1, history[i]->line);
            }
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
    
        // shell id
        sh->shell_pgid = getpid();
    
        // process group id
        if(setpgid(sh->shell_pgid, sh->shell_pgid) < 0){
            perror("setpgid");
            exit(1);
        }
        tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
        tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
    }
    
    // Ignore signals in the shell
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
}

void sh_destroy(struct shell *sh){
    free(sh->prompt);
    free(sh);
}

void parse_args(int argc, char **argv){
    int c;
    while ((c=getopt(argc, argv, "v")) != -1)
    {
        switch (c)
        {
        case 'v':
            // print version then skedaddle
            printf("Version %d.%d\n",lab_VERSION_MAJOR, lab_VERSION_MINOR);
            exit(0);
            break;
        
        default:
            break;
        }
    }
}