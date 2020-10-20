#!/usr/bin/python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
#
# TODO: project status ...  ??      history? info?
# TODO: job status ...      ??
# TODO: job log ...         ??
# TODO: job errors ...      ??
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

    j                                   # List all jobs.
    j l                                 # List all jobs.
    job delete jobid ["name"] [id]      # Delete job(s) by ID or name.
    job disable jobid ["name"] [id]     # Disable job(s) by ID or name. -- NOTDONEYET
    job enable jobid ["name"] [id]      # Enable job(s) by ID or name. -- NOTDONEYET
    job list                            # List all jobs.
    job list jobid ["name"] [id]        # List job(s) by ID or name.
    jobs project ["name"] [id]          # List job(s) for one project ID or name.
    job run jobid ["name"] [id]         # Run job(s) by ID or name.
    job stop jobid ["name"] [id]        # Stop job(s) by ID or name.
    job verify jobid ["name"] [id]      # Start verify job(s) by ID or name.
    jobs                                # Easier to remember than "job". :)

    job create "hi" src dst "job name"
                                        # Create job attached to project "hi",
                                          moving src to dst, named "job name".

    job new projid src_uri dest_uri "job name" src_vers dst_vers
                                        # Alternative name for "job create".

    job edit jobid {name=value}         # Useful for remote -- untested recently.

    p                                   # List all projects.
    proj                                # List all projects.
    proj list                           # List all projects.
    proj list projectid                 # List specific project by name or ID.
    proj dele "name of project" [id]    # Delete project by name or ID.
    proj create "name of project" src_vers dst_vers
                                        # Create project with provided name.
                                          with source SMB version, and destination.
    projects                            # Easier to remember than proj. :)

Examples:
    job new 26 nfs://172.22.14.107/vol/m4_v1 nfs://172.22.14.107/vol/m4_v2
    jobs
    job run 59
    sleep 240
    job dele 59

    project create "MyMigrations" "1.0" "2.0"       # source share uses SMB version 1.0
    project create "MyMigrations" "1.0" ""          # default dst to 3.1.1 - Linux default.
    p c "MyMigrations" "1.0" "default"              # If you want to see the argument default.
    job create 26 cifs://172.22.14.107/m4_v1 cifs://172.22.14.107/m4_v2 "mySMB"
    job c 26 nfs://172.22.14.54/vol/m4_v1 parsec://m4_139/ "myNFS"
    job n 26 parsec://m4_139/ nfs://172.22.14.54/vol/m4_v2 "myRemote"
    job edit 57 edit dst.remoteverifier=...
    j e 58 edit src.remoteverifier=...
    j r 57
    j p 19                                          # See all jobs for project 19.

    ./m4.py --parsec IpAddr --user My --passwd Mine pro l "MyMigrations" 33 44
    export pxDEV=IpAddr pxUSER=My pxPASS=Mine ; ./m4.py jobs l 20 21 22 "myNFS"

Environment Variables:
    The following environment variables may be set to avoid using the --parsec,
    --user and --passwd command line options:
        pxDEV   hostname or IP address of Parsec appliance
        pxUSER  user name of the account you want to authenticate to
        pxPASS  password for the account you want to authenticate to
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
# parsing first argument
list_proj    = ['p', 'pr', 'pro', 'proj', 'proje', 'projec', 'project', 'projects']
list_job     = ['j', 'jo', 'job', 'jobs']
list_exit    = ['e', 'x', 'ex', 'exi', 'exit', 'q', 'qu', 'qui', 'quit']
list_help    = ['he', 'hel', 'help']                                    # Overlaps history.
list_history = ['hi', 'his', 'hist', 'histo', 'histor', 'history']      # Overlaps help.
list_sleep   = ['s', 'sl', 'sle', 'slee', 'sleep']
list_brief   = ['b', 'br', 'bri', 'brie', 'brief']

tab_words = ['projects ', 'jobs ', 'quit', 'exit', 'help', 'history', 'sleep', 'brief']
#-----------------------------------------------------------------------------
class dnsCompleter:
    def __init__(self, options):
        self.options = options
        self.current_candidates = []
        return
    # End of __init__

    def complete(self, text, state):
        if state == 0:
            # This is the first time for this text -- build possible match list.
            origline = readline.get_line_buffer().rstrip()
            line = origline.lstrip()
            delta = len(origline) - len(line)
            begin = readline.get_begidx() - delta
            end = readline.get_endidx() - delta
            being_completed = line[begin:end]
            words = line.split()

            if not words:                   # If no starting word, all are possible.
                self.current_candidates = sorted(self.options)
            else:
                try:
                    if begin != 0:          # If first character of first word.
                        # later word        # No autofill for other arguments.
                        return None
                    # fi
                    # first word
                    candidates = self.options
                    if being_completed:
                        # Match with portion of input already typed.
                        self.current_candidates = [ w for w in candidates if w.startswith(being_completed) ]
                    else:
                        # Matching empty string, use all possible input.
                        self.current_candidates = candidates
                    # fi
                except (KeyError, IndexError) as err:
                    self.current_candidates = []
                # yrt
            # fi
        # fi
        try:
            response = self.current_candidates[state]
        except IndexError:
            response = None
        # yrt
        return response
    # End of complete
