#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#
# NOTDONEYET - --one-line   - job, project, ...
# NOTDONEYET - Fibre Channel "devices" -> protocolid -> systemid
# NOTDONEYET - iSCSI

# NOTDONEYET - storage files create NOTDONEYET      # Create a storage file -- hidden.
#            - storage files delete NOTDONEYET      # Verify deleting a hidden storage file.
# NOTDONEYET - jobs edit jobid {...} - not tested, cleaned up, etc.
# NOTDONEYET - fast3
# NOTDONEYET - schedules
'''
2020-04-01 Modified from something Joel had and something William provided ...

Stand-alone CLI for Parsec appliance - uses REST interface.
Note: Commands may be abbreviated to uniqueness. Example: "j l" for "job list".

Synopsis:
    exit                                # Exit script when in input mode.
    exit N                              # Exit script with value.
    help                                # Print this help.
    history                             # List previous successful commands.
    sleep                               # Sleep (ignored without value).
    sleep N                             # Sleep for number of seconds provided.
    brief                               # j and p have little output.
    brief on/off                        # Enable brief output, or back to full output.
    one-line                            # j and p have one line output.
    one-line on/off                     # Enable one line output, or back to full output.

    j                                   # List all jobs.
    j l                                 # List all jobs.
    job delete jobid ["name"] [id]      # Delete job(s) by ID or name.
    job list                            # List all jobs.
    job list jobid ["name"] [id]        # List job(s) by ID or name.
    jobs project ["name"] [id]          # List job(s) for one project ID or name.
    job run jobid ["name"] [id]         # Run job(s) by ID or name.
    job stop jobid ["name"] [id]        # Stop job(s) by ID or name.
    job verify jobid ["name"] [id]      # Start verify job(s) by ID or name.
    jobs                                # Easier to remember than "job". :)

    job create projid src_uri dest_uri "job name" src_vers dst_vers
                                        # project id number, or project name.
    job create "hi" src dst "job name"  # Create job attached to project "hi",
                                        #    moving src to dst, named "job name".
    job edit jobid {name=value}         # Useful for remote -- untested recently.
    p                                   # List all projects.
    proj                                # List all projects.
    proj list                           # List all projects.
    proj list projectid                 # List specific project by name or ID.
    proj dele "name of project" [id]    # Delete project by name or ID.
    proj create "name of project" src_vers dst_vers
                                        # Create project with provided name.
                                        #    with source SMB version, and destination.
    projects                            # Easier to remember than proj. :)

Examples:
    jobs
    job run 59
    sleep 240
    job dele 59
    project create "MySMB" "1.0" "2.0"  # source share uses SMB version 1.0
    project create "MySMB" "1.0" ""     # default dst to 3.1.1 - Linux default.
    p c "MySMB" "1.0" "default"         # If you want to see the argument default.
    job create 26 cifs://172.22.14.107/m4_v1 cifs://172.22.14.107/m4_v2 "mySMB"
    job c 26 nfs://172.22.14.107/vol/m4_v1 nfs://172.22.14.107/vol/m4_v2
    j c 26 nfs://172.22.14.54/vol/m4_v1 parsec://m4_139/ "myNFS"
    j c 26 parsec://m4_139/ nfs://172.22.14.54/vol/m4_v2 "myRemote"
    j c 27 block://25eb8ab90056482af6c9ce9009399e675 block://21ab345f355a8287b6c9ce9009399e675 job_name_FC
    job edit 57 edit dst.remoteverifier=...
    j e 58 edit src.remoteverifier=...
    j r 57                              # Run job 57.
    p l 19                              # See all jobs for project 19.

    ./m4.py --parsec IpAddr --user My --passwd Mine pro l "MySMB" 33 44
    export pxDEV=IpAddr pxUSER=My pxPASS=Mine ; ./m4.py jobs l 20 21 22 "myNFS"

Environment Variables:
    The following environment variables may be set to avoid using the --parsec,
    --user and --passwd command line options:
        pxDEV   hostname or IP address of Parsec appliance
        pxUSER  user name of the account you wish to authenticate to
        pxPASS  password for the account you wish to authenticate to
'''
#-----------------------------------------------------------------------------
import argparse
import json
import os
import pprint
import re
import readline
import requests
import rlcompleter
import shlex
import sys
import termios
import time
import yaml
# Use the tab key for completion.
if sys.platform == 'darwin':
    readline.parse_and_bind ("bind ^I rl_complete")
else:
    readline.parse_and_bind("tab: complete")
# fi
#-----------------------------------------------------------------------------
HIST_FILE = os.path.join(os.path.expanduser("~"), ".m4py_history")

PROJECT_JOBRATE = None
PROJECT_RUNLIMIT = None

# Following for CIFS volumes.
username='AD/Parsec.Backup'
password='Cobra!Indigo'
#=============================================================================
# For the completer - the list of words.
# Note: trailing spaces are words that take arguments, split_for_unique ignores them. 
# NOTE: lower case.
tab_words = { 
              'exit':      [],      # No arguments
              'quit':      [],      # No arguments
              'history':   [],      # No arguments
              'sleep':     [],      # A number of seconds to sleep.
              'example':   [],      # Example of how to do things.
              'brief':     ['on', 'enabled',
                            'off', 'disabled',
                            'help', '?'],
              'one-line':  ['on', 'enabled',
                            'off', 'disabled',
                            'help', '?'],
              'jobs':      ['list', 'create', 'delete', 'edit', 'run', 'start',
                            'stop', 'verify', 'help', '?'],
              'projects' : {
                            'list': ['?', 'help'],
                            'create': ['?', 'help'],
                            'delete': ['?', 'help'],
                            'help':'', '?':''
                           },
              'storage':   {
                            'list': ['devices', 'files', 'protocols', 'systems'],
                            'devices': ["list", "help", '?'],
                            'files': ["list", "help", '?', "create", "delete"],
                            'protocols': ["create", "delete", "list", "help", '?'],
                            'systems': ["create", "delete", "list", "help", '?'],
                            'help':'', '?':''
                           },
              'help':      ['example', 'projects', 'jobs', 'quit', 'exit', 'history', 'sleep',
                            'brief', 'one-line', 'storage'],
              '?':         ['example', 'projects', 'jobs', 'quit', 'exit', 'history', 'sleep',
                            'brief', 'one-line', 'storage']
            }
#-----------------------------------------------------------------------------
# Note: trailing spaces are words that take arguments, split_for_unique ignores them.
def split_for_unique(string):
    array = []
    for i in range(len(string)):
        first = string[0: i+1: 1]
        array.append(first)
    return array
# End of split_for_unique
#-----------------------------------------------------------------------------
# Get rid of entries that are the same.
def unique_dict_array(strings):
    d = { x : split_for_unique(x) for x in strings }
    k = [x for x in d.keys()]
    for i1 in range(len(k)):                # From first to last-1
        v1 = d[k[i1]]
        flag1 = []                          # Items to delete from v1.
        for i2 in range(i1+1, len(k)):      # From next to last
            for c in v1:
                v2 = d[k[i2]]
                for b in v2:
                    if c == b:
                        if c not in flag1:
                            flag1.append(c)
                        d[k[i2]].remove(b)
                    # fi
                # rof
            # rof
        # rof
        for r in flag1:
            d[k[i1]].remove(r)
        # rof
    # ror
    return d
# End of unique_dict_array
#-----------------------------------------------------------------------------
class dnsCompleter:
    def __init__(self, options):
        self.options = options
        self.current_candidates = []
        return
    # End of __init__

    def complete(self, text, state):
        # State 0 is the only one that we set up anything for.
        if state == 0:
            # This is the first time for this text, with "tab" pressed.
            origline = readline.get_line_buffer()   # Need to know about white-space on right.
            begin = readline.get_begidx()           # Start of word being completed in line.
            end = readline.get_endidx()             # Where tab was pressed in line.
            the_line_to_complete = origline[0:end]
            being_completed = origline[begin:end]   # Word being completed.
            the_words = the_line_to_complete.split()
            number_words = len(the_words)           # Subtract one if using as index. range() goes up to...
            if being_completed != '':
                number_words = number_words - 1

            # print("--origline='{}'".format(origline))
            # print("--begin={}".format(begin))
            # print("--end={}".format(end))
            # print("--the_line_to_complete='{}'".format(the_line_to_complete))
            # print("--being_completed='{}'".format(being_completed))
            # print("--the_words='{}'".format(the_words))
            # print("--number_words={}".format(number_words))

            # Possibilities -
            #   one tab auto complete for word starting with...
            #   two tabs display possibilities for words starting with -- or all words if nothing.
            #
            # Need to determine number of words.
            # Need to determine if start of new word (true), false if in middle of word.
            # If --being_completed == '', set up for two tabs with all possible here.
            # If --being_completed == 'hi', setup for two tabs with all possible 'hi's AND one tab completes.

            # See if first words exist and are right.
            c = ''
            what = unique_dict_array(self.options)
            full = self.options
            for j in range(number_words):
                unique = False
                c = ''
                for c in what:
                    if the_words[j].lower() in what[c]:
                        unique = True
                        break;
                if not unique:
                    return None
                full = full[c]
                what = unique_dict_array(full)

            if being_completed == '':                   # If no starting word, all are possible.
                self.current_candidates = sorted(what.keys())
            else:
                self.current_candidates = sorted(what.keys())
                try:
                    candidates = self.current_candidates
                    if being_completed:
                        # Match with portion of input already typed.
                        self.current_candidates = [ w for w in candidates if w.startswith(being_completed.lower()) ]
                        # Add space after it if only one choice.
                        if len(self.current_candidates) == 1:
                            self.current_candidates[0] = self.current_candidates[0] + ' '
                    # fi    being_completed
                except (KeyError, IndexError) as err:
                    self.current_candidates = []
                # yrt
            # fi    not the_words
        # fi   state == 0
        try:
            response = self.current_candidates[state]
        except IndexError:
            response = None
        # yrt
        return response
    # End of complete
