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

tab_words = ['projects ', 'jobs ', 'quit', 'exit', 'help', 'history', 'sleep']
#-----------------------------------------------------------------------------
class dnsCompleter:
    def __init__(self, options):
        self.options = options
        self.current_candidates = []
        return

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
    parser.add_argument('rest', nargs='*',
                        help='Optional command to execute')
    args = parser.parse_args()

    if not args.parsec or not args.user or not args.passwd:
        print('Need --parsec and --user and --passwd')
        exit(1)
    return
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
#-----------------------------------------------------------------------------
def _httpMap(code):
    if isinstance(code, str):
        code = int(code)
    return _httpResponseMap.get(code, 'UNKNOWN')
#-----------------------------------------------------------------------------
# Print error from 'get/post' request and return.
def print_get_error(str, response):
    print(str)
    print(json.dumps(response.json(), indent=4))
    print('Exiting with status 1')
    return
#-----------------------------------------------------------------------------
# Print error from URL 'get/post' request, and exit.
def print_get_error_exit(str, response):
    print_get_error(str, response)
    exit(1)
#-----------------------------------------------------------------------------
# Called initially.
def Login(ipaddr, version, user, pw):
    # Get Authentication Authorization Bearer.
    BASEURL = 'https://%s%s/' % (ipaddr, version)
    url = BASEURL + 'auth'
    r = requests.get(url, auth=(user, pw), verify=False)
    if r.status_code != 200:
        print_get_error_exit('Could not login to device! Return response:', r)
    return({'Authorization':'Bearer %s' % r.text}, BASEURL)
#-----------------------------------------------------------------------------
def send_get(authentication, base_url, tackon):
    r = requests.get(base_url + tackon, headers=authentication, verify=False)
    if r.status_code != 200:
        print_get_error("Request for %s failed. Response:" % tackon, r)
        return (False, None)
    return(True, r)
#-----------------------------------------------------------------------------
def send_post(base_url, authentication, str, goodvalue):
    global pp
    r = requests.post(base_url + str, headers=authentication, verify=False)
    if r.status_code != goodvalue:
        print('%3d : %s' % (r.status_code, _httpMap(r.status_code)))
        results = r.json()
        if results and 'detail' in results:
            print(results['detail'])
        pp.pprint(results)
        return (False, r)
    return (True, r)
#-----------------------------------------------------------------------------
def send_post_json(base_url, authentication, str, info, goodvalue):
    global pp
    r = requests.post(base_url + str, headers=authentication, verify=False, json=info)
    if r.status_code != goodvalue:
        print('%3d : %s' % (r.status_code, _httpMap(r.status_code)))
        results = r.json()
        if results and 'detail' in results:
            print(results['detail'])
        pp.pprint(results)
        return (False, r)
    return (True, r)
#-----------------------------------------------------------------------------
def send_delete(base_url, authentication, str):
    global pp
    r = requests.delete(base_url + str, headers=authentication, verify=False)
    if r.status_code != 204:
        print('%3d : %s' % (r.status_code, _httpMap(r.status_code)))
        results = r.json()
        if results and 'detail' in results:
            print(results['detail'])
        pp.pprint(results)
        return (False, r)
    return (True, r)
#-----------------------------------------------------------------------------
# Called when program starts. This sets projMap dictionary.
def ProjList(authentication, base_url, vargs, display):
    global projMap

    if not vargs:
        # If no argument, print out all project ID's and their names.
        (ret, projlist) = send_get(authentication, base_url, 'datamovement/projects')
        if not ret:
            return False
        results = projlist.json()
        if results['projects'] == []:
            if display:
                print('No projects present.')
            return True
        if display:
            print('Projects:')
        for proj in results['projects']:
            projMap[proj['id']] = proj['name']
            if display:
                print('%(id)4d: message: %(message)s    name \"%(name)s\"' % proj)
        return True

    # Possible multiple arguments with names/numbers.
    ret = False
    for id in vargs:
        if not id:
            continue
        if not id.isdigit():
            (rt, projlist) = send_get(authentication, base_url, 'datamovement/projects')
            if not rt:
                continue
            found = False
            for project in projlist.json()['projects']:
                if project['name'] == id:
                    print('Project "%s" id %s' % (id, project['id']))
                    found = True
                    break
            if not found:
                print('Could not get information on project name "%s"' % id)
            continue

        # A project id number if here.
        (rt, r) = send_get(authentication, base_url, 'datamovement/projects/%s' % id)
        if not rt:
            continue
        ret = True
        results = r.json()
        print('Project "%s":' % id)
        print(yaml.dump(results, default_flow_style=False))
    return ret