# End of dnsCompleter
#-----------------------------------------------------------------------------
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
    parser.add_argument('--version', default='/api/v2',
                        help='The REST api version - default "/api/v2".')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help = 'Verbose mode output')
    parser.add_argument('--brief', '-b', '-br', action='store_true',
                        help = 'Very brief output for job/project list')
    parser.add_argument('rest', nargs='*',
                        help='Optional command to execute')
    args = parser.parse_args()

    if not args.parsec or not args.user or not args.passwd:
        print('Need --parsec and --user and --passwd')
        exit(1)
    # fi
    return
# End of parse_args
#-----------------------------------------------------------------------------
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
    BASEURL = 'https://%s%s/' % (ipaddr, version)
    url = BASEURL + 'auth'
    r = requests.get(url, auth=(user, pw), verify=False)
    if r.status_code != 200:
        print_get_error_exit('Could not login to device! Return response:', r)
    # fi
    return({'Authorization':'Bearer %s' % r.text}, BASEURL)
# End of Login
#-----------------------------------------------------------------------------
def send_get(authentication, base_url, tackon):
    r = requests.get(base_url + tackon, headers=authentication, verify=False)
    if r.status_code != 200:
        print_get_error("Request for %s failed. Response:" % tackon, r)
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
#-----------------------------------------------------------------------------
# Called when program starts. This sets projMap dictionary.
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
            if args.brief and display:
                print('No projects present.')
            # fi
            return True
        # fi
        if args.brief and display:
            print('Projects:')
        # fi
        for proj in results['projects']:
            projMap[proj['id']] = proj['name']
            if display:
                if not args.brief:
                    print('%(id)4d: message: %(message)s    name \"%(name)s\"' % proj)
                else:
                    print('%(id)4d' % proj)
                # fi
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
                    print('Project "%s" id %s' % (id, project['id']))
                    found = True
                    break
                # fi
            # rof
            if not found:
                print('Could not get information on project name "%s"' % id)
            # fi
            continue
        # fi

        # A project id number if here.
        (rt, r) = send_get(authentication, base_url, 'datamovement/projects/%s' % id)
        if not rt:
            continue
        # fi
        ret = True
        results = r.json()
        print('Project "%s":' % id)
        print(yaml.dump(results, default_flow_style=False))
    # rof
    return ret
# End of ProjList
#-----------------------------------------------------------------------------
def Help():
    print(__doc__)
    return True
# End of Help
#-----------------------------------------------------------------------------
def Exit(subtype):
    if subtype is None or subtype == '':
        exit(0)
    # fi
    if not subtype.isdigit():
        print("Error: argument is not a number '%s'" % subtype)
        return False
    # fi
    exit(int(subtype))
# End of Exit
#-----------------------------------------------------------------------------
def Sleep(subtype):
    if subtype is not None and subtype != '':
        if not subtype.isdigit():
            print("Error: argument is not a number '%s'" % subtype)
            return False
        # fi
        time.sleep(int(subtype))
    # fi
    return True
# End of Sleep
#-----------------------------------------------------------------------------
def Brief(subtype):
    global args

    if subtype is None or subtype == '' or subtype == 'on':
        args.brief = True
        print('Brief mode enabled')
        return True
    # fi
    if subtype == 'of' or subtype == 'off':
        print('Brief mode disabled (i.e. full print mode)')
        args.brief = False
        return True
    # fi

    print("Error: Brief argument '%s' no recognized as 'on' or 'off'" % subtype)
    if args.brief is None or args.brief:
        print("Error: Brief left as 'on'")
    else:
        print("Error: Brief left as 'off'")
    # fi
    return False
# End of Brief
#-----------------------------------------------------------------------------
def History():
    global history

    cnt = 1
    for cmd in history:
        print(' %4d: %s' % (cnt, cmd))
        cnt += 1
    # rof
    return True
# End of History
#-----------------------------------------------------------------------------
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

    print("Unknown URI '%s'" % uri)
    return (False,dict())
