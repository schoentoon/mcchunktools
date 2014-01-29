/*  mcchunktools
 *  Copyright (C) 2013-2014  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mcchunk.h"

#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>

static const struct option g_LongOpts[] = {
  { "help",       no_argument,       0, 'h' },
  { "world",      required_argument, 0, 'w' },
  { "regionfile", required_argument, 0, 'r' },
  { 0, 0, 0, 0 }
};

char* to_byte_str(unsigned long long b) {
  const unsigned long long KiB = 1024;
  const unsigned long long MiB = KiB * 1024;
  const unsigned long long GiB = MiB * 1024;
  const unsigned long long TiB = GiB * 1024;

  char buf[BUFSIZ];
  if (b > TiB)
    snprintf(buf, sizeof(buf), "%f TiB", ((float) b)/TiB);
  else if (b > GiB)
    snprintf(buf, sizeof(buf), "%f GiB", ((float) b)/GiB);
  else if (b > MiB)
    snprintf(buf, sizeof(buf), "%f MiB", ((float) b)/MiB);
  else if (b > KiB)
    snprintf(buf, sizeof(buf), "%f KiB", ((float) b)/KiB);
  else
    snprintf(buf, sizeof(buf), "%llu B", b);
  return strdup(buf);
};

int usage(char* program) {
  fprintf(stderr, "Usage: %s [OPTIONS]\n", program);
  fprintf(stderr, "--world, -w Optimize this world.\n");
  fprintf(stderr, "--regionfile, -r Run it on a single region file instead of an entire world.\n");
  return 1;
};

int is_mca(const char* str) {
  if (str != NULL) {
    size_t size = strlen(str);
    if (size >= 4 && str[size-4] == '.' && str[size-3] == 'm' && str[size-2] == 'c' && str[size-1] == 'a')
      return 1;
  };
  return 0;
};

int loop_directory(const char* dir) {
  DIR* d = opendir(dir);
  if (d == NULL)
    return -1;
  int output = 0;
  struct dirent* dp;
  while ((dp = readdir(d)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
      continue;
    if (dp->d_type == DT_DIR) {
      char fullpath[strlen(dir) + strlen(dp->d_name) + 2];
      if (snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, dp->d_name) > 0)
        output += loop_directory(fullpath);
    } else if (dp->d_type == DT_REG && is_mca(dp->d_name) == 1) {
      char fullpath[strlen(dir) + strlen(dp->d_name) + 2];
      if (snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, dp->d_name) > 0) {
        regionfile* region = open_regionfile(fullpath);
        int cleaned_up = cleanup_region(region);
        if (cleaned_up > 0)
          output += cleaned_up;
        else {
          fprintf(stderr, "An error occured while processing %s, exiting.\n", fullpath);
          exit(1);
        };
        fprintf(stderr, "%s:%d\n", fullpath, cleaned_up);
        free_region(region);
      };
    };
  };
  closedir(d);
  return output;
};

int main(int argc, char** argv) {
  char* wdir = NULL;
  {
    int arg, optindex;
    while ((arg = getopt_long(argc, argv, "hw:r:", g_LongOpts, &optindex)) != -1) {
      switch (arg) {
      case 'h':
        return usage(argv[0]);
      case 'w':
        wdir = optarg;
        break;
      case 'r': {
        regionfile* region = open_regionfile(optarg);
        int cleaned_up = cleanup_region(region);
        if (cleaned_up < 0) {
          fprintf(stderr, "An error occured while processing %s, exiting.\n", optarg);
          return 1;
        };
        fprintf(stderr, "%s:%d\n", optarg, cleaned_up);
        free_region(region);
        if (!wdir)
          wdir = (char*) 1; // Just to indicate we did at least something so we don't exit with non zero.
        break;
      }
      }
    }
  }
  if (!wdir)
    return usage(argv[0]);
  else if (wdir == (char*) 1)
    return 0;
  {
    size_t len = strlen(wdir);
    if (wdir[len-1] == '/')
      wdir[len-1] = '\0';
  }
  int cleaned_up = loop_directory(wdir);
  char* byte_str = to_byte_str(cleaned_up);
  fprintf(stderr, "Zeroed out a total of %s.\n", byte_str);
  free(byte_str);
  return 0;
};