#-----------------------------------------------------------------------------
def Help():
    print(__doc__)
    return True
#-----------------------------------------------------------------------------
def Exit(subtype):
    if subtype is None or subtype == '':
        exit(0)
    if not subtype.isdigit():
        print("Error: argument is not a number '%s'" % subtype)
        return False
    exit(int(subtype))
#-----------------------------------------------------------------------------
def Sleep(subtype):
    if subtype is not None and subtype != '':
        if not subtype.isdigit():
            print("Error: argument is not a number '%s'" % subtype)
            return False
        time.sleep(int(subtype))
    return True
#-----------------------------------------------------------------------------
def History():
    global history

    cnt = 1
    for cmd in history:
        print(' %4d: %s' % (cnt, cmd))
        cnt += 1
    return True
#-----------------------------------------------------------------------------
# Called from JobCreate.
def _parseJobURI(uri):
    global username, password
    # NFS
    m = re.match('^nfs://([^/]+)/(.*)$', uri)
    if m:
        return (True, {'type':'NFS', 'host':m.group(1), 'export':'/'+m.group(2)})

    # SMB/CIFS
    m = re.match('^cifs://([^/]+)/(.*)$', uri)
    if not m:
        m = re.match('^smb://([^/]+)/(.*)$', uri)
    if m:
        return (True, {'type':'SMB', 'host':m.group(1), 'share':m.group(2),
                 'username':username, 'password':password})

    # BLOCK - FC or iSCSI
    m = re.match('^block://(\w+)/*$', uri)
    if m:
        return (True, {'type':'BLOCK', 'serialnumber':m.group(1)})

    # Replication
    m = re.match('^parsec://([^/]+)/*$', uri)
    if m:
        return (True, {'type':'REPLICATION', 'host':m.group(1), 'remoteverifier':None})

    print("Unknown URI '%s'" % uri)
    return (False,dict())
#-----------------------------------------------------------------------------
def JobCreate(authentication, base_url, vargs):
    global _jobId

    if not vargs or not vargs or not vargs[0]:
        print("Must have 4 arguments to create/new:")
        print("   projectId, source, destination, JobName")
        return False

    if len(vargs) != 4:
        print("Must have 4 arguments to create/new (not %s):" % len(vargs))
        print("   projectId, source, destination, JobName")
        return False

    (rt, s) = _parseJobURI(vargs[1])
    if not rt:
        print("Second argument must be URI for source. nfs://127.0.0.1/vol")
        return False
    (rt, d) = _parseJobURI(vargs[2])
    if not rt:
        print("Third argument must be URI for Destination. smb://127.0.0.1/share")
        return False

    id = vargs[0]
    if not id.isdigit():
        # A project name if here.
        (ret, projlist) = send_get(authentication, base_url, 'datamovement/projects')
        if not ret:
            print("Error: First argument is not a number or project name '%s'" % id)
            return False
        found = False
        for project in projlist.json()['projects']:
            if project['name'] == id:
                id = project['id']
                found = True
                break
        if not found:
            print("Could not get information for project name %s" % id)
            return False

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

    results = r.json()
    if results and 'id' in results:
        print('Job ID: %(id)s' % results)
    if results and 'detail' in results:
        print(results['detail'])
    return True
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
    return '%(type)s://???' % job
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
    print('      state: %(state)s    name: \"%(name)s\"' % job)
    print('      src: \"%s\"' % _jobURL(job['source']))
    print('      dst: \"%s\"' % _jobURL(job['destination']))
    print('      message: %s' % mess)
    return
