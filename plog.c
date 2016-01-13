/*Copyright 2015 Vito Mule'

This file may be used subject to the terms and conditions of the
GNU Library General Public License Version 2 as published by the
Free Software Foundation.This program is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU Library General Public License for more
details.*/

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
  float version = 1.0;
  regex_t re_log;
  regex_t re_pid;
  int i;

  if (argc < 2) {
    printf("Usage: plog pid...\n");
    exit(255);
  }

  /*Allowed on the command line:
    --version
    -V
    /proc/nnnn
    nnnn
    where nnnn is any number that doesn't begin with 0.
    If --version or -V are present, further arguments are ignored
    completely.*/

  regcomp(&re_pid, "^((/proc/+)?[1-9][0-9]*|-V|--version)$",
          REG_EXTENDED|REG_NOSUB);

  for (i = 1; i < argc; i++) {
    if (regexec(&re_pid, argv[i], 0, NULL, 0) != 0) {
      printf("plog: invalid process id: %s\n", argv[i]);
      exit(1);
    }
    if (!strcmp("-V", argv[i]) || !strcmp("--version", argv[i])) {
      printf("plog version: %.1f\n", version);
      exit(0);
    }
  }

  regfree(&re_pid);

  regcomp(&re_log, "^(.*log)$",REG_EXTENDED|REG_NOSUB);

  /*At this point, all arguments are in the form /proc/nnnn
    or nnnn, so a simple check based on the first char is
    possible.*/

  struct dirent *namelist;
  char* fullpath = (char*) malloc(PATH_MAX+1);
  char* linkpath = (char*) malloc(PATH_MAX+1);
  char buf[PATH_MAX+1];
  DIR *proc_dir;

  if (argv[1][0] != '/') {
    strncpy(fullpath, "/proc/", PATH_MAX);
    strncat(fullpath, argv[1], PATH_MAX - strlen(fullpath));
    strncat(fullpath, "/fd/", PATH_MAX - strlen(fullpath));
  }
  else {
    strncpy(fullpath, argv[1], PATH_MAX);
    strncat(fullpath, "/fd/", PATH_MAX - strlen(fullpath));
  }

  printf("Pid no %s:\n", argv[1]);
  proc_dir = opendir(fullpath);
  if (!proc_dir) {
    perror("opendir PID dir: ");
    return 1;
  }
  while((namelist = readdir(proc_dir))) {
    strncpy(linkpath, fullpath, PATH_MAX);
    strncat(linkpath, namelist->d_name, PATH_MAX - strlen(linkpath));
    readlink(linkpath, buf, PATH_MAX -1);

    if (regexec(&re_log, buf, 0, NULL, 0) == 0) {
      printf("Log path: %s\n", buf);
    }
    memset(&linkpath[0], 0, sizeof(linkpath));
    memset(&buf[0], 0, sizeof(buf));
  }
  memset(&fullpath[0], 0, sizeof(fullpath));
  regfree(&re_log);
  return 0;
}