# End of class dnsCompleter
#=============================================================================
def parse_args():
    global args

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='this is the epilog')
    parser.add_argument('--parsec', '-P', type=str, default=os.environ.get('pxDEV',None),
                        help='hostname of Parsec appliance')
    parser.add_argument('--user', '-u', type=str, default=os.environ.get('pxUSER', None),
                        help='Parsec user name')
    parser.add_argument('--passwd', '-p', type=str, default=os.environ.get('pxPASS', None),
                        help='Parsec password')
    parser.add_argument('--version', default='/api/v3',
                        help='The REST api version - default "/api/v3".')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help = 'Verbose mode output')
    parser.add_argument('--brief', '-b', '-br', action='store_true',
                        help = 'Very brief output for job/project list, etc.')
    parser.add_argument('--one-line', '--one', '-o', '-one', '-one-line', action='store_true',
                        help = 'One line output for: job/project list, etc.')
    parser.add_argument('rest', nargs='*',
                        help='Optional command to execute')
    args = parser.parse_args()

    if not args.parsec or not args.user or not args.passwd:
        print('Need --parsec and --user and --passwd')
        exit(1)
    # fi
    return
# End of parse_args
#=============================================================================
_httpResponseMap = {
    100 : 'Continue',
    101 : 'Switching Protocols',
    103 : 'Early Hints',
    200 : 'OK',
    201 : 'Created',
    202 : 'Accepted',
    203 : 'Non-Authoritative Information',
    204 : 'No Content',
    205 : 'Reset Content',
    206 : 'Partial Content',
    300 : 'Multiple Choices',
    301 : 'Moved Permanently',
    302 : 'Found',
    303 : 'See Other',
    304 : 'Not Modified',
    307 : 'Temporary Redirect',
    308 : 'Permanent Redirect',
    400 : 'Bad Request',
    401 : 'Unauthorized',
    402 : 'Payment Required',
    403 : 'Forbidden',
    404 : 'Not Found',
    405 : 'Method Not Allowed',
    406 : 'Not Acceptable',
    407 : 'Proxy Authentication Required',
    408 : 'Request Timeout',
    409 : 'Conflict',
    410 : 'Gone',
    411 : 'Length Required',
    412 : 'Precondition Failed',
    413 : 'Payload Too Large',
    414 : 'URI Too Long',
    415 : 'Unsupported Media Type',
    416 : 'Range Not Satisfiable',
    417 : 'Expectation Failed',
    418 : "I'm a teapot",
    422 : 'Unprocessable Entity',
    425 : 'Too Early',
    426 : 'Upgrade Required',
    428 : 'Precondition Required',
    429 : 'Too Many Requests',
    431 : 'Request Header Fields Too Large',
    451 : 'Unavailable For Legal Reasons',
    500 : 'Internal Server Error',
    501 : 'Not Implemented',
    502 : 'Bad Gateway',
    503 : 'Service Unavailable',
    504 : 'Gateway Timeout',
    505 : 'HTTP Version Not Supported',
    506 : 'Variant Also Negotiates',
    507 : 'Insufficient Storage',
    508 : 'Loop Detected',
    511 : 'Network Authentication Required',
}
# End of _httpResponseMap
#-----------------------------------------------------------------------------
def _httpMap(code):
    if isinstance(code, str):
        code = int(code)
    # fi
    return _httpResponseMap.get(code, 'UNKNOWN')
# End of _httpMap
#-----------------------------------------------------------------------------
# Print error from 'get/post' request and return.
def print_get_error(str, response):
    print(str)
    print(json.dumps(response.json(), indent=4))
    print('Exiting with status 1')
    return
# End of print_get_error
#-----------------------------------------------------------------------------
# Print error from URL 'get/post' request, and exit.
def print_get_error_exit(str, response):
    print_get_error(str, response)
    exit(1)
# End of print_get_error_exit
#-----------------------------------------------------------------------------
# Called initially.
def Login(ipaddr, version, user, pw):
    # Get Authentication Authorization Bearer.
    BASEURL = 'https://{}{}/'.format(ipaddr, version)
    url = BASEURL + 'auth'
    r = requests.get(url, auth=(user, pw), verify=False)
    if r.status_code != 200:
        print_get_error_exit('Could not login to device! Return response:', r)
    # fi
    return({'Authorization':'Bearer {}'.format(r.text)}, BASEURL)
# End of Login
#-----------------------------------------------------------------------------
def send_get(authentication, base_url, tackon):
    r = requests.get(base_url + tackon, headers=authentication, verify=False)
    if r.status_code != 200:
        print_get_error("Request for {} failed. Response:".format(tackon), r)
        return (False, None)
    # fi
    return(True, r)
# End of send_get
#-----------------------------------------------------------------------------
def send_post(base_url, authentication, str, goodvalue):
    global pp
    r = requests.post(base_url + str, headers=authentication, verify=False)
    if r.status_code != goodvalue:
        print('%3d : %s' % (r.status_code, _httpMap(r.status_code)))
        results = r.json()
        if results and 'detail' in results:
            print(results['detail'])
        # fi
        pp.pprint(results)
        return (False, r)
    # fi
    return (True, r)
# End of send_post
#-----------------------------------------------------------------------------
def send_post_json(base_url, authentication, str, info, goodvalue):
    global pp
    r = requests.post(base_url + str, headers=authentication, verify=False, json=info)
    if r.status_code != goodvalue:
        print('%3d : %s' % (r.status_code, _httpMap(r.status_code)))
        results = r.json()
        if results and 'detail' in results:
            print(results['detail'])
        # fi
        pp.pprint(results)
        return (False, r)
    # fi
    return (True, r)
# End of send_post_json
#-----------------------------------------------------------------------------
def send_delete(base_url, authentication, str):
    global pp
    r = requests.delete(base_url + str, headers=authentication, verify=False)
    if r.status_code != 204:
        print('%3d : %s' % (r.status_code, _httpMap(r.status_code)))
        results = r.json()
        if results and 'detail' in results:
            print(results['detail'])
        # fi
        pp.pprint(results)
        return (False, r)
    # fi
    return (True, r)
# End of send_delete
#=============================================================================
def print_project_output(proj, full):
    global args

    if args.brief:
        print("{}".format(proj['id']))
    elif args.one_line:
        print("{} '{}' '{}'".format(proj['id'], proj['message'], proj['name']))
    elif not full:
        print("id:{} message:'{}' name:'{}'".format( proj['id'], proj['message'], proj['name']))
    else:
        print('Project "{}":'.format(proj['id']))
        print(yaml.dump(proj, default_flow_style=False))
    # fi
# End of print_project_output
#-----------------------------------------------------------------------------
# Called when program starts. This sets projMap dictionary if display is True.
def ProjList(authentication, base_url, vargs, display):
    global projMap
    global args

    if not vargs:
        # If no argument, print out all project ID's and their names.
        (ret, projlist) = send_get(authentication, base_url, 'datamovement/projects')
        if not ret:
            return False
        # fi
        results = projlist.json()
        if results['projects'] == []:
            if not args.brief and not args.brief and display:
                print('No projects present.')
            # fi
            return True
        # fi
        if not args.brief and not args.brief and display:
            print('Projects:')
        # fi
        for proj in results['projects']:
            projMap[proj['id']] = proj['name']
            if display:
                print_project_output(proj, False)
            # fi
        # rof
        return True
    # fi

    # Possible multiple arguments with names/numbers.
    ret = False
    for id in vargs:
        if not id:
            continue
        # fi
        if not id.isdigit():
            (rt, projlist) = send_get(authentication, base_url, 'datamovement/projects')
            if not rt:
                continue
            # fi
            found = False
            for project in projlist.json()['projects']:
                if project['name'] == id:
                    print_project_output(project, True)
                    found = True
                    ret = True
                # fi
            # rof
            if not found and not args.brief and not args.one_line:
                print('Could not get information on project name "{}"'.format(id))
            # fi
            continue
        # fi

        # A project id number if here.
        (rt, r) = send_get(authentication, base_url, 'datamovement/projects/{}'.format(id))
        if not rt:
            continue
        # fi
        results = r.json()
        print_project_output(results, True)
        ret = True
    # rof
    return ret
# End of ProjList
#=============================================================================
def print_help_exit():
    print("Exit script when in commannd line input mode.")
    print("  Examples:")
    print("    exit                                 # Exit script with value 0")
    print("    exit N                               # Exit script with value N")
    return
# End of print_help_exit
#-----------------------------------------------------------------------------
def print_help_history():
    print("List history of previous successful commands.")
    print("  Example:")
    print("    history")
    print("      1: projects")
    print("      2: jobs")
    print("      3: storage files")
    print("      4: storage devices")
    print("      5: storage protocols")
    print("      6: storage systems")
    return
# End of print_help_history
#-----------------------------------------------------------------------------
def print_help_sleep():
    print("Sleep for a bit, then continue executing.")
    print("  Examples:")
    print("    sleep                                # Sleep (ignored without value).")
    print("    sleep N                              # Sleep for number of seconds provided.")
    return
# End of print_help_sleep
#-----------------------------------------------------------------------------
def print_help_brief():
    print("Enable brief(er) output, or back to full output.")
    print("  Examples:")
    print("    brief on                             # jobs and projects have little output.")
    print("    brief off                            # jobs and projects have full output.")
    print("    brief enabled                        # Same as on.")
    print("    brief disabled                       # Same as off.")
    print("    brief                                # Default to little output (on/enabled).")
    return
# End of print_help_brief
#-----------------------------------------------------------------------------
def print_help_one_line():
    print("Enable one line output, or back to full output.")
    print("  Examples:")
    print("    one-line on                          # jobs and projects have one line output.")
    print("    one-line off                         # jobs and projects have full output.")
    print("    one-line enabled                     # Same as on.")
    print("    one-line disabled                    # Same as off.")
    print("    one-line                             # Default to one line output (on/enabled).")
    return