#-----------------------------------------------------------------------------
def JobList(authentication, base_url, vargs):
    if not vargs:
        # If no argument, print out all job ID's and their names.
        url = base_url + 'datamovement/jobs'
        (rt, r) = send_get(authentication, base_url, 'datamovement/jobs')
        if not rt:
            return False
        results = r.json()
        if results['jobs'] == []:
            print('No jobs present.')
        else:
            print('Jobs:')
            for job in results['jobs']:
                _printJob(job)
        return True

    ret = False
    for jid in vargs:
        if not jid.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == jid:
                    print('Job "%s"   id %s   projectid %s   state %s   status %s' % 
                             (jid, job['id'], job['projectid'], job['state'], job['status']))
                    found = True
                    break
            if not found:
                print('Could not get information on job name "%s"' % jid)
            continue
        # Must be a number if here
        (rt, r) = send_get(authentication, base_url, 'datamovement/jobs/%s' % jid)
        if not rt:
            continue
        results = r.json()
        if results['status'] == 404:
            print('No job {0} present.'.format(jid))
        else:
            ret = True
            print('Job %s:' % jid)
            print(yaml.dump(results, default_flow_style=False))
    return ret
#-----------------------------------------------------------------------------
def JobProjList(authentication, base_url, vargs):
    if not vargs:
        print('No project ID present.')
        return False
    if len(vargs) != 1:
        print('Only one project ID allowed.')
        return False

    projid = vargs[0]
    if not projid:
        print('No project ID present.')
        return False
    if not projid.isdigit():
        (rt, projlist) = send_get(authentication, base_url, 'datamovement/projects')
        if not rt:
            print('No projects exist.')
            return False
        found = False
        for project in projlist.json()['projects']:
            if project['name'] == projid:
                print('Project "%s" id %s' % (projid, project['id']))
                found = True
                projid = project['id']
                break
        if not found:
            print('Could not get information on project name "%s"' % id)
            return False
    else:
        projid = int(projid)

    # A project id number if here.
    (rt, r) = send_get(authentication, base_url, 'datamovement/projects/%s' % projid)
    if not rt:
        if vargs[0] != projid:
            print('Could not get information on project name "%s" giving id "%s"' % (vargs[0], projid))
        else:
            print('Could not get information on project id "%s"' % projid)
        return False

    # We know project projid exists.
    ret = False

    # Get all the jobs.
    url = base_url + 'datamovement/jobs'
    (rt, r) = send_get(authentication, base_url, 'datamovement/jobs')
    if not rt:
        return False
    joblist = r.json()
    if joblist['jobs'] == []:
        print('No jobs present.')
    else:
        print('Jobs:')
        for job in joblist['jobs']:
            if job['projectid'] == projid:
                ret = True
                _printJob(job)
    if not ret:
        print('No jobs for project ID="%s" present.' % projid)
        return False
    return True
#-----------------------------------------------------------------------------
def JobDele(authentication, base_url, vargs):
    ret = False
    for id in vargs:
        if not id.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == id:
                    id =  job['id']
                    found = True
                    break
            if not found:
                print('Could not get information on job name "%s"' % id)
                continue
        # If name, "id" changed.
        (rt, r) = send_delete(base_url, authentication, 'datamovement/jobs/%s' % id)
        if not rt:
            continue
        print('Job Dele', id)
        ret = True
    return ret
#-----------------------------------------------------------------------------
def JobRun(authentication, base_url, vargs):
    ret = False
    for id in vargs:
        if not id.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == id:
                    id =  job['id']
                    found = True
                    break
            if not found:
                print('Could not get information on job name "%s"' % id)
                continue
        # If name, "id" changed.
        (rt, r) = send_post(base_url, authentication, 'datamovement/jobs/%s/start?verify=false' % id, 202)
        if not rt:
            continue
        print('Job Run', id)
        ret = True
    return ret
