/***************************************************************************
                           sysfs.c  -  description
                             -------------------
    begin             : 15.05.2010
    copyright         : (C) 2010 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "global.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "util.h"
#include "sysfs.h"

#define SYSFS_PATH_ENV		"SYSFS_PATH"

#define SYSFS_PATH_MAX		1024 //256
#define	SYSFS_NAME_LEN		64
#define SYSFS_BUS_ID_SIZE	32

/* mount path for sysfs, can be overridden by exporting SYSFS_PATH */
#define SYSFS_MNT_PATH		"/sys"



/**
 * sysfs_remove_trailing_slash: Removes any trailing '/' in the given path
 * @path: Path to look for the trailing '/'
 * Returns 0 on success 1 on error
 */
int32_t mhs_remove_trailing_slash(char *path)
{
size_t len;

if (!path)
  return(-1);

len = strlen(path);
while ((len > 0) && (path[len-1] == '/'))
  path[--len] = '\0';
return(0);
}


/*
 * sysfs_get_mnt_path: Gets the sysfs mount point.
 * @mnt_path: place to put "sysfs" mount point
 * @len: size of mnt_path
 * returns 0 with success and -1 with error.
 */
char *sysfs_get_mnt_path(void)
{
char *sys_path;
const char *sysfs_path_env;

sys_path = NULL;
/* possible overrride of real mount path */
sysfs_path_env = getenv(SYSFS_PATH_ENV);
if (sysfs_path_env != NULL)
  {
  sys_path = mhs_strdup(sysfs_path_env);
  mhs_remove_trailing_slash(sys_path);
  }
else
  sys_path = mhs_strdup(SYSFS_MNT_PATH);

return(sys_path);
}


/**
 * sysfs_get_link: returns link source
 * @path: symbolic link's path
 * @target: where to put name
 * @len: size of name
 */
char *mhs_get_link(const char *path)
{
char *devdir, *linkpath, *temp_path, *target;
char *d = NULL, *s = NULL;
int32_t slashes = 0, count = 0;

if (!path)
  return(NULL);  
devdir = NULL;
linkpath = NULL;
temp_path = NULL;
target = NULL;

devdir = mhs_malloc(SYSFS_PATH_MAX+1);
if (!devdir)
  goto sysfs_get_link_error;
memset(devdir, 0, SYSFS_PATH_MAX+1);
safe_strcpy(devdir, SYSFS_PATH_MAX+1, path);
linkpath = mhs_malloc(SYSFS_PATH_MAX+1);
if (!linkpath)
  goto sysfs_get_link_error;

memset(linkpath, 0, SYSFS_PATH_MAX+1);
if ((readlink(path, linkpath, SYSFS_PATH_MAX)) < 0)
  goto sysfs_get_link_error;

d = linkpath;
/*
 * Three cases here:
 * 1. relative path => format ../..
 * 2. absolute path => format /abcd/efgh
 * 3. relative path _from_ this dir => format abcd/efgh
 */
switch (*d)
  {
  case '.' : { //  handle the case where link is of type ./abcd/xxx
             temp_path = mhs_strdup(devdir);
             if (*(d+1) == '/')
               d += 2;
             else if (*(d+1) == '.')
               {  // relative path, getting rid of leading "../.."
               while ((*d == '/') || (*d == '.'))
                 {
                 if (*d == '/')
                 slashes++;
                 d++;
                 }
               d--;
               s = &devdir[strlen(devdir)-1];
               while (s != NULL && count != (slashes+1))
                 {
                 s--;
                 if (*s == '/')
                   count++;
                 }
               count = SYSFS_PATH_MAX-strlen(devdir);
               strncpy(s, d, count-1);
               s[count-1] = '\0';
               target = mhs_strdup(devdir);
               break;
               }
             s = strrchr(temp_path, '/');
             if (s != NULL)
               {
               *(s+1) = '\0';
               target = mhs_strconcat(temp_path, d, NULL);
               }
             else
               target = mhs_strdup(d);
             break;
             }
  case '/' : { // absolute path - copy as is
             target = mhs_strdup(linkpath);
             break;
             }
  default  : { // relative path from this directory
             temp_path = mhs_strdup(devdir);
             s = strrchr(temp_path, '/');
             if (s != NULL)
               {
               *(s+1) = '\0';
               target = mhs_strconcat(temp_path, linkpath, NULL);
               }
             else
               target = mhs_strconcat(temp_path, linkpath, NULL);
             }
  }

sysfs_get_link_error:
safe_free(devdir);
safe_free(linkpath);
safe_free(temp_path);
return(target);
}


/**
 * sysfs_path_is_dir: Check if the path supplied points to a directory
 * @path: path to validate
 * Returns 0 if path points to dir, 1 otherwise
 */
int32_t mhs_path_is_dir(const char *path)
{
struct stat astats;

if (!path)
  return(-1);
if ((lstat(path, &astats)) != 0)
  return(-1);
if (S_ISDIR(astats.st_mode))
  return(0);
return(-1);
}


/**
 * sysfs_path_is_link: Check if the path supplied points to a link
 * @path: path to validate
 * Returns 0 if path points to link, 1 otherwise
 */
int32_t mhs_path_is_link(const char *path)
{
struct stat astats;

if (!path)
  return(-1);
if ((lstat(path, &astats)) != 0)
  return(-1);
if (S_ISLNK(astats.st_mode))
  return(0);
return(-1);
}


/**
 * sysfs_path_is_file: Check if the path supplied points to a file
 * @path: path to validate
 * Returns 0 if path points to file, 1 otherwise
 */
int32_t mhs_path_is_file(const char *path)
{
struct stat astats;

if (!path)
  return(-1);
if ((lstat(path, &astats)) != 0)
  return(-1);
if (S_ISREG(astats.st_mode))
  return (0);
return(-1);
}


int32_t mhs_sys_read_value(char *dir, char *filename, char *data, int32_t max_size)
{
int32_t fd, len;
char *full_path, *s; //*tmp_dir

if (!data)
  return(-1);
data[0] = '\0';
if ((!dir) || (!filename))
  return(-1);
len = -1;  
/*tmp_dir = mhs_strdup(dir);
if ((s = strrchr(tmp_dir, '/')))
  *s = '\0';
if ((s = strrchr(tmp_dir, '/')))
  *s = '\0';*/
full_path = mhs_strconcat(dir, "/", filename, NULL);
if ((fd = open(full_path, O_RDONLY)) >= 0)
  {
  if ((len = read(fd, data, max_size-1)) >= 0)
    data[len] = '\0';
  else  
    data[0] = '\0';
  close(fd);
  }
if ((s = strrchr(data, '\n')))
  *s = '\0';
safe_free(full_path);
//safe_free(tmp_dir);
return(len);
}


uint32_t mhs_sys_read_as_ulong(char *dir, char *filename, int32_t base, int32_t *error)
{
int32_t err;
uint32_t value;
char *str, *endptr;

value = 0;
err = 0;
if (!(str = (char *)mhs_malloc(MAX_LINE_SIZE)))
  err = -1;
else
  {
  if (mhs_sys_read_value(dir, filename, str, MAX_LINE_SIZE) > 0)
    value = strtoul(str, (char**)&endptr, base);
  else
    err = -1;
  }  
safe_free(str);
if (error)
  *error = err;
return(value);
}

