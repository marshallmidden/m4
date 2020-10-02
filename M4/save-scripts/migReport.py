#!/usr/bin/env python3
# vim: noai ts=4 sts=4 et sw=4 ft=python
'''
Create a Migration Report for MKII migration jobs

Example:
    %(prog)s report --migration 12          # Create a migation report for migation job # 12
'''

def main(values):
    import argparse
    import datetime
    import json
    import os
    import pprint
    import sys
    import traceback

    import logging
    sys.path.append('/px/bin')
    from pxlogging import PxLogger
    PxLogger.setloggerinfo('com.parseclabs.misc', '/px/log/misc.log')
    logger = PxLogger.getlogger()

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='')

    ##
    ## Set up arguments and reasonable defaults
    ##
    parser.add_argument('op', choices=['report'], nargs='?', default='report')
    parser.add_argument('--migration', '-m',  type=int, default=None)
    parser.add_argument('--page', '-p',  type=int, default=15)
    parser.add_argument('--run', '-r',  type=int, default=None)
    parser.add_argument('--database', '-d',  type=str, default='/px/pw/db/development.sqlite3')
    parser.add_argument('--log', '-L',  type=str, default=None)
    parser.add_argument("--debug", dest='logLevel', const=logging.DEBUG, default=logging.WARN, action='store_const', help='set logging level')
    parser.add_argument("--info", dest='logLevel', const=logging.INFO, action='store_const', help='set logging level')
    parser.add_argument("--warn", dest='logLevel', const=logging.WARN, action='store_const', help='set logging level')
    parser.add_argument("--error", dest='logLevel', const=logging.ERROR, action='store_const', help='set logging level')
    parser.add_argument('--csv', action='store_true',
                            help = 'If wish job log and error log in CSV (Comma Separated Value) format.')
    parser.add_argument('--json', action='store_true',
                            help = 'If wish job log and error log in JSON format.')

    args = parser.parse_args(values)

    logger.setLevel(args.logLevel)
    pp = pprint.PrettyPrinter(indent=4)

    import sqlite3
    from sqlite3 import Error

    # Main program continues after several routines are defined.
    #-----------------------------------------------------------------------------
    _aMap = {
        'LS_UPGRADE': 'Upgrade',
        'SetTime': 'Set Clock',
        'DISTANCE': 'Distance',
    }

    def actionMap(name) :
        return _aMap.get(name, name)
    # End of actionMap
    #-----------------------------------------------------------------------------
    _sMap = {
        'SUCCESS': 'Success',
        'CANCELED': 'Canceled',
        'CANCELLED': 'Canceled',
        'SUSPENDED': 'Suspend',
        'FAILURE': 'Failure',
        'VERIFIED': 'Verified',
        'VERIFYING': 'Verifying',
        'NOT RUN': 'NotRun',
        'INCOMPLETE': 'Incomplete',
        'FINALIZED': 'Finalized',
        'OPERATORSTOP': 'Stop',
    }
    def statusMap(name) :
        return _sMap.get(name, name)

    # End of statusMap
    #-----------------------------------------------------------------------------
    #
    #   returns an iterator of migration IDs.
    #   If the input is defined, then just that migration, 
    #   if None, then we look up all of the migrations IDs
    #
    def getMigrations(migration) :

        if migration :
            yield migration

        else :
            cursor = connect.cursor()
            cursor.execute('SELECT id FROM job limit 1')
            for row in cursor.fetchall() :
                yield row[0]
    # End of getMigrations
    #-----------------------------------------------------------------------------
    def getRemote(id) :
        '''Get information about the remote device from it's id'''

        sql = '''SELECT systemid, ipaddr, ipport
                   FROM remotepxn
                  WHERE id = ? 
                LIMIT 1
            '''
        cursor = connect.cursor()
        cursor.execute(sql, (id,))
        row = cursor.fetchone()
        if not row : row = ('?', '?', '?')
        return {
            'system' : row[0],
            'ip' : row[1],
            'port' : row[2],
        }
    # End of getRemote
    #-----------------------------------------------------------------------------
    def getCustomLog(data, run=None) :
        '''Look through the custlog information for a specific run instance'''
        fldb = None
        custlog = None
        cursor = connect.cursor()
        cursor.execute('SELECT datetime(timestamp,"localtime"), msglvl, message FROM custlog WHERE jobrhid = %(jobrhid)s' % data)  # legacy, try the system db
        rows = cursor.fetchall()
        ## If we didn't find any data here, look into the bowels of the MKII engine
        if not rows:
            try:
                dbFile = "/media/parsecdata/lhr/%(id)s/fl_%(id)s_%(jobrhid)s.db" % data
                if not os.path.exists(dbFile) :
                    raise OSError("No FDR data")
                fldb = sqlite3.connect(dbFile)
                cursor = fldb.cursor()
                cursor.execute('SELECT datetime(timestamp,"localtime"), lvl, msg FROM custlog') # try the job's fldb if not
                rows = cursor.fetchall()
            except sqlite3.OperationalError as e:
                logger.info(str(e))
            except OSError as e:
                logger.info(str(e))
            except Exception as e:
                logger.info(str(e))
        custlog = []
        for row in rows :
            level = '    '
            if 'Info' in row[1] : level = 'INFO  '
            if 'Warn' in row[1] : level = 'WARN !'
            if 'Error' in row[1] : level ='ERR  *'
            if args.csv:
                custlog.append( (row[0], level, row[2]) )
            elif args.json:
                custlog.append( ( { 'date':row[0], 'level':level, 'message':row[2] } ) )
            else:
                custlog.append( "%s %s %s" % (row[0], level, row[2]) )

        data['cust'] = custlog
        if fldb:
            fldb.close()
    # End of getCustomLog
    #-----------------------------------------------------------------------------
    def getErrLog(data, run=None) :
        fldb= None
        if args.csv:
            report = [ ('No FDR data found', '', '')]
        elif args.json:
            report = [ ( { 'message':'No FDR data found', 'level':'', 'date':'' } )]
        else:
            report = ['   No FDR data found']

        try:
            dbFile = "/media/parsecdata/lhr/%(id)s/fl_%(id)s_%(jobrhid)s.db" % data
            if not os.path.exists(dbFile) :
                raise OSError("No FDR data job=%(id)s run=%(jobrhid)s" % data )
            fldb = sqlite3.connect(dbFile)
            cursor = fldb.cursor()

            ## Make a cache of the files in this run  array of (dir, name)
            fileInfo = []
            cursor.execute('SELECT id, dirnid, name FROM node ORDER BY id')
            for row in cursor.fetchall() :
                fileInfo.append( (int(row[1]), row[2]) )

            def pathName( nid ) :
                pn = []
                while nid > 0 :
                    fi = fileInfo[nid]
                    # pn.append( fi[1] )
                    pn.insert( 0,  fi[1] )
                    nid = fi[0]
                path = '/' + '/'.join(pn)
                return path

            ## Get log messages array of messages, indices begin at 1, not zero
            logMsg = []
            logMsg.append( "Not Used")
            cursor.execute('SELECT id, msg FROM msg1 ORDER BY id')
            for row in cursor.fetchall() :
                logMsg.append( row[1] )

            report = []
            lastMsgId = -1
            cursor.execute('SELECT datetime(timestamp,"localtime"), nid, lvl, msg1id, msg2 FROM errlog  ORDER BY msg1id, timestamp')
            for row in cursor.fetchall() :
                if not row[2]: continue
                mid = int( row[3] )
                if lastMsgId != mid :
                    lastMsgId = mid
                    if not args.csv and not args.json:
                        report.append( logMsg[mid] )
                msg2 = row[4] if row[4] else ''
                if args.csv:
                    report.append((row[0], row[2] + ' %s' % mid, pathName( int(row[1]) ) + ' ' + msg2 + "\n"+ logMsg[mid]))
                elif args.json:
                    report.append( ( { 'date':row[0], 'level':row[2] + ' %s' % mid, 'message':pathName( int(row[1]) ) + ' ' + msg2 + "\n"+ logMsg[mid] } ) )
                else:
                    report.append('   ' + row[0] + ' ' + pathName( int(row[1]) ) + ' ' + row[2] + ' ' + msg2)
                    
        except sqlite3.OperationalError as e:
            logger.info(str(e))
        except OSError as e:
            logger.info(str(e))
        except Exception as e:
            error = traceback.format_exc()
            logger.error(error)

        data['err'] = report
        if fldb:
            fldb.close()
    # End of getErrLog
    #-----------------------------------------------------------------------------
    def getHistory(id, op=None) :
        '''
        0 id INTEGER True None True 
        1 projectid integer True None False 
        2 jobid integer True None False 
        3 projrhid integer True None False 
        4 runreason varchar(255) False None False 
        5 runoptions varchar(255) False None False 
        6 status varchar(255) True None False 
        7 statusmessage varchar(255) False None False 
        8 detail varchar(255) False None False 
        9 errors integer False None False 
        10 warnings integer False None False 
        11 start datetime True None False 
        12 end datetime False None False 
        13 operation varchar(255) False None False 
        '''

        sql = '''SELECT operation, datetime(start,'localtime'), datetime(end,'localtime'), status, detail, id
                   FROM jobrh
                  WHERE jobid = ? 
                ORDER BY id DESC
            '''

        history = []
        cursor = connect.cursor()
        cursor.execute(sql, (id,))
        for row in cursor.fetchall() :
            begTime = datetime.datetime.strptime(row[1], "%Y-%m-%d %H:%M:%S")
            endTime = datetime.datetime.strptime(row[2] if row[2] else row[1], "%Y-%m-%d %H:%M:%S")
            data = json.loads(row[4]) if row[4] else dict()
            data['action'] = row[0] if row[0] != None else 'STDMIG'
            data['start'] = row[1]
            data['finish'] = row[2] if row[2] else ''
            data['status'] = row[3]
            data['elapsed'] = str(endTime - begTime)
            data['ielapsed'] = (endTime - begTime)
            data['jobrhid'] = row[5]

            if 'ftot' in data :
                data['files.total'] = data.get('ftot',0)
                data['files.new'] = data.get('fwrtot',0)
            if 'fsobj.files.new' in data :
                data['files.new'] = int(data.get('fsobj.dirs.new','0')) + int(data.get('fsobj.files.writtento','0')) + int(data.get('fsobj.others.new','0'))
                data['files.total'] = int(data.get('fsobj.dirs.considered','0')) + int(data.get('fsobj.files.considered','0')) + int(data.get('fsobj.others.considered','0'))


            data['bytes.total'] = data.get('btot',0)
            data['bytes.new'] = data.get('wbtot',0)
            if 'bytes.written' in data :
                data['bytes.new'] = data.get('bytes.written',0)
                data['bytes.total'] = data.get('bytes.included',0)

    #        print("XXX %(files.new)s of %(files.total)s  :: %(bytes.new)s of %(bytes.total)s" % data)

            data['id'] = id
            run = int(row[5])
            getCustomLog(data, run)
            getErrLog(data, run)

            history.append(data)

        job = dict()
        cursor = connect.cursor()
        sql = '''SELECT p.name, j.engine,
                        j.srcmode, j.srchostname, j.srcimportpath, j.srcaltrootdir, j.srcserialnum, j.srcremoteid,
                        j.dstmode, j.dsthostname, j.dstimportpath, j.dstaltrootdir, j.dstserialnum, j.dstremoteid,
                        p.id, j.label
                   FROM job j, project p
                  WHERE j.id = ?
                    AND p.id = j.projectid'''

        def mkEndPt(mode, host, path, root, serial, remote) :
            if mode == 'NFS' : return "NFS //%s%s" % (host, path)
            if mode == 'SMB' : return "SMB //%s%s" % (host, path)
            if mode == 'SCSI' : return "SCSI %s" % (serial)
            if mode == 'FC' : return "FC %s" % (serial)
            if mode == 'NVME' : return "NVME %s" % (serial)
            if mode == 'DISTANCE' :
                return "PARSEC %(system)s [%(ip)s:%(port)s]" % getRemote(remote)
            return mode
            
        cursor.execute(sql, (id,))
        row = cursor.fetchone()
        if not row  : return None
        # NOTE: row[1] ... j.engine is not used.
        if args.json:
            job['label'] = row[0]                       # project label
            source = dict()
            source['type'] = row[2]                 # srcmode
            source['host'] = row[3]                 # srchostname
            source['share'] = row[4].lstrip('/')    # srcimportpath delete leading /'s
            source['altrootdir'] = row[5]           # srcaltroot
            source['serialnumber'] = row[6]         # srcserialnum
            source['remoteid'] = row[7]             # srcremoteid
            job['source'] = source
            destination = dict()
            destination['type'] = row[8]            # dstmode
            destination['host'] = row[9]            # dsthostname
            destination['share'] = row[10].lstrip('/') # dstimportpath delete leading /'s
            destination['altrootdir'] = row[11]     # dstaltroot
            destination['serialnumber'] = row[12]   # dstserialnum
            destination['remoteid'] = row[13]       # dstremoteid
            job['destination'] = destination
            job['projectid'] = row[14]                  # projectid
            job['name'] = row[15]
        else:
            job['project'] = row[0]
            job['src'] = mkEndPt(row[2], row[3], row[4], row[5], row[6], row[7])
            job['dst'] = mkEndPt(row[8], row[9], row[10], row[11], row[12], row[13])
            job['projectId'] = row[14]
            job['label'] = row[15]

        # Get Appliance name
        cursor = connect.cursor()
        cursor.execute('SELECT value FROM dnvconfig WHERE domain = "systemsettings" AND name="systemname"')
        row = cursor.fetchone()
        if args.json:
            job['hostname'] = row[0]
        else:
            job['host'] = row[0]

        job['history'] = history
        job['id'] = id                                  # Both
        nonce = datetime.datetime.now()
        job['published'] = nonce.strftime("%Y-%m-%d %H:%M:%S")
        job['ts'] = nonce.strftime("%Y%m%d-%H%M")

        return job
    # End of getHistory
    #-----------------------------------------------------------------------------
    def printErrHtml(message) :
        import io

        fd = io.StringIO()
        # fd = sys.stdout
        fd.write("""<html>
<head>
<title>Error</title>
</head>

<body>
Error: %s
</body>
</html>""" % message)
        return fd.getvalue()
    # End of printErrHtml
    #-----------------------------------------------------------------------------
    # Format History as HTML
    def printAsHTML(info, page=15) :
        import io

        fd = io.StringIO()
    #    fd = open("mid%04d.html" % info['id']  , "w")
    #    fd = sys.stdout
        cnt = 0

        info['pageSize']  = page
        fd.write("""<html>
<head>
<title>Job Report %(id)d %(label)s</title>
<style>
BODY { font-family: arial; color: #555; }
H2 { background: #f4f6f8; color: #777; border: 0px solid #bbb; padding: 10px 5px 10px; font-weight: normal; }
TR { border: 1px solid #aaa; }
TH { background: #f9fafe; color: #888; border-color: #000; border: 1px solid aaa; font-weight: normal;}
TD { border: 1px solid #aaa; border-right: 1px solid #ccc; border-left: 1px solid #ccc; color: #000; }
a:link { text-decoration: none; }
a:hover { color: hotpink; }
PRE {margin: 0 0 0 0;}

.even { color: #000;}
.odd { color: #000; }
.rollup { background: #555; color: #fff; }


.bold { font-weight : bold; }
.done { color : #fff; background : #0f0; font-weight : bold; }
.working { color : #000; background : #cdddac; font-weight : bold; }
.reopen { color : #000; background : #fbcba2; }
.note { color : #000; background : #ffff00; }
.test { color : #000; background : #00ffff; }


.acVerify { background: #65a6ff; color: #fff; font-variant: small-caps; }
.acSTDMIG { background: #00bca4; color: #fff; font-variant: small-caps; }
.acCancel { background: #bc00a4; color: #fff; font-variant: small-caps; }
.acOperatorStop { background: #000; color: #fff; font-variant: small-caps; }
.acNone { background: #aa00aa; color: #fff; font-variant: small-caps; }
.acDistance { background: #a0ccb8; color: #555; font-variant: small-caps; }

.stSuccess { background: #2ecc71; color: #fff; font-variant: small-caps; }
.stSuspend { background: #9fa8b1; color: #fff; font-variant: small-caps; }
.stFailure { background: #e74c3c; color: #fff; font-variant: small-caps; }
.stFailed { background: #e74c3c; color: #fff; font-variant: small-caps; }
.stCanceled { background: #2d2d2d; color: #ccc; font-variant: small-caps; }
.stVerified { background: #3498db; color: #fff; font-variant: small-caps; }
.stVerifying { background: #65a6ff; color: #fff; font-variant: small-caps; }
.stOperatorStop { background: #000; color: #fff; font-variant: small-caps; }
.stRunning { background: #0c8b7b; color: #fff; font-variant: small-caps; }
.stNotRun { background: #FEFEFF; color: #555a60; font-variant: small-caps; }
.stIncomplete { background: #e67e22; color: #fff; font-variant: small-caps; }
.stFinalized { background: #41e5c0; color: #fff; font-variant: small-caps; }
.isClickable { cursor: pointer; padding-right: 5px; padding-left:5px;}
.isClickable:hover { background:  #f4f6f8; color: hotpink; }

a { cursor: pointer; padding-right: 5px; padding-left:5px;} a:hover { background: #f4f6f8;}
</style>

<script type='text/javascript'>

pageSize = %(pageSize)d;
page = 0;
totalRows = 0;
totalPages = 0;

function setRows(count) {
    totalRows = count;
    totalPages = Math.ceil(count / pageSize);
}

function paginate(incr) {
//    console.log("page(" + incr + ")");
    curp = page;
    newp = page + incr;
    if( newp < 0 ) newp = 0;
    if( newp >= totalPages ) newp = totalPages - 1;
//    console.log("page=" + page + "  cur=" + curp + "  new=" + newp );

    elem = document.getElementById('pgnum');
    if( elem != null ) elem.innerHTML = (newp + 1) + ' of ' +  totalPages;

    if( newp == page ) return;
    
    // Make the new page visible
    if( newp != page ) for(r= 0; r < pageSize; r++) {
        rowid = newp * pageSize + r;
//        console.log("show(" + rowid +")");
        base = 'r' + rowid;
        showRow(document.getElementById(base      ));
        hideRow(document.getElementById(base + 'j'));
        hideRow(document.getElementById(base + 'e'));
    }

    // Make the current page invisible
    for(r= 0; r < pageSize; r++) {
        rowid = curp * pageSize + r;
//        console.log("hide(" + rowid +")");
        base = 'r' + rowid;
        hideRow(document.getElementById(base      ));
        hideRow(document.getElementById(base + 'j'));
        hideRow(document.getElementById(base + 'e'));
    }
    page = newp;

}



function hideRow(id) { if( id != null) id.style.display = 'none'; }
function showRow(id) { if( id != null) id.style.display = ''; }

function opX(rowid) {
    hideRow(document.getElementById(rowid + 'j'));
    hideRow(document.getElementById(rowid + 'e'));
}
function opJ(rowid) {
    showRow(document.getElementById(rowid + 'j'));
    hideRow(document.getElementById(rowid + 'e'));
}
function opE(rowid) {
    hideRow(document.getElementById(rowid + 'j'));
    showRow(document.getElementById(rowid + 'e'));
}

function allX() {
    for(r = 0; x = document.getElementById('r' + r + 'j'); r++) { hideRow(x) }
    for(r = 0; x = document.getElementById('r' + r + 'e'); r++) { hideRow(x) }
}
function allJ() {
    for(r = 0; x = document.getElementById('r' + r + 'j'); r++) { showRow(x) }
    for(r = 0; x = document.getElementById('r' + r + 'e'); r++) { hideRow(x) }
}
function allE() {
    for(r = 0; x = document.getElementById('r' + r + 'j'); r++) { hideRow(x) }
    for(r = 0; x = document.getElementById('r' + r + 'e'); r++) { showRow(x) }
}
</script>

</head>

<body>
<h2 id='rtop'><img src='data:image/png;base64,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
hfX2dYm+VuqEbq+aROvlsz3HaS8qol3/LkUDCGU+fiW1Q5jY3/vEPV1cZXdRPg/0D48ZV17CfcpU
XCECc6IHYZyRMXjkN7Tv5EkTVB6oa5/gYJ6VILUrpxmShAxspiIYWm4NCQAiJTOszDzsL4dWA15X
2kFDJ3ctzjKUuzvtuWYrmKiKYDDG2GZtwl89Hgh6PIfBuB/pGW/fe+vfDSR9hOC3TZs42hkl/FAg
tM5eYTO8i5ioMz6qB8vqp+Tah6Qc2EttC3HHuBy8UqGC93iMFqeGdb/ms2eZCkZN37WnwhgAeRil
Ef43o0hNJiDHtQ2AzwhgixDB4/AdvTNr9YDxVrG127Tjj14ABMgnBoUY3XEIj6fClr7oOQU+T880
G+iaSDb3XTsxbYy4Q9yoSsVXVQrKSKFH0VdGh6lBVclfnPzbxE8LaSD6et00bfClwJAhN3vojocM
EYgyzk5q8gogD50fbIJr05Z0VIQBgIwqwY+PKTo1PUjBfDIZS/t/lYyd/qeHUNpywyIT4DOA1GMN
FLDL7e8Lnpee611udn/BV/xn1/O758kJsbpUBOqpC292T8jFhdw9QQbOn9A9A6edNthRT19u1i6k
WLwIdRgluAPKf88Ci55XXhnLY+DNFZh8czDhErlHAfWktaQrjoDsFzqXyRoYkz77kx8CiXXZ0uTL
E/GGXWquMVib0BSYgK4nhohyDJvFhoGScG14B+8q+jeTOS7qckgQwiYxs2QUvja6pievyC46GMMK
ypON8Y6ktP7+rYV2BwlrFL7DbTCXpwI44Ihl8rgUwEDUZNYujM6o4/mZ9iDZvNp8jQppijtDTpSQ
5smGUUS2th5Dk+oJd5zf9AD1oLQoRlpVW4342PeQQk+vqVvB2RFR3r8VOhMbxUn0T6AMZSYMlgVE
9WACJIfs9Fr/0urf/fu3E/14vCkG+OX5pp5uInL3px31XzfUnbClfzNzBu79vwEwR1lDNIUaGAAA
EOtpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVN
ME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+Cjx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0
YS8iIHg6eG1wdGs9IlhNUCBDb3JlIDQuNC4wLUV4aXYyIj4KIDxyZGY6UkRGIHhtbG5zOnJkZj0i
aHR0cDovL3d3dy53My5vcmcvMTk5OS8wMi8yMi1yZGYtc3ludGF4LW5zIyI+CiAgPHJkZjpEZXNj
cmlwdGlvbiByZGY6YWJvdXQ9IiIKICAgIHhtbG5zOmlwdGNFeHQ9Imh0dHA6Ly9pcHRjLm9yZy9z
dGQvSXB0YzR4bXBFeHQvMjAwOC0wMi0yOS8iCiAgICB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFk
b2JlLmNvbS94YXAvMS4wL21tLyIKICAgIHhtbG5zOnN0RXZ0PSJodHRwOi8vbnMuYWRvYmUuY29t
L3hhcC8xLjAvc1R5cGUvUmVzb3VyY2VFdmVudCMiCiAgICB4bWxuczpwbHVzPSJodHRwOi8vbnMu
dXNlcGx1cy5vcmcvbGRmL3htcC8xLjAvIgogICAgeG1sbnM6R0lNUD0iaHR0cDovL3d3dy5naW1w
Lm9yZy94bXAvIgogICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEv
IgogICAgeG1sbnM6ZXhpZj0iaHR0cDovL25zLmFkb2JlLmNvbS9leGlmLzEuMC8iCiAgICB4bWxu
czp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iCiAgIHhtcE1NOkRvY3VtZW50SUQ9
ImdpbXA6ZG9jaWQ6Z2ltcDo0YzMyOGEzZS0wZGEyLTQ1MzItODlkMS03Y2IwMDFiNjdmYTUiCiAg
IHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6OGEyZmExYWYtMmFjYi00OTI4LWFhMzgtZWY2OTI1
NmI3ZDBkIgogICB4bXBNTTpPcmlnaW5hbERvY3VtZW50SUQ9InhtcC5kaWQ6NjlhNTQxYTEtMGEy
Yi00ZGNmLWJlMDYtYzU0YmQzNWY5MmQ1IgogICBHSU1QOkFQST0iMi4wIgogICBHSU1QOlBsYXRm
b3JtPSJNYWMgT1MiCiAgIEdJTVA6VGltZVN0YW1wPSIxNTU4MDQ0ODEzNDI3Mjc3IgogICBHSU1Q
OlZlcnNpb249IjIuMTAuOCIKICAgZGM6Rm9ybWF0PSJpbWFnZS9wbmciCiAgIHhtcDpDcmVhdG9y
VG9vbD0iR0lNUCAyLjEwIj4KICAgPGlwdGNFeHQ6TG9jYXRpb25DcmVhdGVkPgogICAgPHJkZjpC
YWcvPgogICA8L2lwdGNFeHQ6TG9jYXRpb25DcmVhdGVkPgogICA8aXB0Y0V4dDpMb2NhdGlvblNo
b3duPgogICAgPHJkZjpCYWcvPgogICA8L2lwdGNFeHQ6TG9jYXRpb25TaG93bj4KICAgPGlwdGNF
eHQ6QXJ0d29ya09yT2JqZWN0PgogICAgPHJkZjpCYWcvPgogICA8L2lwdGNFeHQ6QXJ0d29ya09y
T2JqZWN0PgogICA8aXB0Y0V4dDpSZWdpc3RyeUlkPgogICAgPHJkZjpCYWcvPgogICA8L2lwdGNF
eHQ6UmVnaXN0cnlJZD4KICAgPHhtcE1NOkhpc3Rvcnk+CiAgICA8cmRmOlNlcT4KICAgICA8cmRm
OmxpCiAgICAgIHN0RXZ0OmFjdGlvbj0ic2F2ZWQiCiAgICAgIHN0RXZ0OmNoYW5nZWQ9Ii8iCiAg
ICAgIHN0RXZ0Omluc3RhbmNlSUQ9InhtcC5paWQ6ZTNmNjQ0ZGEtYTI0Mi00ZjUyLTgxZTUtMjRh
ZDgyY2QwOTFiIgogICAgICBzdEV2dDpzb2Z0d2FyZUFnZW50PSJHaW1wIDIuMTAgKE1hYyBPUyki
CiAgICAgIHN0RXZ0OndoZW49IjIwMTktMDUtMTVUMTY6MDY6MDAtMDU6MDAiLz4KICAgICA8cmRm
OmxpCiAgICAgIHN0RXZ0OmFjdGlvbj0ic2F2ZWQiCiAgICAgIHN0RXZ0OmNoYW5nZWQ9Ii8iCiAg
ICAgIHN0RXZ0Omluc3RhbmNlSUQ9InhtcC5paWQ6MDE3ZTVjZTItZDk2Yi00Y2VjLWJmN2QtZjBj
ZjM5MjE4ZWMzIgogICAgICBzdEV2dDpzb2Z0d2FyZUFnZW50PSJHaW1wIDIuMTAgKE1hYyBPUyki
CiAgICAgIHN0RXZ0OndoZW49IjIwMTktMDUtMTZUMTc6MTM6MzMtMDU6MDAiLz4KICAgIDwvcmRm
OlNlcT4KICAgPC94bXBNTTpIaXN0b3J5PgogICA8cGx1czpJbWFnZVN1cHBsaWVyPgogICAgPHJk
ZjpTZXEvPgogICA8L3BsdXM6SW1hZ2VTdXBwbGllcj4KICAgPHBsdXM6SW1hZ2VDcmVhdG9yPgog
ICAgPHJkZjpTZXEvPgogICA8L3BsdXM6SW1hZ2VDcmVhdG9yPgogICA8cGx1czpDb3B5cmlnaHRP
d25lcj4KICAgIDxyZGY6U2VxLz4KICAgPC9wbHVzOkNvcHlyaWdodE93bmVyPgogICA8cGx1czpM
aWNlbnNvcj4KICAgIDxyZGY6U2VxLz4KICAgPC9wbHVzOkxpY2Vuc29yPgogICA8ZXhpZjpVc2Vy
Q29tbWVudD4KICAgIDxyZGY6QWx0PgogICAgIDxyZGY6bGkgeG1sOmxhbmc9IngtZGVmYXVsdCI+
U2NyZWVuc2hvdDwvcmRmOmxpPgogICAgPC9yZGY6QWx0PgogICA8L2V4aWY6VXNlckNvbW1lbnQ+
CiAgPC9yZGY6RGVzY3JpcHRpb24+CiA8L3JkZjpSREY+CjwveDp4bXBtZXRhPgogICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAK
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg
ICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgCjw/eHBhY2tldCBlbmQ9
InciPz7IIUJyAAAABmJLR0QA9AD2APhG2DiuAAAACXBIWXMAAC4jAAAuIwF4pT92AAAAB3RJTUUH
4wUQFg0hJZGqsgAAEYpJREFUaN7tW3lYE+fWP+9MJhthC2FJDPu+iYKKUFxQ0eKCetVqtW7d3Nra
1mpr+/W79qndrvfWtmprra1LF3ettdSqLX4gLiCigCwiYASMgISQQLaZzLz3DwaLaaz0a731Pg/n
zzzvmfPO+Z33LL95A/AXC8ZYYKftLhhjBH3yh4X4q8HUNxkTDDpT6F+9lz5A/ziYyGKyBTRUNaV6
eMt0AMD1wfHHRfA7QSAAgGpps8huNHfKtc0dhERMQT8/Ga32db3lIhFYAYBFCOF7PYvjMHlqb3Fs
WJK/hiCJW73R6ZM/AVCMMTJ22iR5hQ3h67ddGKUz0IM7LYwKMKhYDkiEABMEohFAnYcrVeMrl5wr
Krt5Oj7SWysSCti7gEmcPlScYumwBfYL89kOAEwfFH+O/GYj0m60Ck+euz6g7Kr+yeY26wiMQQ0A
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
BlpQ/AEgBqxiAAAAAElFTkSuQmCC'>&nbsp;&nbsp;&nbsp;&nbsp;Job Report : %(label)s</h2>
<table border='0' cellpadding='2' cellspacing='0' >
<tr><th>System</th><td>%(host)s</td>
    <th>Date</th><td>%(published)s</td>
    <th rowspan='3'>&nbsp;&nbsp;&nbsp;</th>
    <th>Page</th>
    <th rowspan='3'><a href='' download='Job-%(ts)s-id%(id)d.html'><button>Download</button></a></th></tr>
<tr><th>Job ID</th><td>%(id)d</td>
    <th>Source</th><td>%(src)s</td>
    <th rowspan='2'><button title='go to start' onclick="paginate(-10000)">&lt;&lt;</button><button title='back' onclick="paginate(-1)">&lt;</button><span id='pgnum'>1</span><button title='forward' onclick="paginate(1)">&gt;</button><button title='go to end' onclick="paginate(10000)">&gt;&gt;</button></th> </tr>
<tr><th>Project</th><td><a href='/projects/%(projectId)s/report'>%(project)s</a> &nbsp;&nbsp;[ID: %(projectId)s]</td>
    <th>Destination</th><td>%(dst)s</td></tr>
</table>
<p>
""" % info)

        orderOfMag= [ c for c in " KMGTPEZY"]

        def readable(value, format='%7.3f', units='') :
        #    print("readable(", value, ") type=", type(value))
            try:
                value = int(value)
            except:
                return ('--.---', '')
            index = 0
            tval = value
            while tval >= 1000 :
                tval /= 1000
                index += 1
            return(format % tval, orderOfMag[index] + units)

        def comma(value) :
            '''Convert a numberic value into a string with commas ever 3 decimal places'''
            if value == '(transient)' : value = 0
            if isinstance(value, str) : value = int(value)
            result = "{:,}".format(value)
            return "%18.18s" % result

        if len( info['history'] ) > 0 :
            fd.write("<table cellpadding='2' cellspacing='0'>")

            rollup = {
                'files.move' : 0,
                'files.work' : 0, 
                'bytes.move' : 0,
                'bytes.work' : 0,
                'runs' : 0,
                'elapsed' : datetime.timedelta(seconds=0),
            }

            # Compute Roll-up values first
            for hitem in info['history'] :
                rollup['runs'] += 1
                moved = ''
                work = ''
                sep = ''
                if 'files.total' in hitem : 
                    ftot = hitem['files.total']
                    fnew = hitem['files.new']
                    rollup['files.move'] += int(fnew)
                    rollup['files.work'] += int(ftot)
                if 'bytes.total' in hitem : 
                    btot = hitem['bytes.total']
                    bnew = hitem['bytes.new']
                    rollup['bytes.move'] += int(bnew)
                    rollup['bytes.work'] += int(btot)
        
                if 'ielapsed' in hitem :
                    rollup['elapsed'] += hitem['ielapsed']
            moved = ''
            work = ''
            sep = ''
            if rollup['files.work'] :
                (hrTot,unitTot) = readable(rollup['files.work'], '%7.3f', 'File')
                (hrNew,unitNew) = readable(rollup['files.move'], '%7.3f' ,'File')
                moved += sep + "%10.10s %s" % (hrNew, unitNew)
                work += sep + "%10.10s %s" % (hrTot, unitTot)
                sep = "\n"
            if rollup['bytes.work'] :
                (hrTot,unitTot) = readable(rollup['bytes.work'], '%7.3f', 'Byte')
                (hrNew,unitNew) = readable(rollup['bytes.move'], '%7.3f' ,'Byte')
                moved += sep + "%10.10s %s" % (hrNew, unitNew)
                work += sep + "%10.10s %s" % (hrTot, unitTot)
                sep = "\n"
            if work :
                fd.write("""
<tr class='rollup' id='rtot'>
<td colspan='3' style="text-align:center; color: #fff;" >Lifetime Summary</td><td style="color:#fff;">%s</td><td colspan='2'  style="text-align:center; color: #fff;">%d runs</td><td style="color:#fff;"><pre>%s</pre></td><td style="color:#fff;"><pre>%s</pre></td>
</tr>
<tr><th colspan='8'>&nbsp;</th></tr>
""" % (str(rollup['elapsed']), rollup['runs'], moved, work))

            fd.write("""<tr><th>Ops</th><th>Start</th><th>End</th><th>Elapsed</th><th>Action</th><th>Status</th><th>Data Moved</th><th>Work Done</th></tr>\n""")

            # For run history
            cnt = 0
            lastDate = ''
            style = 'even'
            for hitem in info['history'] :
                if lastDate != hitem['start'][0:10] :
                    lastDate = hitem['start'][0:10]
                    style = 'even' if style == 'odd' else 'odd'
        
                moved = ''
                work = ''
                sep = ''
                if 'files.total' in hitem : 
                    ftot = hitem['files.total']
                    fnew = hitem['files.new']
                    (hrTot,unitTot) = readable(ftot, '%7.3f', 'File')
                    (hrNew,unitNew) = readable(fnew, '%7.3f' ,'File')
                    moved += sep + "%10.10s %s" % (hrNew, unitNew)
                    work += sep + "%10.10s %s" % (hrTot, unitTot)
                    sep = "\n"
                if 'bytes.total' in hitem : 
                    btot = hitem['bytes.total']
                    bnew = hitem['bytes.new']
                    (hrTot,unitTot) = readable(btot, '%7.3f', 'Byte')
                    (hrNew,unitNew) = readable(bnew, '%7.3f', 'Byte')
                    moved += sep + "%10.10s %s" % (hrNew, unitNew)
                    work += sep + "%10.10s %s" % (hrTot, unitTot)
                    sep = "\n"
        
            #    hitem['kpp'] = pp.pformat(hitem)
                hitem['moved'] = moved
                hitem['work'] = work
                hitem['style'] = style
                hitem['cnt'] = cnt
                hitem['visible'] = '' if cnt < page else 'none'
                fd.write("""
<tr id='r%(cnt)d' class='%(style)s' style="display:%(visible)s">
<td id='r%(cnt)d'><a href='#r%(cnt)d' title='close log' onclick="opX('r%(cnt)d')">&times;</a>|<a href='#r%(cnt)d' title='Job Log' onclick="opJ('r%(cnt)d')">J</a>|<a href='#r%(cnt)d' title='Exception List' onclick="opE('r%(cnt)d')">E</a></td>
<td>%(start)s</td><td>%(finish)s</td><td>%(elapsed)s</td><td class='ac%(action)s'>%(action)s</td><td class='st%(status)s'>%(status)s</td><td><pre>%(moved)s</pre></td><td><pre>%(work)s</pre></td></tr>
""" % hitem)

                fd.write("<tr id='r%(cnt)dj' style='display:none'><td class='%(style)s' colspan='8'><pre>" % hitem)
                if 'cust' in hitem and hitem['cust'] : 
                    fd.write("\n".join(hitem['cust']) )
                else :
                    fd.write("No log data")
                fd.write("</pre></td></tr>\n")

                fd.write("<tr id='r%(cnt)de' style='display:none'><td class='%(style)s' colspan='8'><pre>" % hitem)
                if 'err' in hitem and hitem['err'] : 
                    fd.write("\n".join( hitem['err']) )
                else :
                    fd.write("No log data")
                fd.write("</pre></td></tr>\n")

                del hitem['moved']
                del hitem['work']
                del hitem['style']
                del hitem['cnt']
                del hitem['visible']

                cnt += 1

            fd.write("</table>\n")

        elif 'jobmsg' in info :
            fd.write("\n<p>%(jobmsg)s" % info)
        else :
            fd.write("\n<p>No job history information")

        del info['pageSize']
        fd.write("""
<script type="text/javascript"> setRows(%d); paginate(0);</script>

</body>
</html>
""" % cnt)
        return fd.getvalue()
    #    fd.close()
    # End of printAsHTML
    #-----------------------------------------------------------------------------
    def printCSV(info) :
        import csv
        import io

        fd = io.StringIO()
        csv_w = csv.writer(fd, quoting=csv.QUOTE_ALL)

        elapsed = 0
        newbytes = 0
        totalbytes = 0
        newfiles = 0
        totalfiles = 0

        if info :
            #-----------------------------------------------------------------------------
            for hitem in info['history'] :
                if not 'files.new' in hitem:
                    hitem['files.new'] = '0'
                if not 'bytes.new' in hitem:
                    hitem['bytes.new'] = '0'
                if not 'files.total' in hitem:
                    hitem['files.total'] = '0'
                if not 'bytes.total' in hitem:
                    hitem['bytes.total'] = '0'
                elapsed += int(hitem['ielapsed'].total_seconds())
                newfiles += int(hitem['files.new'])
                newbytes += int(hitem['bytes.new'])
                totalfiles += int(hitem['files.total'])
                totalbytes += int(hitem['bytes.total'])
            csv_w.writerow(('Total Elapsed Time', 'Read files/bytes', 'Written files/bytes'))
            csv_w.writerow((elapsed, '{}/{}'.format(totalfiles,totalbytes), '{}/{}'.format(newfiles,newbytes)))
            #-----------------------------------------------------------------------------
            for hitem in info['history'] :
                if 'cust' in hitem and hitem['cust'] : 
                    csv_w.writerow(('Date & Time', 'Level', 'Message'))
                    for h in hitem['cust'] :
                        csv_w.writerow(h)
                else :
                    csv_w.writerow(("No Job log data", '', ''))

                if 'err' in hitem and hitem['err'] : 
                    csv_w.writerow(('Date & Time', 'Type', 'File/Directory -- Message'))
                    for e in hitem['err'] :
                        csv_w.writerow(e)
                else :
                    csv_w.writerow(("No Error log data", '', ''))
        else :
            # No information gathered.
            csv_w.writerow(('Error:', '', "No such migration job"))
        return fd.getvalue()
    # End of printCSV
    #-----------------------------------------------------------------------------
    def printJSON(info) :
        import json

        if not info:
            return({'stderr':'No such migration job'})

        fd = sys.stdout
        cnt = 0

        elapsed = 0
        newbytes = 0
        totalbytes = 0
        newfiles = 0
        totalfiles = 0

        if len( info['history'] ) > 0 :
            # For run history
            toutput = []
            routput = {}
            for hitem in info['history'] :
                if not 'files.new' in hitem:
                    hitem['files.new'] = '0'
                if not 'bytes.new' in hitem:
                    hitem['bytes.new'] = '0'
                if not 'files.total' in hitem:
                    hitem['files.total'] = '0'
                if not 'bytes.total' in hitem:
                    hitem['bytes.total'] = '0'

                run = { 'start' : hitem['start'],
                        'end' : hitem['finish'],
                        'elapsed' : int(hitem['ielapsed'].total_seconds()),
                        'action' : hitem['action'],
                        'status' : hitem['status'], 
                        'newfiles': int(hitem['files.new']),
                        'newbytes': int(hitem['bytes.new']),
                        'totalfiles': int(hitem['files.total']),
                        'totalbytes': int(hitem['bytes.total']) }
                elapsed += int(hitem['ielapsed'].total_seconds())
                newfiles += int(hitem['files.new'])
                newbytes += int(hitem['bytes.new'])
                totalfiles += int(hitem['files.total'])
                totalbytes += int(hitem['bytes.total'])
                routput.update( { 'run' : run } )
                if 'cust' in hitem and hitem['cust'] : 
                    routput.update( { 'joblog':hitem['cust'] } )
                else :
                    routput.update( { 'joblog':"No job log data" } )

                if 'err' in hitem and hitem['err'] : 
                    routput.update( { 'errlog':hitem['err'] } )
                else :
                    routput.update( {'errlog':"No error log data" } )
                toutput.append(routput)
                routput = {}
            output = { 'job' :
                       { 
                         'label' : info['label'],
                         'destination' : info['destination'],    # destination -- tested? Is a dict?
                         'source' : info['source'],             # source -- tested? Is a dict?
                         'projectid' : int(info['projectid']),
                         'hostname' : info['hostname'],
                         'name' : info['name'],
                         'id' : int(info['id']),
                         'elapsed': elapsed,
                         'newbytes': newbytes,
                         'totalbytes': totalbytes,
                         'newfiles': newfiles,
                         'totalfiles': totalfiles,
                       },
                       'runs' : toutput
                     }

        elif 'jobmsg' in info :
            output = { 'job' :
                       { 
                         'label' : info['label'],
                         'destination' : info['destination'],    # destination -- tested? Is a dict?
                         'source' : info['source'],             # source -- tested? Is a dict?
                         'projectid' : int(info['projectid']),
                         'hostname' : info['hostname'],
                         'name' : info['name'],
                         'id' : int(info['id']),
                         'elapsed': elapsed,
                         'newbytes': newbytes,
                         'totalbytes': totalbytes,
                         'newfiles': newfiles,
                         'totalfiles': totalfiles,
                       },
                       'runs' :
                         {
                           'jobmsg':info['jobmsg']
                         }
                     }
        else :
            output = { 'job' :
                       { 
                         'label' : info['label'],
                         'destination' : info['destination'],    # destination -- tested? Is a dict?
                         'source' : info['source'],             # source -- tested? Is a dict?
                         'projectid' : int(info['projectid']),
                         'hostname' : info['hostname'],
                         'name' : info['name'],
                         'id' : int(info['id']),
                         'elapsed': elapsed,
                         'newbytes': newbytes,
                         'totalbytes': totalbytes,
                         'newfiles': newfiles,
                         'totalfiles': totalfiles,
                       },
                       'runs' :
                         {
                           'message':"No job history information"
                         }
                     }
        return output
    # End of printJSON
    #-----------------------------------------------------------------------------
    # Execute the main routine.
    connect = sqlite3.connect(args.database)
    for mid in getMigrations(args.migration) :

        info = getHistory(mid, args.op)
        if args.csv :
            output = printCSV(info)
        elif args.json :
            output = printJSON(info)
        elif info :
            output = printAsHTML(info)
        else :
            output = printErrHtml("No such migration job")
        return output
#-----------------------------------------------------------------------------
if __name__ == '__main__':
    import sys

    output = main(sys.argv[1:])
    if type(output) != str:
        import json
        print(json.dumps(output, skipkeys=True, sort_keys=True, indent=2, ensure_ascii=True, separators=(',', ':')), file=sys.stdout)
    else:
        print(output, end='')

    exit(0)
#-----------------------------------------------------------------------------
# End of file migReport.py
