// Copyright 2015 Vito Mule'
//
// This file may be used subject to the terms and conditions of the
// GNU Library General Public License Version 2 as published by the
// Free Software Foundation.This program is distributed in the hope
// that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Library General Public License for more
// details.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

int main(int argc, char const *argv[]) {
  float version = 1.0;
  regex_t re_log;
  regex_t re_pid;
  int i;

  if (argc < 2) {
    printf("Usage: plog pid...\n");
    exit(255);
  }

  // Allowed on the command line:
  // --version
  // -V
  // /proc/nnnn
  // nnnn
  // where nnnn is any number that doesn't begin with 0.
  // If --version or -V are present, further arguments are ignored
  // completely.

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

  // At this point, all arguments are in the form /proc/nnnn
  // or nnnn, so a simple check based on the first char is
  // possible.

  int entries;
  struct dirent **namelist;
  char* fullpath = (char*) malloc(PATH_MAX+1);
  char* linkpath = (char*) malloc(PATH_MAX+1);
  char buf[PATH_MAX+1];

  if (argv[1][0] != '/') {
    strcpy(fullpath, "/proc/");
    strcat(fullpath, argv[1]);
    strcat(fullpath, "/fd/");
  }
  else {
    strcpy(fullpath, argv[1]);
    strcat(fullpath, "/fd/");
  }

  printf("Pid no %s:\n", argv[1]);
  entries = scandir(fullpath, &namelist, NULL, NULL);
  if (entries < 1) {
    perror("scandir");
  }
  else {;
    for (i = 0; i < entries; i++) {
      strcpy(linkpath, fullpath);
      strcat(linkpath, namelist[i]->d_name);
      readlink(linkpath, buf, PATH_MAX -1);

      if (regexec(&re_log, buf, 0, NULL, 0) == 0) {
        printf("Log path: %s\n", buf);
      }
      free(namelist[i]);
      memset(&linkpath[0], 0, sizeof(linkpath));
      memset(&buf[0], 0, sizeof(buf));
    }
  free(namelist);
  }
  memset(&fullpath[0], 0, sizeof(fullpath));

  return 0;
}