# End of print_help_one_line
#-----------------------------------------------------------------------------
def print_help_jobs_list():
    print("    jobs list                            # List all jobs.")
    print("    jobs list [ID/name...]               # List specific job(s) by ID or name.")
# End of print_help_jobs_list
#-----------------------------------------------------------------------------
def print_help_jobs_create():
    print("    jobs create p_name src dst job_name  # Create job named 'job_name', attached to project")
    print("                                         #   named 'p_name', moving src URI to dst URI.")
    print("    j c 26 nfs://10.0.0.7/vol/v1 nfs://10.0.0.9/vol/v2 'v1->v2'")
    print("    job cr 26 cifs://10.7.7.7/V1 cifs://10.8.8.8/V2 'mySMB'")
    return
# End of print_help_jobs_create
#-----------------------------------------------------------------------------
def print_help_jobs_delete():
    print("    jobs delete ID/name [ID/name...]     # Delete job(s) by ID or name.")
    print("    jobs delete 44 mySMB                 # Delete job #44 and one named mySMB.")
    return
# End of print_help_jobs_delete
#-----------------------------------------------------------------------------
def print_help_jobs_start():
    print("    jobs start ID/name [ID/name...]      # Same as 'run'")
    print("    jobs run ID/name [ID/name...]        # Run job(s) by ID or name.")
    return
# End of print_help_jobs_start
#-----------------------------------------------------------------------------
def print_help_jobs_stop():
    print("    jobs stop ID/name [ID/name...]       # Stop job(s) by ID or name.")
    return
# End of print_help_jobs_stop
#-----------------------------------------------------------------------------
def print_help_jobs_edit():
    print("    jobs edit ID/name {name=value}       # Useful for remote -- untested recently.")
    print("    jobs edit 57 dst.remoteverifier=...")
    print("    j e 58 src.remoteverifier=...")
    return
# End of print_help_jobs_edit
#-----------------------------------------------------------------------------
def print_help_jobs_verify():
    print("    job verify ID/name [ID/name...]      # Start verify of job(s) by ID or name.")
    return
# End of print_help_jobs_verify
#-----------------------------------------------------------------------------
def print_help_jobs_all():
    print("List, create, delete, start, stop, edit, or verify a job.")
    print("  Examples:")
    print("    jobs                                 # Default to 'jobs list' (all jobs stuff).")
    print("    j                                    # Abbreviation and default to 'list'.")
    print_help_jobs_list()
    print_help_jobs_create()
    print_help_jobs_delete()
    print_help_jobs_start()
    print_help_jobs_stop()
    print_help_jobs_edit()
    print_help_jobs_verify()
    return
# End of print_help_jobs_all
#-----------------------------------------------------------------------------
def print_help_jobs(vargs):
    first_list = unique_dict_array(tab_words["jobs"])
    if vargs is None or vargs[0] == '':
        return print_help_jobs_all()
    subtype = vargs[0]
    if subtype in first_list['list']:
        return print_help_jobs_list()
    if subtype in first_list['create']:
        return print_help_jobs_create()
    if subtype in first_list['delete']:
        return print_help_jobs_delete()
    if subtype in first_list['start'] or subtype in first_list['run']:
        return print_help_jobs_start()
    if subtype in first_list['stop']:
        return print_help_jobs_stop()
    if subtype in first_list['edit']:
        return print_help_jobs_edit()
    if subtype in first_list['verify']:
        return print_help_jobs_verify()
    print("No help for jobs '{}'.".format(subtype))
    return print_help_jobs_all()
# End of print_help_jobs
#-----------------------------------------------------------------------------
def print_help_projects_list():
    print("    projects list                        # List all projects.")
    print("    projects list ID/name [...]          # List projects by ID or name.")
    print("    p l 19                               # List all jobs with project 19.")
    return
# End of print_help_projects_list
#-----------------------------------------------------------------------------
def print_help_projects_delete():
    print("    projects delete ID/name [ID/name...] # Delete project(s) by ID or name.")
    return
# End of print_help_projects_delete
#-----------------------------------------------------------------------------
def print_help_projects_create():
    print("    p c pname src_vers dst_vers          # Create project named 'pname', with source SMB version")
    print("    pr create 'MySMB' '1.0' '2.0'        # source share uses SMB version 1.0, dst=2.0.")
    print("    proj create 'MySMB' '1.0' ''         # default dst to 3.1.1 - Linux default.")
    print("    p c 'MySMB' '1.0' 'default'          # If you wish to see the argument default.")
    print("    proj create 'name of project' src_vers dst_vers")
    print("                                         # Create project with provided name.")
    print("                                         #    with source SMB version, and destination.")
    print("    project create 'MySMB' '1.0' '2.0'   # source share uses SMB version 1.0")
    print("    project create 'MySMB' '1.0' ''      # default dst to 3.1.1 - Linux default.")
    print("    p c 'MySMB' '1.0' 'default'          # If you want to see the argument default (above).")
    print("    projects create 44 NFS")             # No need for SMB version arguments.
    print("    projects create 45 SCSI")            # No need for SMB version arguments.
    return
# End of print_help_projects_create
#-----------------------------------------------------------------------------
def print_help_projects_all():
    print("List/create/delete project:")
    print("  Examples:")
    print("    projects                             # Default to 'projects list' (all project stuff).")
    print_help_projects_list()
    print_help_projects_delete()
    print_help_projects_create()
    return
# End of print_help_projects_all
#-----------------------------------------------------------------------------
def print_help_projects(vargs):
    first_list = unique_dict_array(tab_words["projects"])
    if vargs is None or vargs[0] == '':
        return print_help_projects_all()
    subtype = vargs[0]
    if subtype in first_list['list']:
        return print_help_projects_list()
    if subtype in first_list['delete']:
        return print_help_projects_delete()
    if subtype in first_list['create']:
        return print_help_projects_create()
    print("No help for project '{}'.".format(subtype))
    return print_help_projects_all()
# End of print_help_projects
#-----------------------------------------------------------------------------
def print_help_storage_list():
    print("    storage list                         # List all storage devices, files, protocols, systems.")
    print("    storage list devices                 # List all storage devices (storage/assets/devices).")
    print("    storage list files                   # List all storage files (storage/assets/files).")
    print("    storage list protocols               # List all storage protocols (storage/protocols).")
    print("    storage list systems                 # List all storage systems (storage/systems).")
    return
# End of print_help_storage_list
#-----------------------------------------------------------------------------
def print_help_storage_devices():
    print("    storage devices                      # List all storage devices (storage list devices).")
    print("    storage devices list                 # List all storage devices (storage list devices).")
    return
# End of print_help_storage_devices
#-----------------------------------------------------------------------------
def print_help_storage_files():
    print("    storage files                        # List all storage files (storage list files).")
    print("    storage files list                   # List all storage files (storage list files).")
    print("    storage files create NOTDONEYET      # Create a storage file -- hidden.")
    print("    storage files delete ID/name...      # Delete a storage file.")
    return
# End of print_help_storage_files
#-----------------------------------------------------------------------------
def print_help_storage_protocols():
    print("    storage protocols                    # List all storage protocols (storage list protocols).")
    print("    storage protocols list               # List all storage protocols (storage list protocols).")
    print("    storage protocols create SystemStorageID SMB IP 'UserName' 'Password' 'NameForProtocolid'")
    print("    storage protocols create 43 SMB 172.22.14.116 'AD/LoginName' 'BlueSnake' 'SomethingSMB'")
    print("    storage protocols create 44 NFS 172.22.14.103 'SomethingNFS'")
    print("    storage protocols delete ID/name...  # Delete a protocolid for files/devices.")
    return
# End of print_help_storage_files
#-----------------------------------------------------------------------------
def print_help_storage_systems():
    print("    storage systems                      # List all storage systems (storage list systems).")
    print("    storage systems list                 # List all storage system (storage list system).")
    print("    storage systems create ID/name...    # Create a system for files/devices.")
    print("    storage systems delete ID/name...    # Delete a system for files/devices.")
    return
# End of print_help_storage_systems
#-----------------------------------------------------------------------------
def print_help_storage_all():
    print("List/create/delete storage: devices, files, protocols, or systems.")
    print("  Examples:")
    print("    storage                              # Default to 'storage list' (all storage stuff).")
    print_help_storage_list()
    print_help_storage_devices()
    print_help_storage_files()
    print_help_storage_protocols()
    print_help_storage_systems()
    return
# End of print_help_storage_all
#-----------------------------------------------------------------------------
def print_help_storage(vargs):
    first_list = unique_dict_array(tab_words["storage"])
    if vargs is None or vargs[0] == '':
        return print_help_storage_all()
    subtype = vargs[0]
    if subtype in first_list['list']:
        return print_help_storage_list()
    if subtype in first_list['devices']:
        return print_help_storage_devices()
    if subtype in first_list['files']:
        return print_help_storage_files()
    if subtype in first_list['protocols']:
        return print_help_storage_protocols()
    if subtype in first_list['systems']:
        return print_help_storage_systems()
    print("No help for storage '{}'.".format(subtype))
    return print_help_storage_all()
