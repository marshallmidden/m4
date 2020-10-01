/*
 *      Usage:
 *              root
 *                      spawn a root shell
 *              root cmd
 *                      run "cmd" as root
 *
 *      To install:
 *              chown root:root root
 *              chmod 6550 root
 *
 *      Then, add each user who should be allowed to run this to group
 *      "root".  Only root and group root have execute permission on this
 *      file.  Note that if a user is added to group "root", he will not
 *      get those group permissions until the next time he logs in.  If
 *      you can't wait, then do a 'newgrp root' to become a member right now.
 *
 *      If you don't want to bother with making a group, and don't mind
 *      leaving your system wide open, then just make this file mode 4755.
 *      This is, of course, a bad idea.  But you asked.
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>

/* ------------------------------------------------------------------------ */
static const struct nl {
  const char *str;
  const int   lth;
} nl[] = {
  {"CPU=", 4},
  {"PS1=", 4},
  {"TERM=", 5},
  {"HOST=", 5},
  {"USER=", 5},
  {"PATH=", 5},
  {"HOME=", 5},
  {"SHELL=", 6},
  {"OSTYPE=", 7},
  {"SSH_TTY=", 8},
  {"LOGNAME=", 8},
  {"HOSTNAME=", 9},
  {"HOSTTYPE=", 9},
  {"MACHTYPE=", 9},
  {"SSH_CLIENT=", 11},
  {"PROFILEREAD=", 12},
  {"SSH_CONNECTION=", 15},
  {"_=", 2}
};
 #define n_nl    18
// #define n_nl    16

static int keep_environment = 0;        /* Keep environment argument to root. */

/* ------------------------------------------------------------------------ */
struct delete_later {
  char *str;
  struct delete_later *next;
} *dl = NULL;

/* ------------------------------------------------------------------------ */
int main(int argc, char **argv, char **env)
{
    int             uid;
    int             gid;
    int             n;
    char           *name;
    char           *cp;
    struct passwd  *pw;
    char **e = env;
    struct delete_later *next_p = NULL;
    struct delete_later *current_p = NULL;

    /* Process parameters. */
    uid = -1;
    gid = -1;

    while (--argc >= 1)
    {
        argv++;

        /* Handle alternate user name. */
        if (argv[0][0] == '-' && argv[0][1] == 'u')
        {
            if (argv[0][2] == 0)
            {
                if (argc == 1)
                {
                    (void)fprintf(stderr, "-u requires username\n");
                    exit(2);
                }
                name = argv[1];
                argv++;
                argc--;
            }
            else
            {
                name = &argv[0][2];
            }
            if ((pw = getpwnam(name)) == 0)
            {
                (void)fprintf(stderr, "Unknown user: %s\n", name);
                exit(2);
            }
            uid = pw->pw_uid;
            gid = pw->pw_gid;
        }
        /* Keep environment variables */
        else if (argv[0][0] == '-' && argv[0][1] == 'k')
        {
            keep_environment = 1;
        }
        /* Change nice level. */
        else if ((argv[0][0] == '-' && (n = atoi(&argv[0][1]))) ||
                 (argv[0][0] == '+' && (n = -atoi(&argv[0][1]))))
        {
            errno = 0;
            n = nice(n);
            if (errno)
            {
                perror("Cannot nice");
                exit(2);
            }
        }
        else if (argv[0][0] == '-')
        {
            (void)fprintf(stderr, "Usage: root [-u name] [+/-pri] [cmd [args ...]]\n");
            exit(2);
        }
        else
        {
            break;
        }
    }

    /* Set user id. */
    if (uid == -1)
    {
        uid = geteuid();
    }
    if (gid == -1)
    {
        gid = getegid();
    }

    if (setuid(uid) == -1)
    {
        perror("Cannot change user id");
        exit(2);
    }

    if (setgid(gid) == -1)
    {
        perror("Cannot change group id");
        exit(2);
    }

    /*
     *      If no command is specified,
     *      fire up the selected shell.
     */

    if (argc == 0)
    {
        name = getenv("SHELL");
        if (name == 0)
        {
            name = "/bin/sh";
        }
        argv[0] = name;
        for (cp = name; *cp;)
        {
            if (*cp++ == '/')
            {
                argv[0] = cp;
            }
        }
    }

    /* If a command is given, use that. */
    else
    {
        name = argv[0];
    }

    e = env;
    while (*e != NULL)
    {
        int i;
        int j;
        size_t s;
        char *w;

        j = 0;
        for (i = 0; i < n_nl; i++)
        {
            if (keep_environment == 1 || strncmp(nl[i].str, e[0], nl[i].lth) == 0)
            {
                /* Do not touch certain env variables. */
//                fprintf(stderr, "keep this %s\n", e[0]);
                j = 1;
                break;
            }
        }
        if (j == 0)
        {
            w = strchr(e[0], '=');
            if (w != NULL)
            {
                s = w - e[0];
                w = malloc(s+1);
                memset(w, 0, s+1);
                strncpy(w, e[0], s);
//                fprintf(stderr, "toss this %s\n", e[0]);
                current_p = malloc(sizeof(*current_p));
                current_p->str = w;
                current_p->next = NULL;
                if (next_p == NULL)
                {
                    dl = current_p;
                }
                else
                {
                    next_p->next = current_p;
                }
                next_p = current_p;
            }
//            else
//            {
//                fprintf(stderr, "no equal %s\n", e[0]);
//            }
        }

        e++;
    }

    current_p = dl;
    while (current_p != NULL)
    {
//        fprintf(stderr, "unsetenv(%s)\n", current_p->str);
        unsetenv(current_p->str);
        free(current_p->str);
        next_p = current_p->next;
        free(current_p);
        current_p = next_p;
    }

//     putenv("USER=root");
//     putenv("LOGIN=root");
//     putenv("LOGNAME=root");
    setenv("USER", "root", 1);
    setenv("LOGIN", "root", 1);
    setenv("LOGNAME", "root", 1);

//    e = env;
//    while (*e != NULL)
//    {
//        fprintf(stderr, "have this %s\n", e[0]);
//        e++;
//    }
//    exit(1);

    /* Execute the selected command. */
    (void)execvp(name, argv);
//      (void) execve(name, argv, env) ;

    perror(argv[0]);
    return (2);
}

/*
 * vi: sw=4 ts=8 expandtab
 */