# End of _parseJobURI
#-----------------------------------------------------------------------------
def JobCreate(authentication, base_url, vargs):
    global _jobId

    if not vargs or not vargs or not vargs[0]:
        print("Must have 4 arguments to create/new:")
        print("   projectId, source, destination, JobName")
        return False
    # fi

    if len(vargs) != 4:
        print("Must have 4 arguments to create/new (not %s):" % len(vargs))
        print("   projectId, source, destination, JobName")
        return False
    # fi

    (rt, s) = _parseJobURI(vargs[1])
    if not rt:
        print("Second argument must be URI for source. nfs://127.0.0.1/vol")
        return False
    # fi
    (rt, d) = _parseJobURI(vargs[2])
    if not rt:
        print("Third argument must be URI for Destination. smb://127.0.0.1/share")
        return False
    # fi

    id = vargs[0]
    if not id.isdigit():
        # A project name if here.
        (ret, projlist) = send_get(authentication, base_url, 'datamovement/projects')
        if not ret:
            print("Error: First argument is not a number or project name '%s'" % id)
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
            print("Could not get information for project name %s" % id)
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
        print('Job ID: %(id)s' % results)
    # fi
    if results and 'detail' in results:
        print(results['detail'])
    # fi
    return True
# End of JobCreate
#-----------------------------------------------------------------------------
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
    print('%(id)4d: Status %(status)-8s   projectid %(projectid)s    project name \"%(proj)s\"' % job)
    mess = ''
    if 'message' in job:
        if job['message']:
            mess = job['message']
        # fi
    # fi
    print('      state: %(state)s    name: \"%(name)s\"' % job)
    print('      src: \"%s\"' % _jobURL(job['source']))
    print('      dst: \"%s\"' % _jobURL(job['destination']))
    print('      message: %s' % mess)
    return
# End of _printJob
#-----------------------------------------------------------------------------
def JobList(authentication, base_url, vargs):
    if not vargs:
        # If no argument, print out all job ID's and their names.
        url = base_url + 'datamovement/jobs'
        (rt, r) = send_get(authentication, base_url, 'datamovement/jobs')
        if not rt:
            return False
        # fi
        results = r.json()
        if args.brief:
            if results['jobs'] != []:
                for job in results['jobs']:
                    print('%(id)4d' % job)
                # rof
            # fi
            return True
        # fi

        if results['jobs'] == []:
            print('No jobs present.')
        else:
            print('Jobs:')
            for job in results['jobs']:
                _printJob(job)
            # rof
        # fi
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
                    print('Job "%s"   id %s   projectid %s   state %s   status %s' % 
                             (jid, job['id'], job['projectid'], job['state'], job['status']))
                    found = True
                    break
                # fi
            # rof
            if not found:
                print('Could not get information on job name "%s"' % jid)
            # fi
            continue
        # fi
        # Must be a number if here
        (rt, r) = send_get(authentication, base_url, 'datamovement/jobs/%s' % jid)
        if not rt:
            continue
        # fi
        results = r.json()
        if results['status'] == 404:
            print('No job {0} present.'.format(jid))
        else:
            ret = True
            print('Job %s:' % jid)
            print(yaml.dump(results, default_flow_style=False))
        # fi
    # rof
    return ret
# End of JobList
#-----------------------------------------------------------------------------
def JobProjList(authentication, base_url, vargs):
    if not vargs:
        print('No project ID present.')
        return False
    # fi
    if len(vargs) != 1:
        print('Only one project ID allowed.')
        return False
    # fi

    projid = vargs[0]
    if not projid:
        print('No project ID present.')
        return False
    # fi
    if not projid.isdigit():
        (rt, projlist) = send_get(authentication, base_url, 'datamovement/projects')
        if not rt:
            print('No projects exist.')
            return False
        # fi
        found = False
        for project in projlist.json()['projects']:
            if project['name'] == projid:
                print('Project "%s" id %s' % (projid, project['id']))
                found = True
                projid = project['id']
                break
            # fi
        # rof
        if not found:
            print('Could not get information on project name "%s"' % id)
            return False
        # fi
    else:
        projid = int(projid)
    # fi

    # A project id number if here.
    (rt, r) = send_get(authentication, base_url, 'datamovement/projects/%s' % projid)
    if not rt:
        if vargs[0] != projid:
            print('Could not get information on project name "%s" giving id "%s"' % (vargs[0], projid))
        else:
            print('Could not get information on project id "%s"' % projid)
        # fi
        return False
    # fi

    # We know project projid exists.
    ret = False

    # Get all the jobs.
    url = base_url + 'datamovement/jobs'
    (rt, r) = send_get(authentication, base_url, 'datamovement/jobs')
    if not rt:
        return False
    # fi
    joblist = r.json()
    if joblist['jobs'] == []:
        print('No jobs present.')
    else:
        print('Jobs:')
        for job in joblist['jobs']:
            if job['projectid'] == projid:
                ret = True
                _printJob(job)
            # fi
        # rof
    # fi
    if not ret:
        print('No jobs for project ID="%s" present.' % projid)
        return False
    # fi
    return True