# End of print_help_storage
#-----------------------------------------------------------------------------
def print_help_example():
    print('Example to do an SMB migration from "v1" to "v2":')
    print('  # Get all jobs, stop them, then delete them.')
    print('      J=`./m4.py --brief jobs list`')
    print('      ./m4.py jobs stop $J')
    print('      ./m4.py jobs delete $J')
    print('  # Get all projects, then delete them.')
    print('      P=`./m4.py --brief projects list`')
    print('      ./m4.py proj delete ${P}')
    print('  # Get all storage files, then delete them.')
    print("      SF=`./m4.py --brief storage files | awk '{print $1}'`")
    print('      ./m4.py storage files delete ${SF}')
    print('  # Get all storage protocols, then delete them.')
    print("      SP=`./m4.py --brief storage protocols | awk '{print $1}'`")
    print('      ./m4.py storage protocols delete ${SP}')
    print('  # Get all storage systems, then delete them.')
    print("      SS=`./m4.py --brief storage systems | awk '{print $1}'`")
    print('      ./m4.py storage systems delete ${SS}')

    print('  # Add Storage Systems')
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    print("      SSSMB=`./m4.py --brief storage systems create SMB_stuff | awk '{print $1}'")
    print("  #      Storage System Created 50 - 'SMB_stuff'         # if not --brief")
    print("      SPSMB=`./m4.py --brief storage protocols create ${SSSMB} SMB 172.22.14.116 'AD/LoginName' 'BlueSnake' 'SomethingSMB' | awk '{print $1}'`")
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    print("      SSNFS=`./m4.py --brief storage systems create NFS_stuff | awk '{print $1}'`")
    print("  #      Storage System Created 51 - 'NFS_stuff'         # if not --brief")
    print("      SPNFS=`./m4.py --brief storage protocols create ${SSNFS} NFS 172.22.13.103 SomethingNFS | awk '{print $1}'`")
    print("  #      Storage Protocol Created 23 - 'SomethingNFS'    # if not --brief")
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    print("      SSSCSI=`./m4.py --brief storage systems create SCSI_stuff | awk '{print $1}'`")
    print("  #      Storage System Created 52 - 'SCSI_stuff'        # if not --brief")
    print("      SPSCSI=`./m4.py --brief storage protocols create ${SSCSI} SCSI SomethingSCSI | awk '{print $1}'")
    print("  #      Storage System Created 24 - 'SomethingSCSI'     # if not --brief")
    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    print('  # Create projects and jobs.')
    print('      PNSMB="Scripted SMB project for na116 v1 to na116 v2"')
    print('      JNSMB="job to copy na116 v1 to na116 v1"')
    print('      PNNFS="Scripted NFS project for 172.22.13.103 v1 to 172.22.13.103 v2"')
    print('      JNNFS="job to copy 172.22.13.103 v1 to 172.22.13.103 v1"')
    print('      PNSCSI="Scripted SCSI project for 10g to 10g"')
    print('      JNSCSI="job to copy FC 10g to FC 10g"')
    print('  # Create the project - note SMB version 2.0 source and destination.')
    print('     ./m4.py p c "${PNSMB}" 2.0 2.0')
    print('  #     Project Created 512')
    print('     ./m4.py p c "${PNNFS}"')
    print('  #     Project Created 513')
    print('     ./m4.py p c "${PNSCSI}"')
    print('  #     Project Created 513')
    print('  # Create a job for doing the migration.')
    print('     ./m4.py jobs create "${PNSMB}" cifs://172.22.14.116/v1 cifs://172.22.14.116/v2 "${JNSMB}"')
    print('  #     Job ID: 596')
    print('     ./m4.py jobs create "${PNNFS}" nfs://172.22.14.54/v1 nfs://172.22.14.54/v2 "${JNNFS}"')
    print('  #     Job ID: 597')
    print('     ./m4.py jobs create "${PNSCSI}" block://25eb8ab90056482af6c9ce9009399e675 block://21ab345f355a8287b6c9ce9009399e675 "${JNSCSI}"')
    print('  #     Job ID: 598')
    print('  # Run the jobs for doing the migration.')
    print('     ./m4.py jobs run "${JNSMB}"')
    print('     ./m4.py jobs run "${JNNFS}"')
    print('     ./m4.py jobs run "${JNSCSI}"')
    return
# End of print_help_example
#-----------------------------------------------------------------------------
def print_all_help():
    print("")
    print("Stand-alone CLI for Parsec appliance - uses REST interface.")
    print("Note: Commands may be abbreviated to uniqueness. Example: 'j l' for 'job list'.")
    print("")
    print("Synopsis:")
    print_help_exit()
    print_help_history()
    print_help_sleep()
    print_help_brief()
    print_help_one_line()
    print_help_jobs_all()
    print_help_projects_all()
    print_help_storage_all()
    print_help_example()
    print("")
    print("Needed arguments/Environment variables:")
    print("    ./m4.py --parsec IpAddr --user My --passwd Mine pro l 'MySMB' 33 44")
    print("    export pxDEV=IpAddr pxUSER=My pxPASS=Mine ; ./m4.py jobs l 20 21 22 'myNFS'")
    print("Environment Variables:")
    print("  The following environment variables may be set to avoid using the --parsec,")
    print("  --user and --passwd command line options:")
    print("    pxDEV      hostname or IP address of Parsec appliance")
    print("    pxUSER     user name of the account you wish to authenticate to")
    print("    pxPASS     password for the account you wish to authenticate to")
    return
# End of print_all_help
#-----------------------------------------------------------------------------
def Help(subtype, first_list, vargs):
    commands = tab_words
    print("Commands possible:", ', '.join(sorted(commands)))
    if (first_list is None or subtype is None or subtype == '' or
        subtype[0] == '?' or subtype in first_list['help']):
        return print_all_help()
    if subtype in first_list['exit'] or subtype in first_list['quit']:
        return print_help_exit()
    if subtype in first_list['history']:
        return print_help_history()
    if subtype in first_list['sleep']:
        return print_help_sleep()
    if subtype in first_list['brief']:
        return print_help_brief()
    if subtype in first_list['one-line']:
        return print_help_one_line()
    if subtype in first_list['jobs']:
        return print_help_jobs(vargs)
    if subtype in first_list['projects']:
        return print_help_projects(vargs)
    if subtype in first_list['example']:
        return print_help_example()
    if subtype in first_list['storage']:
        return print_help_storage(vargs)
    print("No help for command '{}'".format(subtype))
    # return print_all_help()
    return
# End of Help
#=============================================================================
def Exit(subtype):
    if subtype is None or subtype == '':
        exit(0)
    # fi
    if not subtype.isdigit():
        print("Error: argument is not a number '{}'".format(subtype))
        return False
    # fi
    exit(int(subtype))
# End of Exit
#=============================================================================
def Sleep(subtype):
    if subtype is not None and subtype != '':
        if not subtype.isdigit():
            print("Error: argument is not a number '{}'".format(subtype))
            return False
        # fi
        time.sleep(int(subtype))
    # fi
    return True
# End of Sleep
#=============================================================================
def Brief(subtype):
    global args

    if subtype is None or subtype == '' or subtype == 'on' or subtype in split_for_unique('enabled'):
        args.brief = True
        print('Brief mode enabled')
        return True
    # fi
    if subtype == 'of' or subtype == 'off' or subtype in split_for_unique('disabled'):
        print('Brief mode disabled (i.e. full print mode)')
        args.brief = False
        return True
    # fi

    print("Error: Brief argument '{}' not recognized as on/off/enable/disable".format(subtype))
    if args.brief is None or args.brief:
        print("Error: Brief left as 'on'")
    else:
        print("Error: Brief left as 'off'")
    # fi
    return False
# End of Brief
#=============================================================================
def One_Line(subtype):
    global args

    if subtype is None or subtype == '' or subtype == 'on' or subtype in split_for_unique('enabled'):
        args.one_line = True
        print('one-line mode enabled')
        return True
    # fi
    if subtype == 'of' or subtype == 'off' or subtype in split_for_unique('disabled'):
        print('one-line mode disabled (i.e. full print mode)')
        args.one_line = False
        return True
    # fi

    print("Error: one-line argument '{}' not recognized as on/off/enabled/disabled'".format(subtype))
    if args.one_line is None or args.one_line:
        print("Error: one-line left as 'on'")
    else:
        print("Error: one-line left as 'off'")
    # fi
    return False
# End of One_Line
#=============================================================================
def History():
    global history

    cnt = 1
    for cmd in history:
        print(' %4d: %s' % (cnt, cmd))
        cnt += 1
    # rof
    return True
# End of History
#=============================================================================
# Called from JobCreate.
def _parseJobURI(uri):
    global username, password
    # NFS
    m = re.match('^nfs://([^/]+)/(.*)$', uri)
    if m:
        return (True, {'type':'NFS', 'host':m.group(1), 'export':'/'+m.group(2)})
    # fi

    # SMB/CIFS
    m = re.match('^cifs://([^/]+)/(.*)$', uri)
    if not m:
        m = re.match('^smb://([^/]+)/(.*)$', uri)
    # fi
    if m:
        return (True, {'type':'SMB', 'host':m.group(1), 'share':m.group(2),
                 'username':username, 'password':password})
    # fi

    # BLOCK - FC or iSCSI
    m = re.match('^block://(\w+)/*$', uri)
    if m:
        return (True, {'type':'BLOCK', 'serialnumber':m.group(1)})
    # fi

    # Replication
    m = re.match('^parsec://([^/]+)/*$', uri)
    if m:
        return (True, {'type':'REPLICATION', 'host':m.group(1), 'remoteverifier':None})
    # fi

    print("Unknown URI '{}'".format(uri))
    return (False,dict())
# End of _parseJobURI
#-----------------------------------------------------------------------------
def JobCreate(authentication, base_url, vargs):
    global args
    global _jobId

    if not vargs or not vargs[0]:
        print("Must have 4 arguments to create:")
        print("   job create projectId, source, destination, JobName")
        return False
    # fi

    if len(vargs) != 4:
        print("Must have 4 arguments to create (not {}):".format(len(vargs)))
        print("   job create projectId, source, destination, JobName")
        return False
    # fi

    (rt, s) = _parseJobURI(vargs[1])
    if not rt:
        print("Second argument to job create must be URI for source. nfs://127.0.0.1/vol")
        return False
    # fi
    (rt, d) = _parseJobURI(vargs[2])
    if not rt:
        print("Third argument to job create must be URI for Destination. smb://127.0.0.1/share")
        return False
    # fi

    id = vargs[0]
    if not id.isdigit():
        # A project name if here.
        (ret, projlist) = send_get(authentication, base_url, 'datamovement/projects')
        if not ret:
            print("Error: First argument to job create is not a number or project name '{}'".format(id))
            return False
        # fi
        found = False
        for project in projlist.json()['projects']:
            if project['name'] == id:
                id = project['id']
                found = True
                break
            # fi
        # rof
        if not found:
            print("Could not find project name {}".format(id))
            return False
        # fi
    # fi

    # A project id number if here.
    id = int(id)
    info = {
        'encrypt':False,
        'name':vargs[3],
        'projectid':id,
        'source':s,
        'destination':d
    }

    _jobId += 1
    (rt, r) = send_post_json(base_url, authentication, 'datamovement/jobs', info, 200)
    if not rt:
        return False
    # fi

    results = r.json()
    if results and 'id' in results:
        if args.brief or args.one_line:
            print('{}'.format(results['id']))
        else:
            print('Job ID: {}'.format(results['id']))
        # fi
    # fi
    if results and 'detail' in results:
        print(results['detail'])
    # fi
    return True
