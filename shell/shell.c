#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>

#include "tokenizer.h"

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "prints the current working directory"},
  {cmd_cd, "cd", "changes the current working directory to the specified directory"},
};

/* Parse the $PATH envirment.
 * Returns a new array of pointers to each path value */
char **parse_path_env(char *path_env)
{
  char **pathes = (char **)malloc(4096 * sizeof(char *));
  int i;
  char *path;

  path = strtok(path_env, ":");
  for (i = 0; i < 4096 && path; i++) {
    pathes[i] = path;
    path = strtok(NULL, ":");
  }

  if (i == 4096 && path) {
    fprintf(stderr, "$PATH too long to parse\n");
    free(pathes);
    return NULL;
  }
  
  pathes[i] = NULL;
  return pathes;
}

/* Find program from the PATH environment */
char *resolve_path(char *path, char **pathes)
{
  char *ret;

  struct dirent *dir_entry;
  for (int i = 0; pathes[i]; ++i) {
    DIR *dir;
    if ((dir = opendir(pathes[i])) != NULL) {
      while ((dir_entry = readdir(dir)) != NULL) {
	if (strcmp(dir_entry->d_name, path) == 0) {
	  ret = (char *)malloc(strlen(dir_entry->d_name) + 1 + strlen(path) + 1);
	  strcpy(ret, pathes[i]);
	  strncat(ret, "/", 1);
	  strcat(ret, path);
	  return ret;
	}
      }
      closedir(dir);
    }
  }
  return NULL;
}

/* Prints a helpful description for the given command */
int cmd_help(struct tokens *tokens) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(struct tokens *tokens) {
  exit(0);
}

/* Prints the current working directory to standard output. */
int cmd_pwd(struct tokens *tokens) {
  char current_dir_name[4096];

  if (getcwd(current_dir_name, 4096) != NULL)
    printf("%s\n", current_dir_name);
  else
    perror("pwd: ");
  return 1;
}

/* Changes the current working directory to the specified directory. */
int cmd_cd(struct tokens *tokens) {
  char *path = tokens_get_token(tokens, 1);

  if (chdir(path) != 0)
    perror("cd: ");
  return 1;
}

/* Execute programs */
void Execv(struct tokens *tokens) {
  int tokens_length = tokens_get_length(tokens);
  char *path = tokens_get_token(tokens, 0);
  char **argv = (char **)malloc((tokens_length + 1) * sizeof(char*));
  pid_t pid;
  int status;
  bool path_from_env = false;

  /* If the path does not contain '/', then check PATH. */
  if (strchr(path, '/') == NULL) {
    char *path_env;
    char *env = getenv("PATH");
    if (env != NULL) {
      char **pathes;
      char *tmp;
      path_env = (char *) malloc(strlen(env) + 1);
      strcpy(path_env, env);
      pathes = parse_path_env(path_env);
      if ((tmp = resolve_path(path, pathes)) != NULL) {
	path = tmp;
	path_from_env = true;
      }
      free(path_env);
    }
  }
    
  for (int i = 0; i < tokens_length; ++i)
    argv[i] = tokens_get_token(tokens, i);
  argv[tokens_length] = NULL;
  
  pid = fork();
  if (pid < 0)
    perror("fork: ");
  else if (pid == 0) {
    if (execv(path, argv) == -1) {
      perror("execv: ");
      exit(1);
    }
  }
  wait(&status);
  if (path_from_env)
    free(path);
  free(argv);
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int main(int argc, char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      /* Run commands as programs. */
      Execv(tokens);
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