# End of JobProjList
#-----------------------------------------------------------------------------
def JobDele(authentication, base_url, vargs):
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
                print('Could not get information on job name "%s"' % id)
                continue
            # fi
        # fi
        # If name, "id" changed.
        (rt, r) = send_delete(base_url, authentication, 'datamovement/jobs/%s' % id)
        if not rt:
            continue
        # fi
        print('Job Dele', id)
        ret = True
    # rof
    return ret
# End of JobDele
#-----------------------------------------------------------------------------
def JobRun(authentication, base_url, vargs):
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
                print('Could not get information on job name "%s"' % id)
                continue
            # fi
        # fi
        # If name, "id" changed.
        (rt, r) = send_post(base_url, authentication, 'datamovement/jobs/%s/start?verify=false' % id, 202)
        if not rt:
            continue
        # fi
        print('Job Run', id)
        ret = True
    # rof
    return ret
# End of JobRun
#-----------------------------------------------------------------------------
def JobVerify(authentication, base_url, vargs):
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
                print('Could not get information on job name "%s"' % id)
                continue
            # fi
        # fi
        # If name, "id" changed.
        (rt, r) = send_post(base_url, authentication, 'datamovement/jobs/%s/start?verify=true' % id, 202)
        if not rt:
            continue
        # fi
        print('Job Verify', id)
        ret = True
    # rof
    return ret
# End of JobVerify
#-----------------------------------------------------------------------------
def JobStop(authentication, base_url, vargs):
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
                print('Could not get information on job name "%s"' % id)
                continue
            # fi
        # fi
        # If name, "id" changed.
        (rt,r) = send_post(base_url, authentication, 'datamovement/jobs/%s/stop' % id, 202)
        if not rt:
            continue
        # fi
        print('Job Stop', id)
        ret = True
    # rof
    return ret
# End of JobStop
#-----------------------------------------------------------------------------
# NOTDONEYET -- not tested, cleaned up, etc.
def JobEdit(authentication, base_url, vargs):
    ''' returns true if good command. '''
    if len(vargs) < 2:
        print('No edit values')
        return False
    # fi

    # Need check vargs is correct format.  NOTDONEYET

    id = None
    data = None
    etag = None
    for edit in vargs:
        if id == None:
            id = edit
            (rt, r) = send_get(authentication, base_url, 'datamovement/jobs/%s' % id)
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
        url = base_url + 'datamovement/jobs/%s' % id
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
def JobDisable(authentication, base_url, t_args):
    print("Job Disable is not written.")
    return False
# End of JobDisable
#-----------------------------------------------------------------------------
def JobEnable(authentication, base_url, t_args):
    print("Job Enable is not written.")
    return False
# End of JobEnable
#-----------------------------------------------------------------------------
# Second argument is 'list', 'dele', 'run', 'verify', 'stop', 'start', 'disable', 'enable', or 'stat'.
# NOTDONEYET - disable, enable, stat

def process_job(subtype, t_args, authentication, base_url):
    # Create or New mean the same thing.
    list_job_create = ['c', 'cr', 'cre', 'crea', 'creat', 'create', 'n', 'ne', 'new']
    list_job_dele   = ['de', 'del', 'dele', 'delet', 'delete']
    list_job_edit   = ['ed', 'edi', 'edit']
    list_job_list   = ['l', 'li', 'lis', 'list']
    list_job_proj   = ['p', 'pr', 'pro', 'proj', 'proje', 'projec', 'project', 'projects']
    list_job_run    = ['r', 'ru', 'run']
    list_job_stop   = ['s', 'st', 'sto', 'stop']
    list_job_verify = ['v', 've', 'ver', 'veri', 'verif', 'verify']
    list_job_disable= ['di', 'dis', 'disa', 'disab', 'disabl', 'disable']
    list_job_enable = ['en', 'ena', 'enab', 'enabl', 'enable']

    ret = False
    if subtype is None:                 # No subtype, print out jobs.
        ret = JobList(authentication, base_url, t_args)
    elif subtype in list_job_list:
        ret = JobList(authentication, base_url, t_args)
    elif subtype in list_job_proj:
        ret = JobProjList(authentication, base_url, t_args)

    elif subtype in list_job_create:
        ret = JobCreate(authentication, base_url, t_args)
    elif subtype in list_job_dele:
        ret = JobDele(authentication, base_url, t_args)
    elif subtype in list_job_edit:
        ret = JobEdit(authentication, base_url, t_args)
    elif subtype in list_job_run:
        ret = JobRun(authentication, base_url, t_args)
    elif subtype in list_job_stop:
        ret = JobStop(authentication, base_url, t_args)
    elif subtype in list_job_verify:
        ret = JobVerify(authentication, base_url, t_args)
    elif subtype in list_job_disable:
        ret = JobDisable(authentication, base_url, t_args)
    elif subtype in list_job_enable:
        ret = JobEnable(authentication, base_url, t_args)
    else:
        print("No job with subtype", subtype, "t_args =", t_args)
        ret = False
    # fi
    return ret