# End of JobCreate
#=============================================================================
# Called from _printJob, which is called from JobList.
def _jobURL(job):
    if job['type'] == 'NFS':
        return 'nfs://%(host)s/%(export)s' % job
    elif job['type'] == 'BLOCK':
        return 'block://%(serialnumber)s' % job
    elif job['type'] == 'REPLICATION':
        return 'parsec://%(host)s' % job
    elif job['type'] == 'SMB':
        return 'cifs://%(host)s/%(share)s' % job
    # fi
    return '%(type)s://???' % job
# End of _jobURL
#-----------------------------------------------------------------------------
# Called from JobList.
def _printJob(job):
    global projMap

    job['proj'] = projMap[job['projectid']]
    print('%(id)4d: Status %(status)-8s   projectid %(projectid)s    project name "%(proj)s"' % job)
    mess = ''
    if 'message' in job:
        if job['message']:
            mess = job['message']
        # fi
    # fi
    print('      state: %(state)s    name: "%(name)s"' % job)
    print('      src: "{}"'.format(_jobURL(job['source'])))
    print('      dst: "{}"'.format(_jobURL(job['destination'])))
    print("      message: '{}'".format(mess))
    return
# End of _printJob
#=============================================================================
def print_job_output(job):
    global args

    if args.brief:
        print('%(id)d' % job)
    elif args.one_line:
        mess = ''
        if 'message' in job:
            if job['message']:
                mess = job['message']
            # fi
        # fi
        print("{} {} '{}' '{}' '{}' '{}' '{}' '{}' {} {} '{}' '{}'".format(
               job['id'], job['projectid'], job['state'], job['status'],
              _jobURL(job['source']), _jobURL(job['destination']), job['name'], mess,
              job['warnings'], job['errors'], job['start'], job['end']))
    else:
        _printJob(job)
    # fi
# End of print_job_output
#-----------------------------------------------------------------------------
def JobList(authentication, base_url, vargs):
    global args

    if not vargs:
        # If no argument, print out all job ID's and their names.
        (rt, r) = send_get(authentication, base_url, 'datamovement/jobs')
        if not rt:
            return False
        # fi
        results = r.json()

        if not args.brief and not args.one_line  and results['jobs'] == []:
            print('No jobs present.')
            return True
        # fi
        if not args.brief and not args.one_line:
            print('Jobs:')
        for job in results['jobs']:
            print_job_output(job)
        # rof
        return True
    # fi

    ret = False
    for jid in vargs:
        if not jid.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            # fi
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == jid:
                    print_job_output(job)
                    found = True
                # fi
            # rof
            if not found:
                print('Could not get information on job name "{}"'.format(jid))
            # fi
            continue
        # fi
        # Must be a number if here
        (rt, r) = send_get(authentication, base_url, 'datamovement/jobs/{}'.format(jid))
        if not rt:
            continue
        # fi
        results = r.json()
        if results['status'] == 404:
            print('No job {0} present.'.format(jid))
        else:
            ret = True
            if not args.brief and not args.one_line:
                print('Job {}:'.format(jid))
            print_job_output(results)
        # fi
    # rof
    return ret
# End of JobList
#-----------------------------------------------------------------------------
def JobDele(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0]:
        print("Error - no jobs given to delete")
        return False
    # fi
    ret = False
    for id in vargs:
        if not id.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            # fi
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == id:
                    id =  job['id']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print('Could not get information on job name "{}"'.format(id))
                continue
            # fi
        # fi
        # If name, "id" changed.
        (rt, r) = send_delete(base_url, authentication, 'datamovement/jobs/{}'.format(id))
        if not rt:
            continue
        # fi
        if args.brief or args.one_line:
            print('{}'.format(id))
        else:
            print('Job Delete {}'.format(id))
        # fi
        ret = True
    # rof
    return ret
# End of JobDele
#-----------------------------------------------------------------------------
def JobRun(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0]:
        print("Error - no jobs given to run")
        return False
    # fi
    ret = False
    for id in vargs:
        if not id.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            # fi
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == id:
                    id =  job['id']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print('Could not get information on job name "{}"'.format(id))
                continue
            # fi
        # fi
        # If name, "id" changed.
        (rt, r) = send_post(base_url, authentication, 'datamovement/jobs/{}/start?verify=false'.format(id), 202)
        if not rt:
            continue
        # fi
        if args.brief or args.one_line:
            print('{}'.format(id))
        else:
            print('Job Run {}'.format(id))
        # fi
        ret = True
    # rof
    return ret
# End of JobRun
#-----------------------------------------------------------------------------
def JobVerify(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0]:
        print("Error - no jobs given to verify")
        return False
    # fi
    ret = False
    for id in vargs:
        if not id.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            # fi
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == id:
                    id =  job['id']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print('Could not get information on job name "{}"'.format(id))
                continue
            # fi
        # fi
        # If name, "id" changed.
        (rt, r) = send_post(base_url, authentication, 'datamovement/jobs/{}/start?verify=true'.format(id), 202)
        if not rt:
            continue
        # fi
        if args.brief or args.one_line:
            print('{}'.format(id))
        else:
            print('Job Verify {}'.format(id))
        # fi
        ret = True
    # rof
    return ret
# End of JobVerify
#-----------------------------------------------------------------------------
def JobStop(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0]:
        print("Error - no jobs given to delete")
        return False
    # fi
    ret = False
    for id in vargs:
        if not id.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            # if
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == id:
                    id =  job['id']
                    found = True
                    break
                # if
            # rof
            if not found:
                print('Could not get information on job name "{}"'.format(id))
                continue
            # fi
        # fi
        # If name, "id" changed.
        (rt,r) = send_post(base_url, authentication, 'datamovement/jobs/{}/stop'.format(id), 202)
        if not rt:
            continue
        # fi
        if args.brief or args.one_line:
            print('{}'.format(id))
        else:
            print('Job Stop {}'.format(id))
        # fi
        ret = True
    # rof
    return ret
# End of JobStop
#-----------------------------------------------------------------------------
# NOTDONEYET - jobs edit jobid {...} - not tested, cleaned up, etc. - args.one_line, args.brief
def JobEdit(authentication, base_url, vargs):
    ''' returns true if good command. '''
    if not vargs or not vargs[0] or len(vargs) < 2:
        print('No edit values')
        return False
    # fi

    # Need check vargs is correct format.

    id = None
    data = None
    etag = None
    for edit in vargs:
        if id == None:
            id = edit
            (rt, r) = send_get(authentication, base_url, 'datamovement/jobs/{}'.format(id))
            etag = r.headers.get('etag').strip('"')
            data = r.json()
            del data['id']
            del data['end']
            del data['errors']
            del data['start']
            del data['state']
            del data['status']
            del data['warnings']
            del data['message']
            continue
        # fi
        fields = edit.split('=', 1)
        if fields[0] == 'src.remoteverifier':
            data['source']['remoteverifier'] = fields[1]
            del data['source']['localverifier']
        elif fields[0] == 'dst.remoteverifier':
            data['destination']['remoteverifier'] = fields[1]
            del data['destination']['localverifier']
        # fi
    # rof
    if data:
        params = {'If-Match':etag}
        url = base_url + 'datamovement/jobs/{}'.format(id)
        authentication.update({'If-Match':etag})
        r = requests.put(url, headers=authentication, json = data, verify=False)
        del authentication['If-Match']
        print('%3d : %s' % (r.status_code, _httpMap(r.status_code)))
        results = r.json()
        if results and 'detail' in results:
            print(results['detail'])
        # fi
    # fi
    return False
# End of JobEdit
#-----------------------------------------------------------------------------
def JobHelp():
    job_words = tab_words["jobs"]
    print("Job subtypes possible:", ', '.join(sorted(job_words)))
    print_help_jobs_all()
    return
# End of JobHelp
#-----------------------------------------------------------------------------
# Second argument is 'list', 'dele', 'run', 'verify', 'stop', 'start'.

