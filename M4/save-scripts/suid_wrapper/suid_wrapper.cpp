#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>

static int execute_command(const std::string &command, char * const *argv);
static int get_basename(const std::string &path, std::string &base);
static int get_executable_name(const std::string &path, std::string &executable);

/* c++ wrapper around basename */
int
get_basename(const std::string &inpath, std::string &base)
{
    int ret = 0;
    base.clear();

    do {
        if (inpath.empty()) {
            ret = EINVAL;
            break;
        }

        char *path = strdupa(inpath.c_str());
        if (path == NULL) {
            ret = EINVAL;
            break;
        }

        char *c = basename(path);
        if (c == NULL) {
            ret = errno;
            break;
        }
        base = c;
    } while (0);

    return ret;
}

/* 
 * Given name of this executable (e.g. 'X', '/a/b/X', './X', etc.),
 * return '/px/bin/<X>.py'
 */
int
get_executable_name(const std::string &path, std::string &out)
{
    int ret = 0;
    out.clear();

    do {
        std::string base;
        ret = get_basename(path, base);
        if (ret != 0) {
            break;
        }
        if (base.empty()) {
            ret = EINVAL;
            break;
        }

        out = "/px/bin/" + base + ".py";

    } while (0);

    return ret;
}

/* exec (no fork) command.  no return unless error. */
int
execute_command(const std::string &command, char * const *argv)
{
    if (command.empty()) {
        return EINVAL;
    }

    execv(command.c_str(), argv);
    return errno;
}

int main(int argc, char **argv)
{
    (void)argc;
    std::string executable;
    int ret = 0;
    uid_t uid = 0;

    do {
        ret = get_executable_name(argv[0], executable);
        if (ret != 0 || executable.empty()) {
            if (ret == 0) {
                ret = EINVAL;
            }
            break;
        }

        argv[0] = const_cast<char *>(executable.c_str());
        uid = getuid();
        setuid(0); // needed for passwd, otherwise passwd will error out with "only root blah blah blah"
        ret = execute_command(executable, argv);
        setuid(uid);

    } while (0);

    std::cerr << executable << " " << std::strerror(ret) <<  std::endl;

    return ret;
}