# End of process_job
#-----------------------------------------------------------------------------
def ProjCreate(authentication, base_url, vargs):
    if not vargs or not vargs or not vargs[0]:
        print("Must have 3 arguments to create/new:")
        print("   projectname [ sourceSMBvers [ destinationSMBvers ] ]")
        return False
    # fi

    if len(vargs) == 1:
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
        print("Must have 3 arguments to create/new (not %s):" % len(vargs))
        print("   projectname [ sourceSMBvers [ destinationSMBvers ] ]")
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
    print('Job Created', r.json()['id'])
    return True
# End of ProjCreate
#-----------------------------------------------------------------------------
def ProjDele(authentication, base_url, vargs):
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
                    print("Error: argument is not a number or project name '%s'" % id)
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
                print("Could not get information for project name %s" % id)
                continue
            # fi
        # fi

        # A project id number if here.
        (rt, r) = send_delete(base_url, authentication, 'datamovement/projects/%s' % id)
        if not rt:
            continue
        # fi
        ret = True
        print('Project %s deleted.' % id)
    # rof
    return ret
# End of ProjDele
#-----------------------------------------------------------------------------
# Second argument is 'list', 'create', 'dele'
# NOTDONEYET - 'run', 'verify', 'stop', 'start', 'disable', 'enable', or 'stat'.

def process_proj(subtype, t_args, authentication, base_url):
    # Create or New mean the same thing.
    list_proj_create = ['c', 'cr', 'cre', 'crea', 'creat', 'create', 'n', 'ne', 'new']
    list_proj_dele   = ['d', 'de', 'del', 'dele', 'delet', 'delete']
    list_proj_list   = ['l', 'li', 'lis', 'list']

    ret = False
    if subtype is None:                 # No subtype, print out projects.
        ret = ProjList(authentication, base_url, t_args, True)
    elif subtype in list_proj_list:
        ret = ProjList(authentication, base_url, t_args, True)

    elif subtype in list_proj_create:
        ret = ProjCreate(authentication, base_url, t_args)
    elif subtype in list_proj_dele:
        ret = ProjDele(authentication, base_url, t_args)
    else:
        print("No project with subtype", subtype, "t_args =", t_args)
        ret = False
    # fi
    return ret
# End of process_proj
#-----------------------------------------------------------------------------
# Parse and process line.
def process_line(t, authentication, base_url):

    if len(t) == 0:
        command = subtype = t_args = None
    elif len(t) == 1:
        command = t[0]
        subtype = t_args = None
    elif len(t) == 2:
        command = t[0]
        subtype = t[1]
        t_args = None
    else:
        command = t[0]
        subtype = t[1]
        t_args = t[2:]
    # fi

    # Try to process command.
    if command is None:                 # Ignore nothing given.
        ret = False
        pass
    elif command in list_proj:
        ret = process_proj(subtype, t_args, authentication, base_url)
    elif command in list_job:
        ret = process_job(subtype, t_args, authentication, base_url)
    elif command in list_exit:
        ret = Exit(subtype)
    elif command in list_help:
        ret = Help()
    elif command in list_history:
        ret = History()
    elif command in list_sleep:
        ret = Sleep(subtype)
    elif command in list_brief:
        ret = Brief(subtype)
    else:
        print("Unrecognized command, or not unique", t)
        Help()
        ret = False
    # fi

    return(ret)
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
                    print("Parsing error in line '%s'" % line)
                    print("    ", str(ex))
                    continue
                except:
                    print("Parsing error in line '%s'" % line)
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
# End of main
#-----------------------------------------------------------------------------
if __name__ == '__main__':
    main()
#-----------------------------------------------------------------------------
exit(0)
#-----------------------------------------------------------------------------
# End of file m4.py