def process_job(subtype, t_args, authentication, base_url):
    list_job = unique_dict_array(tab_words["jobs"])
 
    if subtype is None:                 # No subtype, print out jobs.
        return JobList(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_job['list']:
        return JobList(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_job['create']:
        return JobCreate(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_job['delete']:
        return JobDele(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_job['edit']:
        return JobEdit(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_job['run'] or subtype.lower() in list_job['start']:
        return JobRun(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_job['stop']:
        return JobStop(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_job['verify']:
        return JobVerify(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_job['help'] or subtype[0] == '?':
        JobHelp()
        return False
    # fi
    print("No job with subtype '{}' -- t_args={}".format(subtype, t_args))
    JobHelp()
    return False
# End of process_job
#=============================================================================
def ProjCreate(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0]:
        print("Must have 3 arguments to create:")
        print("    projects create projectname [ sourceSMBvers [ destinationSMBvers ] ]")
        print_help_projects_create()
        return False
    # fi

    if len(vargs) == 1:
        he = unique_dict_array({'help':''})
        if vargs[0] in he['help']:
            print_help_projects_create()
            return True
        # fi
        projname = vargs[0]
        srcvers = None
        dstvers = None
    elif len(vargs) == 2:
        projname = vargs[0]
        srcvers = vargs[1]
        dstvers = None
    elif len(vargs) == 3:
        projname = vargs[0]
        srcvers = vargs[1]
        dstvers = vargs[2]
    else:
        print("Must have 3 arguments to create (not {}):".format(len(vargs)))
        print("    projects create projectname [ sourceSMBvers [ destinationSMBvers ] ]")
        print_help_projects_create()
        return False
    # fi

    if srcvers and srcvers == "default":
        srcvers = None
    # fi
    if dstvers and dstvers == "default":
        dstvers = None
    # fi

    # Create project with projname.
    info = {'name':projname,
            'defaultmigjobrate':PROJECT_JOBRATE,
            'jobrunlimit':PROJECT_RUNLIMIT,
            'source': {'smbversion': srcvers},
            'destination': {'smbversion': dstvers}}

    (rt, r) = send_post_json(base_url, authentication, 'datamovement/projects', info, 200)
    if not rt:
        return False
    # fi
    if args.brief or args.one_line:
        print('{}'.format(r.json()['id']))
    else:
        print('Project Created {}'.format(r.json()['id']))
    # fi
    return True
# End of ProjCreate
#-----------------------------------------------------------------------------
def ProjDele(authentication, base_url, vargs):
    global args

    he = unique_dict_array({'help':''})
    if vargs is None or vargs[0] is None or vargs[0] == '' or vargs[0] in he['help']:
        print_help_projects_delete()
        return True
    # fi
    ret = False
    projlist = None
    for id in vargs:
        if not id:
            continue
        # fi
        if not id.isdigit():
            # A project name if here.
            if projlist is None:
                (ret, projlist) = send_get(authentication, base_url, 'datamovement/projects')
                if not ret:
                    print("Error: argument is not a number or project name '{}'".format(id))
                    continue
                # fi
            # fi
            found = False
            for project in projlist.json()['projects']:
                if project['name'] == id:
                    id = project['id']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print("Could not get information for project name {}".format(id))
                continue
            # fi
        # fi

        # A project id number if here.
        (rt, r) = send_delete(base_url, authentication, 'datamovement/projects/{}'.format(id))
        if not rt:
            continue
        # fi
        ret = True
        if args.brief or args.one_line:
            print('{}'.format(r.json()['id']))
        else:
            print('Project Deleted {}'.format(r.json()['id']))
        # fi
    # rof
    return ret
# End of ProjDele
#-----------------------------------------------------------------------------
def ProjectHelp():
    project_words = tab_words["projects"]
    print("Project subtypes possible:", ', '.join(sorted(project_words)))
    print_help_projects_all()
    return
# End of ProjectHelp
#-----------------------------------------------------------------------------
def process_proj(subtype, t_args, authentication, base_url):
    list_proj = unique_dict_array(tab_words["projects"])

    ret = False
    if subtype is None:                 # No subtype, print out projects.
        return ProjList(authentication, base_url, t_args, True)
    # fi
    if subtype.lower() in list_proj['list']:
        return ProjList(authentication, base_url, t_args, True)
    # fi
    if subtype.lower() in list_proj['create']:
        return ProjCreate(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_proj['delete']:
        return ProjDele(authentication, base_url, t_args)
    # fi
    if subtype.lower() in list_proj['help'] or subtype[0] == '?':
        ProjectHelp()
    else:
        print("No project with subtype '{}' -- t_args={}".format(subtype, t_args))
        ProjectHelp()
    # fi
    return False
# End of process_proj
#=============================================================================
def print_storage_devices_output(s):
    global args

    if args.brief:
        print("{}".format(s['id']))
    elif args.one_line:
        print("{} {} {} '{}'".format(s['id'], s['protocolid'], s['online'], s['name']))
    else:
        print("id={} protocolid={} online={} name='{}'".format(
              s['id'], s['protocolid'], s['online'], s['name']))
    # fi
# End of print_storage_devices_output
#-----------------------------------------------------------------------------
def StorageAssetsDevices_List(authentication, base_url, vargs):
    global args

    if not args.brief and not args.one_line:
        print('...')
    # fi
    (rt, r) = send_get(authentication, base_url, 'storage/assets/devices')
    if not rt:
        if not args.brief and not args.one_line:
            print('Could not get information on storage devices')
        # fi
        return False
    # fi

    storagedevices = r.json()
    if storagedevices['deviceassets'] == []:
        if not args.brief and not args.one_line:
            print('No storage devices present.')
        # fi
        return False
    # fi

    if not args.one_line and not args.brief:
        print('Storage Devices:')
    for s in storagedevices['deviceassets']:
        if not vargs or s['name'] in vargs or str(s['id']) in vargs:
            print_storage_devices_output(s)
        # fi
    # rof
    return True
# End of StorageAssetsDevices_List
#-----------------------------------------------------------------------------
def StorageAssetsDevicesHelp():
    storage_assets_devices_words = tab_words["storage"]["devices"]
    print("Storage Devices subtypes possible:", ', '.join(sorted(storage_assets_devices_words)))
    print_help_storage_devices()
    return
# End of StorageAssetsDevicesHelp
#-----------------------------------------------------------------------------
def StorageAssetsDevices(authentication, base_url, t_args):
    global args

    if not t_args or not t_args[0]:
        StorageAssetsDevices_List(authentication, base_url, None)
        return True
    # fi
    subtype = t_args[0].lower()
    storagedevices = unique_dict_array(tab_words["storage"]["devices"])
    if subtype in storagedevices['list']:
        StorageAssetsDevices_List(authentication, base_url, t_args[1:])
        return True
    # fi
    if subtype in storagedevices['help'] or subtype[0] == '?':
        StorageAssetsDevicesHelp()
        return True
    # fi
    if not args.one_line and not args.brief:
        print("No storage devices with subtype '{}' -- t_args={}".format(subtype, t_args[1:]))
        StorageAssetsDevicesHelp()
    return False
# End of StorageAssetsDevices
#-----------------------------------------------------------------------------
def print_storage_files_output(s):
    global args

    d = s["definition"]
    if args.brief:
        print("{}".format(s['id']))
    elif args.one_line:
        print("{} {} {} '{}' '{}' '{}'".format(
              s['id'], s['protocolid'], s['online'], d['type'], s['name'], d['share']))
    else:
        if d['type'] == 'SMB':
            print("id={} protocolid={} online={} type='{}' name='{}' share='{}'".format(
                  s['id'], s['protocolid'], s['online'], d['type'], s['name'], d['share']))
        elif d['type'] == 'NFS':
            print("id={} protocolid={} online={} type='{}' name='{}' export='{}'".format(
                  s['id'], s['protocolid'], s['online'], d['type'], s['name'], d['export']))
        # fi
    # fi
# End of print_storage_files_output
#-----------------------------------------------------------------------------
def StorageAssetsFiles_List(authentication, base_url, vargs):
    global args

    if not args.brief and not args.one_line:
        print('...')
    # fi
    (rt, r) = send_get(authentication, base_url, 'storage/assets/files')
    if not rt:
        if not args.brief and not args.one_line:
            print('Could not get information on storage files')
        # fi
        return False
    # fi

    storagefiles = r.json()
    if storagefiles['fileassets'] == []:
        if not args.brief and not args.one_line:
            print('No storage files present.')
        # fi
        return False
    # fi

    if not args.brief and not args.one_line:
        print('Storage Files:')
    for s in storagefiles['fileassets']:
        if not vargs or s['name'] in vargs or str(s['id']) in vargs:
            print_storage_files_output(s)
        # fi
    # rof
    return True
# End of StorageAssetsFiles_List
#-----------------------------------------------------------------------------
def StorageAssetsFilesHelp():
    storage_assets_files_words = tab_words["storage"]["files"]
    print("Storage Files subtypes possible:", ', '.join(sorted(storage_assets_files_words)))
    print_help_storage_files()
    return
# End of StorageAssetsFilesHelp
#-----------------------------------------------------------------------------
def StorageFiles_Delete(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0]:
        print("Error - no storage files given to delete")
        return False
    # fi
    (ret, storagefileslist) = send_get(authentication, base_url, 'storage/assets/files')
    if not ret:
        print("Error: from send_get storage/assets/files request")
        return False
    # fi
    # print("type(storagefileslist.json())={} storagefileslist.json()={}".format(type(storagefileslist.json()),storagefileslist.json()))
    for id in vargs:
        if not id:
            continue
        # fi
        if id.isdigit():
            found = False
            for file in storagefileslist.json()['fileassets']:
                if str(file['id']) == id:
                    id = file['id']
                    name = file['name']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print("Could not get information for storage file id {}".format(id))
                return False
            # fi
        else:
            found = False
            for file in storagefileslist.json()['fileassets']:
                if file['name'] == id:
                    id = file['id']
                    name = file['name']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print("Could not get information for storage file name {}".format(id))
                return False
            # fi
        # fi

        # A file id number if here.
        (rt, r) = send_delete(base_url, authentication, 'storage/assets/files/{}'.format(id))
        if not rt:
            continue
        # fi
        if args.brief:
            print("{}".format(id))
        elif args.one_line:
            print("{} '{}'".format(id,name))
        else:
            print("Storage File {} '{}' deleted.".format(id,name))
        # fi
    # rof
    return True
# End of StorageFiles_Delete
#-----------------------------------------------------------------------------
def StorageAssetsFiles(authentication, base_url, t_args):
    if not t_args or not t_args[0]:
        StorageAssetsFiles_List(authentication, base_url, None)
        return True
    # fi
    subtype = t_args[0].lower()
    storagefiles = unique_dict_array(tab_words["storage"]["files"])
    if subtype in storagefiles['list']:
        StorageAssetsFiles_List(authentication, base_url, t_args[1:])
        return True
    # fi
    if subtype in storagefiles['create']:
        print("StorageAssetsFiles NOTDONEYET - create")
        return False
    # fi
    if subtype in storagefiles['delete']:
        return StorageFiles_Delete(authentication, base_url, t_args[1:])
    # fi
    if subtype in storagefiles['help'] or subtype[0] == '?':
        StorageAssetsFilesHelp()
        return True
    # fi
    print("No storage files with subtype '{}' -- t_args={}".format(subtype, t_args[1:]))
    StorageAssetsFilesHelp()
    return False
# End of StorageAssetsFiles
#-----------------------------------------------------------------------------
def print_storage_protocols_output(s):
    global args

    d = s["definition"]
    if args.brief:
        if d['type'] == 'SMB':
            print("{} {} '{}' '{}' '{}' '{}' '{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], d['username'], s['name'], s['message']))
        # fi
        elif d['type'] == 'NFS':
            print("{} {} '{}' '{}' '{}' '{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], s['name'], s['message']))
        elif d['type'] == 'SCSI':
            print("{} {} '{}' '{}' '{}' '{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], s['name'], s['message']))
        else:
            print("NOTDONEYET-fix: id={} systemid={} type='{}' name='{}' message='{}'".format(
                  s['id'], s['systemid'], d['type'], s['name'], s['message']))
        # fi
    elif args.one_line:
        if d['type'] == 'SMB':
            print("{} {} '{}' '{}' '{}' '{}' '{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], d['username'], s['name'], s['message']))
        elif d['type'] == 'NFS':
            print("{} {} t'{}' '{}' '{}' '{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], s['name'], s['message']))
        elif d['type'] == 'SCSI':
            print("{} {} '{}' '{}' '{}' '{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], s['name'], s['message']))
        else:
            print("NOTDONEYET-fix: id={} systemid={} type='{}' name='{}' message='{}'".format(
                  s['id'], s['systemid'], d['type'], s['name'], s['message']))
        # fi
    else:
        if d['type'] == 'SMB':
            print("id={} systemid={} type='{}' host='{}' username='{}' name='{}' message='{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], d['username'], s['name'], s['message']))
        elif d['type'] == 'NFS':
            print("id={} systemid={} type='{}' host='{}' name='{}' message='{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], s['name'], s['message']))
        elif d['type'] == 'SCSI':
            print("id={} systemid={} type='{}' host='{}' name='{}' message='{}'".format(
                  s['id'], s['systemid'], d['type'], d['host'], s['name'], s['message']))
        else:
            print("NOTDONEYET-fix: id={} systemid={} type='{}' name='{}' message='{}'".format(
                  s['id'], s['systemid'], d['type'], s['name'], s['message']))
        # fi
    # fi
# End of print_storage_protocols_output
#-----------------------------------------------------------------------------
def StorageProtocols_List(authentication, base_url, vargs):
    global args

    (rt, r) = send_get(authentication, base_url, 'storage/protocols')
    if not rt:
        if not args.brief and not args.one_line:
            print('Could not get information on storage protocols')
        # fi
        return False
    # fi

    storageprotocols = r.json()
    if storageprotocols['protocols'] == []:
        if not args.brief and not args.one_line:
            print('No storage protocols present.')
        # fi
        return False
    # fi

    if not args.brief and not args.one_line:
        print('Storage Protocols:')
    # fi
    for s in storageprotocols['protocols']:
        if not vargs or s['name'] in vargs or str(s['id']) in vargs:
            print_storage_protocols_output(s)
        # fi
    # rof
    return True
# End of StorageProtocols_List
#-----------------------------------------------------------------------------
def StorageProtocols_Delete(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0]:
        print("Error - no storage protocols given to delete")
        return False
    # fi
    (ret, storageprotocolslist) = send_get(authentication, base_url, 'storage/protocols')
    if not ret:
        print("Error: Error from send_get storage/protocols request")
        return False
    # fi
    for id in vargs:
        if not id:
            continue
        # fi
        if id.isdigit():
            found = False
            for protocol in storageprotocolslist.json()['protocols']:
                if str(protocol['id']) == id:
                    id = protocol['id']
                    name = protocol['name']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print("Could not get information for storage protocol id {}".format(id))
                return False
            # fi
        else:
            found = False
            for protocol in storageprotocolslist.json()['protocols']:
                if protocol['name'] == id:
                    id = protocol['id']
                    name = protocol['name']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print("Could not get information for storage protocol name {}".format(id))
                return False
            # fi
        # fi

        # A protocol id number if here.
        (rt, r) = send_delete(base_url, authentication, 'storage/protocols/{}'.format(id))
        if not rt:
            continue
        # fi
        if args.brief:
            print("{}".format(id))
        elif args.one_line:
            print("{} '{}'".format(id,name))
        else:
            print("Storage Protocol {} '{}' deleted.".format(id,name))
        # fi
    # rof
    return True
# End of StorageProtocols_Delete
#-----------------------------------------------------------------------------
def StorageProtocols_Create(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0] or not vargs[1]:
        print("Not enough arguments to create storage protocol:")
        print_help_storage_protocols()
        return False
    # fi
    type = vargs[1].upper()
    if type == 'SMB' and len(vargs) != 6: 
        print("Must have at least 6 arguments to create SMB:")
        print("   storage protocols create systemID SMB HostIP Username Password Name_For_Storage_Group")
        print_help_storage_protocols()
        return False
    # fi
    if type == 'NFS' and len(vargs) != 4: 
        print("Must have at least 6 arguments to create NFS:")
        print("   storage protocols create systemID NFS HostIP Name_For_Storage_Group")
        print_help_storage_protocols()
        return False
    # fi
    if type == 'SCSI' and len(vargs) != 3:
        print("SCSI system protocols create systemID SCSI Name_For_Storage_Group")
        print_help_storage_protocols()
        return False
    # fi

    if not vargs[0].isdigit():
        (rt, r) = send_get(authentication, base_url, 'storage/systems')
        if not rt:
            if not args.brief and not args.one_line:
                print('Could not get information for storage systems to find a name.')
            # fi
            return False
        # fi
        storagesystems = r.json()
        if storagesystems['systems'] == []:
            if not args.brief and not args.one_line:
                print('Could not get information for storage systems to find a name, no storage systems.')
            # fi
            return False
        # fi
        systemid = ''
        for s in storagesystems['systems']:
            if s['name'] == vargs[0]:
                systemid = s['id']
                break;
            # fi
        # rof
        if systemid == '':
            print("Could not find storage system name '{}'".format(vargs[0]))
            return False
        # fi
    else:
        systemid = int(vargs[0])
    # fi
    if type == 'SMB':
        ip = vargs[2]
        username = vargs[3]
        password = vargs[4]
        storage_name = vargs[5]
        # Create SMB protocol.
        info = { 'definition':
                   {
                     "host": ip,
                     "type": type,
                     "username": username,
                     "password": password
                   },
                 'name': storage_name,
                 'systemid': systemid
               }
    elif type == 'NFS':
        ip = vargs[2]
        storage_name = vargs[3]
        # Create NFS protocol.
        info = { 'definition':
                   {
                     "host": ip,
                     "type": type,
                   },
                 'name': storage_name,
                 'systemid': systemid
               }
    # fi
    elif type == 'SCSI':
        storage_name = vargs[2]
        # Create SCSCI protocol.
        info = { 'definition':
                   {
                     "host": None,
                     "type": type,
                   },
                 'name': storage_name,
                 'systemid': systemid
               }
    else:
        print("Type {} system protocols creation -- NOTDONEYET".format(type))
        return False
    #fi
    if not args.brief and not args.one_line:
        print('...')
    #fi
    (rt, r) = send_post_json(base_url, authentication, 'storage/protocols', info, 200)
    if not rt:
        return False
    # fi
    if args.brief:
        print("{}".format(r.json()['id']))
    elif args.one_line:
        print("{} '{}'".format(r.json()['id'], r.json()['name']))
    else:
        print("Storage Protocol Created {} - '{}'".format(r.json()['id'], r.json()['name']))
    return True
# End of StorageProtocols_Create
#-----------------------------------------------------------------------------
def StorageProtocolsHelp():
    storage_protocols_words = tab_words["storage"]["protocols"]
    print("Storage Protocols subtypes possible:", ', '.join(sorted(storage_protocols_words)))
    print_help_storage_protocols()
    return
# End of StorageProtocolsHelp
#-----------------------------------------------------------------------------
def StorageProtocols(authentication, base_url, t_args):
    if not t_args or not t_args[0]:
        StorageProtocols_List(authentication, base_url, None)
        return True
    # fi
    subtype = t_args[0].lower()
    storageprotocol = unique_dict_array(tab_words["storage"]["protocols"])
    if subtype in storageprotocol['list']:
        StorageProtocols_List(authentication, base_url, t_args[1:])
        return True
    # fi
    if subtype in storageprotocol['create']:
        return StorageProtocols_Create(authentication, base_url, t_args[1:])
    # fi
    if subtype in storageprotocol['delete']:
        return StorageProtocols_Delete(authentication, base_url, t_args[1:])
    # fi
    if subtype in storageprotocol['help'] or subtype[0] == '?':
        StorageProtocolsHelp()
        return True
    # fi
    print("No storage protocol with subtype '{}' -- t_args={}".format(subtype, t_args[1:]))
    StorageProtocolsHelp()
    return False
# End of StorageProtocols
#-----------------------------------------------------------------------------
def print_storage_systems_output(s):
    global args

    if args.brief:
        print("{}".format(s['id']))
    elif args.one_line:
        print("{} '{}'".format(s['id'], s['name']))
    else:
        print("id={} name='{}'".format(s['id'], s['name']))
    # fi
# End of print_storage_systems_output
#-----------------------------------------------------------------------------
def StorageSystems_List(authentication, base_url, vargs):
    global args

    (rt, r) = send_get(authentication, base_url, 'storage/systems')
    if not rt:
        if not args.brief and not args.one_line:
            print('Could not get information for storage systems')
        # fi
        return False
    # fi

    storagesystems = r.json()
    if storagesystems['systems'] == []:
        if not args.brief and not args.one_line:
            print('No storage systems present.')
        # fi
        return False
    # fi

    if not args.brief and not args.one_line:
        print('Storage Systems:')
    for s in storagesystems['systems']:
        if not vargs or s['name'] in vargs or str(s['id']) in vargs:
            print_storage_systems_output(s)
        # fi
    # rof
    return True
# End of StorageSystems_List
#-----------------------------------------------------------------------------
def StorageSystems_Delete(authentication, base_url, vargs):
    global args

    if not vargs or not vargs[0]:
        print("Error - no storage systems given to delete")
        return False
    # fi
    (ret, storagesystemslist) = send_get(authentication, base_url, 'storage/systems')
    if not ret:
        print("Error: Error from send_get storage/systems request")
        return False
    # fi
    for id in vargs:
        if not id:
            continue
        # fi
        if id.isdigit():
            found = False
            for system in storagesystemslist.json()['systems']:
                if str(system['id']) == id:
                    id = system['id']
                    name = system['name']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print("Could not get information for system id {}".format(id))
                return False
            # fi
        else:
            found = False
            for system in storagesystemslist.json()['systems']:
                if system['name'] == id:
                    id = system['id']
                    name = system['name']
                    found = True
                    break
                # fi
            # rof
            if not found:
                print("Could not get information for system name {}".format(id))
                return False
            # fi
        # fi

        # A system id number if here.
        (rt, r) = send_delete(base_url, authentication, 'storage/systems/{}'.format(id))
        if not rt:
            continue
        # fi
        if args.brief:
            print("{}".format(id))
        elif args.one_line:
            print("{} '{}'".format(id,name))
        else:
            print("Storage Systems {} '{}' deleted.".format(id,name))
        # fi
    # rof
    return True
# End of StorageSystems_Delete
#-----------------------------------------------------------------------------
def StorageSystems_Create(authentication, base_url, vargs):
    global args

    if (not vargs or not vargs[0]) or len(vargs) != 1:
        print("Must have exactly one argument to Storage Systems Create:")
        print("  storage system create Name_For_Storage_System")
        print("example: storage system create 'Some Storage System for SMB'")
        return False
    # fi
    if not args.brief and not args.one_line:
        print('...')

    # fi
    storage_system_name = vargs[0]
    info = { 'name': storage_system_name }
    (rt, r) = send_post_json(base_url, authentication, 'storage/systems', info, 200)
    if not rt:
        return False
    # fi
    if args.brief:
        print("{}".format(r.json()['id']))
    elif args.one_line:
        print("{} '{}'".format(r.json()['id'], r.json()['name']))
    else:
        print("Storage System Created {} - '{}'".format(r.json()['id'], r.json()['name']))
    # fi
    return True
# End of StorageSystems_Create
#-----------------------------------------------------------------------------
def StorageSystemsHelp():
    storage_systems_words = tab_words["storage"]["systems"]
    print("Storage Systems subtypes possible:", ', '.join(sorted(storage_systems_words)))
    print_help_storage_systems()
    return
# End of StorageSystemsHelp
#-----------------------------------------------------------------------------
def StorageSystems(authentication, base_url, t_args):
    if not t_args or not t_args[0]:
        StorageSystems_List(authentication, base_url, None)
        return True
    # fi
    subtype = t_args[0].lower()
    vargs = t_args[1:]
    storagesystems = unique_dict_array(tab_words["storage"]["systems"])
    if subtype in storagesystems['list']:
        StorageSystems_List(authentication, base_url, t_args[1:])
        return True
    # fi
    if subtype in storagesystems['create']:
        return StorageSystems_Create(authentication, base_url, t_args[1:])
    # fi
    if subtype in storagesystems['delete']:
        return StorageSystems_Delete(authentication, base_url, t_args[1:])
    # fi
    if subtype in storagesystems['help'] or subtype[0] == '?':
        StorageSystemsHelp()
        return True
    # fi
    print("No storage systems with subtype '{}' -- t_args={}".format(subtype, t_args[1:]))
    StorageSystemsHelp()
    return False
# End of StorageSystems
#-----------------------------------------------------------------------------
def StorageHelp():
    storage_words = tab_words["storage"]
    print("Storage subtypes possible:", ', '.join(sorted(storage_words)))
    print_help_storage_all()
    return
# End of StorageHelp
#-----------------------------------------------------------------------------
def process_storage(subtype, t_args, authentication, base_url):
    list_storage = unique_dict_array(tab_words["storage"])

    if subtype is None or subtype.lower() in list_storage['list']:
        ret = False
        if t_args is None or t_args[0] == '':
            # Do all.
            ret = StorageAssetsDevices(authentication, base_url, t_args)
            ret += StorageAssetsFiles(authentication, base_url, t_args)
            ret += StorageProtocols(authentication, base_url, t_args)
            ret += StorageSystems(authentication, base_url, t_args)
        elif t_args[0].lower() in list_storage['devices']:
            ret = StorageAssetsDevices_List(authentication, base_url, t_args[1:])
        elif t_args[0].lower() in list_storage['files']:
            ret += StorageAssetsFiles_List(authentication, base_url, t_args[1:])
        elif t_args[0].lower() in list_storage['protocols']:
            ret += StorageProtocols_List(authentication, base_url, t_args[1:])
        elif t_args[0].lower() in list_storage['systems']:
            ret += StorageSystems_List(authentication, base_url, t_args[1:])
        else:
            print("No storage list with t_args={}".format(t_args))
            StorageHelp()
            ret = False
        return ret
            
    if subtype.lower() in list_storage['devices']:
        return StorageAssetsDevices(authentication, base_url, t_args)
    if (subtype.lower() in list_storage['files']):
        return StorageAssetsFiles(authentication, base_url, t_args)
    if subtype.lower() in list_storage['protocols']:
        return StorageProtocols(authentication, base_url, t_args)
    if subtype.lower() in list_storage['systems']:
        return StorageSystems(authentication, base_url, t_args)
    if subtype.lower() in list_storage['help'] or subtype[0] == '?':
        StorageHelp()
        return False

    print("No storage with subtype '{}' -- t_args={}".format(subtype, t_args))
    StorageHelp()
    return False
# End of process_storage
#=============================================================================
# Parse and process line.
def process_line(t, authentication, base_url):
    if len(t) == 0:
        command = subtype = t_args = None
    elif len(t) == 1:
        command = t[0].lower()
        subtype = t_args = None
    elif len(t) == 2:
        command = t[0].lower()
        subtype = t[1]
        t_args = None
    else:
        command = t[0].lower()
        subtype = t[1]
        t_args = t[2:]
    # fi

    # For parsing first argument - get all possible first, then get rid of duplicates.
    first_list = unique_dict_array(tab_words)

    # Try to process command.
    if command is None:                 # Ignore nothing given.
        return False
    if command in first_list['projects']:
        return process_proj(subtype, t_args, authentication, base_url)
    if command in first_list['jobs']:
        return process_job(subtype, t_args, authentication, base_url)
    if command in first_list['storage']:
        return process_storage(subtype, t_args, authentication, base_url)
    if command in first_list['exit'] or command in first_list['quit']:
        return Exit(subtype)
    if command in first_list['history']:
        return History()
    if command in first_list['sleep']:
        return Sleep(subtype)
    if command in first_list['brief']:
        return Brief(subtype)
    if command in first_list['one-line']:
        return One_Line(subtype)
    if command in first_list['help'] or command[0] == '?':
        return Help(subtype, first_list, t_args)
    if command in first_list['example']:
        return print_help_example()

    print("Unrecognized command, or not unique", t)
    Help(subtype, None, None)
    return False
# End of process_line
#-----------------------------------------------------------------------------
# Main program follows.
def main():
    global projMap, _jobId
    global args
    global history
    global pp

    # Turn off SSL warnings about certificates.
    requests.urllib3.disable_warnings()

    pp = pprint.PrettyPrinter(depth=4)

    parse_args()

    projMap = dict()
    history = []

    _jobId = os.getpid()

    # Login and get authorization.
    (authentication, base_url) = Login(args.parsec, args.version, args.user, args.passwd)

    # Pre-get the project list.
    ProjList(authentication, base_url, None, False)

    if args.rest and args.rest[0]:
        process_line(args.rest, authentication, base_url)
        return
    # fi

    # Command line history.
    if os.path.exists(HIST_FILE):
        readline.read_history_file(HIST_FILE)
    # fi
    readline.set_history_length(10000)

    completer = dnsCompleter( tab_words )
    readline.set_completer(completer.complete)

    while True:
        try:
            if sys.stdin.isatty():
                if sys.platform == 'darwin':
                    input('ready> ')
                    line = readline.get_line_buffer()
                else:
                    line = input('ready> ')
                # fi
            else:
                line = sys.stdin.readline()
            if line:
                line = line.strip()
                if not sys.stdin.isatty():
                    print('READ>',line)
                # fi

                # Parse and process line.
                try:
                    t = shlex.split(line)
                except ValueError as ex:
                    print("Parsing error in line '{}'".format(line))
                    print("    ", str(ex))
                    continue
                except:
                    print("Parsing error in line '{}'".format(line))
                    print("    ", sys.exc_info()[0])
                    continue
                # yrt

                if t and t[0]:
                    good = process_line(t, authentication, base_url)
                    if good:
                        history.append(line)
                    # fi
                # fi
            # fi
        except (EOFError, SystemExit, KeyboardInterrupt):
            # History writing here.
            readline.write_history_file(HIST_FILE)
            sys.exit(1)
        # yrt
    # elihw
    return
# End of main
#-----------------------------------------------------------------------------
if __name__ == '__main__':
    main()
#fi
#-----------------------------------------------------------------------------
exit(0)
#-----------------------------------------------------------------------------
# End of file m4.py
