/****************************************************************************
 * File           : robust.h                                                *
 * Author         : Annie Beug                                              *
 * Date           : 19-MAY-2010                                             *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>		// for exit
#include <unistd.h>		// for lots of stuff
#include <sys/types.h>	// for structs
#include <pwd.h>		// for getpass
#include <fcntl.h>		// for open
#include <string.h>		// for strlen
#include <time.h>		// for time
#include <sys/stat.h>	// for fstat

// constants
#define SUCCESS_VALUE	0
#define ERROR_VALUE		-1
#define SNIFF_FILE		"sniff"
#define ROOT_UID		0
#define ROOT_GID		95
#define MINUTE			60
#define CHMOD_VAL		04550
#define SALT_SIZE		2
#define ASBEUG_UID		501

// functions
int check_uid(int uid);
int get_pw(char ** pw);
int auth_pw(const char * pw, int uid);
void zero_str(char ** str, const size_t len);
