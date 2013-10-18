/**
 * This program performs various checks to see who is running the program and
 * to make sure that the sniff file is in a certain state. If all checks are 
 * successful, the program sets the owner/group of the sniff file to root and
 * changes to the mode to readable by owner, readable and executable to group
 * and other.
 * @author asbeug
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mksuid.h"

int main (int argc, char * const argv[]) {

    // Check that the user is running the program--i.e. real id == eff id
    if (check_uid(ASBEUG_UID) == ERROR_VALUE)
    {
        printf("ERROR: real user id and effective user id differ\n");
        exit(ERROR_VALUE);
    }

    // Prompt user for password
    char * u_pw = malloc(_PASSWORD_LEN); // check return
    char ** u_pw_ptr = &u_pw;
    if (get_pw(u_pw_ptr) == ERROR_VALUE)
    {
        printf("ERROR: could not read password\n");
        exit(ERROR_VALUE);
    }

    // Authenticate password for user
    // We know that the real uid is ASBEUG_UID
    if (auth_pw(u_pw, ASBEUG_UID) == ERROR_VALUE)
    {
        printf("ERROR: password incorrect\n");
        exit(ERROR_VALUE);
    }

    // Clear password buffer in memory
    zero_str(u_pw_ptr, strlen(*u_pw_ptr));

    // Check to see if current dir contains "sniff" file
    // Open by file descriptor to prevent someone messing with the file later
    // File should be the actual file and not a symbolic link
    int fd = open(SNIFF_FILE, O_RDONLY, O_NOFOLLOW);
    if (fd <= ERROR_VALUE)  // open will only return an error value 
    {
        printf("ERROR: could not open file\n");
        exit(ERROR_VALUE);
    }

    // Check the stats on the sniff file
    // Check that malloc worked
    struct stat * stat_ptr = malloc(sizeof(struct stat));
    if (stat == NULL)
    {
        printf("ERROR: could not allocate memory for stat\n");
        exit(ERROR_VALUE);
    }
    
    // Again, open by file descriptor
    if (fstat(fd, stat_ptr) <= ERROR_VALUE)
    {
        printf("ERROR: could not stat file\n");
        exit(ERROR_VALUE);
    }

    // Make sure that file is:
    // 1) owned by user
    if (stat_ptr->st_uid != getuid())
    {
        printf("ERROR: user does not own file\n");
        exit(ERROR_VALUE);
    }

    // 2) executable by owner
    // this code based on: http://fixunix.com/unix/395730-help-st_mode-sys-stat-h.html
    // This masks out all bits except for the one that denotes user
    // executable
    // If it's 0 that means it is not set
    if ((stat_ptr->st_mode & S_IXUSR) == 0)
    {
        printf("ERROR: user does not have execute permissions on file\n");
        exit(ERROR_VALUE);
    }

    // 3) not readable, writable, or executable by anyone else (group or other)
    // Mask out all bits except for group and "other" bits for read, write,
    // and execute
    // We expect these bits to be false / off / not set
    if ((stat_ptr->st_mode & S_IRWXG) != 0 || (stat_ptr->st_mode & S_IRWXO) != 0)
    {
        printf("ERROR: other user has permissions on file\n");
        exit(ERROR_VALUE);
    }

    // 4) not modified more than a minute ago or in the future
    // Check difference between modification time and current time
    // Shouldn't be more than 60 seconds
    if (stat_ptr->st_mtimespec.tv_sec + MINUTE < time(NULL) 
        || stat_ptr->st_mtimespec.tv_sec > time(NULL))
        // TODO do we care about millisecs?
    {
        printf("ERROR: file was modified more than a minute ago\n");
        exit(ERROR_VALUE);
    }

    // Change ownership of sniff to root (uid = 0; gid = 95)
    if (fchown(fd, ROOT_UID, ROOT_GID) <= ERROR_VALUE)
    {
        perror("ERROR: chown to root failed\n");
        exit(ERROR_VALUE);
    }

    // Change mode to 04550
    // 0 == octal
    // 4 == setuid 
    // 5 == readable + executable for user
    // 5 == readable + executable for group
    // 0 == no permissions for other
    int mode = CHMOD_VAL;
    if (fchmod(fd, mode) <= ERROR_VALUE)
    {
        perror("ERROR: chmod on file failed\n");
        exit(ERROR_VALUE);
    }

    return 0;
}

/**
 * Checks that my uid is running this process.
 * If so, returns 0, otherwise returns -1.
 **/
int check_uid(int uid)
{
    if (getuid() == uid)
    {
        return SUCCESS_VALUE;
    }
    else
    {
        return ERROR_VALUE;
    }
}

/**
 * Gets the plain text password from the user.
 * Passed in buffer should be of length _PASSWORD_LEN which is what
 * getpass should return.
 **/
int get_pw(char ** pw)
{
    // We don't want someone without a password (i.e. length less than 1)
    if ((*pw = getpass("Password: ")) == NULL || strlen(*pw) < 1)
    {
        return ERROR_VALUE;
    }
    else
    {
        return SUCCESS_VALUE;
    }
}

/**
 * Checks to see if password matches value in pw file.
 * Checks against the password entry for the running
 **/
int auth_pw(const char * pw, int uid)
{
    // Get the user's password by their id
    struct passwd * pw_ptr = getpwuid(uid);
    if (pw_ptr  == NULL)
    {
        printf("ERROR: could not get password entry\n");
        return ERROR_VALUE;
    }

    // Get the first two chars for the salt
    // Check that malloc worked
    char * salt = malloc(SALT_SIZE);
    if (salt == NULL)
    {
        perror("ERROR: could not allocate space for salt\n");
        return ERROR_VALUE;
    }
    // Check that memcpy worked
    if (memcpy(salt, pw_ptr->pw_passwd, SALT_SIZE) == NULL)
    {
        perror("ERROR: could not copy in to salt\n");
        return ERROR_VALUE;
    }
    
    // crypt plain text pw
    char * crypt_pw = crypt(pw, salt);

    // Check that crypt returned something
    // Compare to hash from pw file
    if (crypt_pw == NULL)
    {
        printf("ERROR: could not encrypt password\n");
        return ERROR_VALUE;
    }
    else if (strcmp(crypt_pw, pw_ptr->pw_passwd) != SUCCESS_VALUE)
    {
        printf("ERROR: password does not match\n");
        return ERROR_VALUE;
    }

    // Check to see that pw entry is not expired
    if (pw_ptr->pw_expire > 0 && pw_ptr->pw_expire < time(NULL))
    {
        printf("ERROR: password entry has expired\n");
        return ERROR_VALUE;
    }
    
    // Zero out the password struct, even though the pw should be  
    // stored as a hash value, there's no point in letting it out
    // it could be dictionary-attacked!!!
    zero_str((char **)&pw_ptr, sizeof(pw_ptr));

    return SUCCESS_VALUE;
}

/**
 * Zero out the given string. This may protect sensitive data after it is no
 * longer needed in memory, such as passwords.
 * Parts of this code were taken from:
 * http://nob.cs.ucdavis.edu/clinic/situations/zerosensitivedata.html
 */
void zero_str(char ** str, const size_t len)
{
    int i;
    for(i = 0; i < len; ++i)
    {
        (*str)[i] = '\0';
    }
}