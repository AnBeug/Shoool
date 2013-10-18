/**
 * This file contains constants and function prototypes for the
 * mksuid program.
 * @author asbeug
 **/

#define SUCCESS_VALUE   0
#define ERROR_VALUE     -1
#define SNIFF_FILE      "sniff"
#define ROOT_UID        0
#define ROOT_GID        95
#define MINUTE          60
#define CHMOD_VAL       04550
#define SALT_SIZE       2
#define ASBEUG_UID      501

int check_uid(int uid);
int get_pw(char ** pw);
int auth_pw(const char * pw, int uid);
void zero_str(char ** str, const size_t len);