#-----------------------------------------------------------------------------
def JobVerify(authentication, base_url, vargs):
    ret = False
    for id in vargs:
        if not id.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == id:
                    id =  job['id']
                    found = True
                    break
            if not found:
                print('Could not get information on job name "%s"' % id)
                continue
        # If name, "id" changed.
        (rt, r) = send_post(base_url, authentication, 'datamovement/jobs/%s/start?verify=true' % id, 202)
        if not rt:
            continue
        print('Job Verify', id)
        ret = True
    return ret
#-----------------------------------------------------------------------------
def JobStop(authentication, base_url, vargs):
    ret = False
    for id in vargs:
        if not id.isdigit():
            # If not a number, then a name.
            (rt, joblist) = send_get(authentication, base_url, 'datamovement/jobs')
            if not rt:
                continue
            found = False
            for job in joblist.json()['jobs']:
                if job['name'] == id:
                    id =  job['id']
                    found = True
                    break
            if not found:
                print('Could not get information on job name "%s"' % id)
                continue
        # If name, "id" changed.
        (rt,r) = send_post(base_url, authentication, 'datamovement/jobs/%s/stop' % id, 202)
        if not rt:
            continue
        print('Job Stop', id)
        ret = True
    return ret
#-----------------------------------------------------------------------------
# NOTDONEYET -- not tested, cleaned up, etc.
def JobEdit(authentication, base_url, vargs):
    ''' returns true if good command. '''
    if len(vargs) < 2:
        print('No edit values')
        return False

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
        fields = edit.split('=', 1)
        if fields[0] == 'src.remoteverifier':
            data['source']['remoteverifier'] = fields[1]
            del data['source']['localverifier']
        elif fields[0] == 'dst.remoteverifier':
            data['destination']['remoteverifier'] = fields[1]
            del data['destination']['localverifier']
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
    return False
#-----------------------------------------------------------------------------
def JobDisable(authentication, base_url, t_args):
    print("Job Disable is not written.")
    return False
#-----------------------------------------------------------------------------
def JobEnable(authentication, base_url, t_args):
    print("Job Enable is not written.")
    return False
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
    return ret
#-----------------------------------------------------------------------------
def ProjCreate(authentication, base_url, vargs):
    if not vargs or not vargs or not vargs[0]:
        print("Must have 3 arguments to create/new:")
        print("   projectname [ sourceSMBvers [ destinationSMBvers ] ]")
        return False

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

    if srcvers and srcvers == "default":
        srcvers = None
    if dstvers and dstvers == "default":
        dstvers = None

    # Create project with projname.
    info = {'name':projname,
            'defaultmigjobrate':PROJECT_JOBRATE,
            'jobrunlimit':PROJECT_RUNLIMIT,
            'source': {'smbversion': srcvers},
            'destination': {'smbversion': dstvers}}

    (rt, r) = send_post_json(base_url, authentication, 'datamovement/projects', info, 200)
    if not rt:
        return False
    print('Job Created', r.json()['id'])
    return True
#-----------------------------------------------------------------------------
def ProjDele(authentication, base_url, vargs):
    ret = False
    projlist = None
    for id in vargs:
        if not id:
            continue
        if not id.isdigit():
            # A project name if here.
            if projlist is None:
                (ret, projlist) = send_get(authentication, base_url, 'datamovement/projects')
                if not ret:
                    print("Error: argument is not a number or project name '%s'" % id)
                    continue
            found = False
            for project in projlist.json()['projects']:
                if project['name'] == id:
                    id = project['id']
                    found = True
                    break
            if not found:
                print("Could not get information for project name %s" % id)
                continue

        # A project id number if here.
        (rt, r) = send_delete(base_url, authentication, 'datamovement/projects/%s' % id)
        if not rt:
            continue
        ret = True
        print('Project %s deleted.' % id)
    return ret
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
    return ret
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
    else:
        print("Unrecognized command, or not unique", t)
        Help()
        ret = False

    return(ret)
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
#-----------------------------------------------------------------------------
if __name__ == '__main__':
    main()
#-----------------------------------------------------------------------------
exit(0)
#-----------------------------------------------------------------------------
# End of file m4.py
