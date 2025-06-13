#include "../../includes/webserv.hpp"

bool    checkFile(string filename, int perm)
{
    int fd = open(filename.c_str(), perm);
    if (fd == -1)
        return false;
    close (fd);
    return true;
}

bool    checkDir(string dirname, int dirStat)
{
    return access(dirname.c_str(), dirStat) == 0;
}
