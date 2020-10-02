#!/usr/bin/env python3
"""
This module provides IPMI utilities
"""

import argparse
import json
import os
import subprocess
import sys
import traceback
import sqlalchemy
from dateutil.parser import parse

from datamodel import DataModel
from pxlogging import PxLogger
from pxjson import PxJSON


class PxIPMI(object):
    """
    IPMI configuration
    """

    #STATE = "STATE"
    #STATUS = "STATUS"
    #MESSAGE = "MESSAGE"
    #INTERNAL = "INTERNAL"

    #routes
    #IPMI_INFO = "IPMI_INFO"
    # JSON keys
    #CONNECTIONTYPE = 'CONNECTIONTYPE'
    #IPV4 = 'IPV4'
    #NETMASK = 'NETMASK'
    #GATEWAY = 'GATEWAY'
    #MAC = 'MAC'
    #VLAN = 'VLAN'

    # JSON values
    #CONNECTIONNONE = 'none'
    #CONNECTIONSTATIC = 'static'
    #CONNECTIONDHCP = 'dhcp'
    #CONNECTIONBIOS = 'bios'
    #CONNECTIONUNKNOWN = 'unknown'

    #ipmitool keys
    IPMI_SOURCE = 'IP Address Source'
    IPMI_IP = 'IP Address'
    IPMI_NETMASK = 'Subnet Mask'
    IPMI_GATEWAY = 'Default Gateway IP'
    IPMI_MAC = 'MAC Address'
    IPMI_VLAN = '802.1q VLAN ID'

    #ipmitool values
    IPMI_SOURCE_NONE = 'none'
    IPMI_SOURCE_STATIC = 'Static Address'
    IPMI_SOURCE_DHCP = 'DHCP Address'
    IPMI_SOURCE_BIOS = 'bios'

    def __init__(self):
        # Logger info
        logger_name = 'com.parseclabs.ipmi'
        log_pathname = '/px/log/ipmi.log'
        PxLogger.setloggerinfo(logger_name, log_pathname)
        self.logger = PxLogger.getlogger()

        self.ipmitool = '/usr/bin/ipmitool'
        self.ipmisupported = os.path.exists('/dev/ipmi0')

    def parseoutput(self, output):
        """
        parse output from impi lan print int json format
        """

        reply = None
        internalreply = None
        status = 1

        if output is None:
            return status, reply, internalreply

        try:
            tmpreply = {}

            for line in output.splitlines():
                pair = line.split(':', 1)
                length = len(pair)

                key = None
                value = None
                if length > 0:
                    key = pair[0].strip()
                if length > 1:
                    value = pair[1].strip()

                if key == self.IPMI_SOURCE:
                    tmpreply[PxJSON.CONNECTIONTYPE] = PxJSON.CONNECTIONUNKNOWN

                    if value == self.IPMI_SOURCE_NONE:
                        tmpreply[PxJSON.CONNECTIONTYPE] = PxJSON.CONNECTIONNONE
                    elif value == self.IPMI_SOURCE_STATIC:
                        tmpreply[PxJSON.CONNECTIONTYPE] = PxJSON.CONNECTIONSTATIC
                    elif value == self.IPMI_SOURCE_DHCP:
                        tmpreply[PxJSON.CONNECTIONTYPE] = PxJSON.CONNECTIONDHCP
                    elif value == self.IPMI_SOURCE_BIOS:
                        tmpreply[PxJSON.CONNECTIONTYPE] = PxJSON.CONNECTIONBIOS
                elif key == self.IPMI_IP:
                    tmpreply[PxJSON.IPV4] = value
                elif key == self.IPMI_NETMASK:
                    tmpreply[PxJSON.NETMASK] = value
                elif key == self.IPMI_GATEWAY:
                    tmpreply[PxJSON.GATEWAY] = value
                elif key == self.IPMI_MAC:
                    tmpreply[PxJSON.MAC] = value
                elif key == self.IPMI_VLAN:
                    tmpreply[PxJSON.VLAN] = value

                status = 0
                if tmpreply:
                    reply = tmpreply

        except Exception as ex:
            self.logger.error(traceback.format_exc())
            internalreply = {"ex": str(ex)}

        return status, reply, internalreply

    def querysystem(self):
        """
        Fetch current IPMI status from system

        :param : None
        :returns: status, reply
        :raises: None
        """

        status = 1
        obj = PxJSON("Unable to obtain IPMI information")

        condition = True

        while condition:
            condition = False
            if not self.ipmisupported:
                # VM guest?
                self.logger.info("IPMI is not supported.  Unable to fetch values from system.")
                reply = {}
                reply[PxJSON.CONNECTIONTYPE] = None
                reply[PxJSON.IPV4] = None
                reply[PxJSON.NETMASK] = None
                reply[PxJSON.GATEWAY] = None
                reply[PxJSON.MAC] = None
                reply[PxJSON.VLAN] = None
                obj.setroute(PxJSON.IPMI_INFO, reply)
                obj.setsuccess()
                status = 0
                break

            args = [self.ipmitool, 'lan', 'print']
            status, obj, output = self.runcmd(args, obj)

            if status == 0:
                status, reply, internalreply = self.parseoutput(output)
                if status == 0:
                    status = 1
                    if reply is None:
                        self.logger.error("reply={0}".format(reply))
                    elif len(reply) != 6: # this is a count of how many json elements we expect
                        self.logger.error("len(reply)={0}, {1}".format(len(reply), reply))
                    else:
                        obj.setroute(PxJSON.IPMI_INFO, reply)
                        obj.setsuccess()
                        status = 0
                elif internalreply:
                    obj.internal(internalreply)

        self.logger.info("status={0} json={1}".format(status, obj.getjsonpretty()))

        return status, obj.getjson()

    def runcmd(self, args, obj):
        """
        Executes command.  Updates PxJSON obj with any errors.

        :param args: list of command arguments
        :param obj: PxJSON obj
        :returns: status, PxJSON obj, output
        :raises: None
        """
        status = 0
        output = None

        if args is None or obj is None or not isinstance(args, list):
            self.logger.error("args={0} obj={1} argsislist={2}".format(args, obj, isinstance(args, list)))
            status = 1
            return status, obj, output

        try:
            self.logger.info("cmd={0}".format(args))
            output = subprocess.check_output(args, stderr=subprocess.STDOUT)

            if output is not None:
                output = output.decode()

            self.logger.info("output={0}".format(output))
        except subprocess.CalledProcessError as ex:
            self.logger.error("cmd={0} status={1} output={2}".format(ex.cmd, ex.returncode, ex.output))

            obj.internal({"status_code": ex.returncode, "text": str(ex.output)})
            self.logger.error(traceback.format_exc())
            status = ex.returncode
        except Exception as ex:
            self.logger.error(traceback.format_exc())
            obj.internal({"ex": str(ex)})
            status = 1

        return status, obj, output

    def checkattribute(self, obj, attribute, attributestring):
        """
        Check if IPMI attribute is None or matches current system status

        :param systemreply: JSON of current system status
        :param attribute: IPMI attribute to change
        :param attributeString: IPMI attribute string
        :returns: False attribute is not None and does not match current
            system status otherwise returns True
        :raises: None
        """
        if (
                attribute is None or
                PxJSON.IPMI_INFO not in (obj or {}) or
                attribute not in (obj[PxJSON.IPMI_INFO] or {}) or
                attributestring == obj[PxJSON.IPMI_INFO][attribute] or
                attributestring == ''
            ):
            return False

        self.logger.info("Changing '{0}' from '{1}' to '{2}'".format(attribute, obj[PxJSON.IPMI_INFO][attribute], attributestring))

        return True

    def testip(self, ip):
        """
        test an IP if it exists

        :param ip - ip address
        :returns: status
        :raises: None
        """
        obj = PxJSON("IP address already exists")
        status, obj, _ = self.runcmd(['/usr/bin/ping', '-qc', '3', ip], obj)
        if status == 0:
            return True, obj
        return False, obj

    def changesystem(self, jsonstr):
        """
        Apply system IPMI configuration

        :param jsonstr - JSON formatted string
        :returns: status, PxJSON
        :raises: None
        Any parameter set to None is not applied to the system.
        """

        status = 1
        obj = PxJSON("Unable to set IPMI configuration")

        status, jsondict = self.validateipmijson(self.loadjson(jsonstr, obj))

        obj.setroute(PxJSON.IPMI_INFO, jsondict[PxJSON.IPMI_INFO])

        if status == 0:
            status, obj = self.changesystem2(obj)

        return status, obj.getjson()

    def changesystem2(self, obj):
        """
        Apply system IPMI configuration

        :param obj: PxJSON object
        :returns: status, PxJSON
        :raises: None
        Any parameter set to None is not applied to the system.
        """

        status = 1
        obj.setfailure("Unable to set IPMI configuration")

        new_connectiontype = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.CONNECTIONTYPE][PxJSON.VALUE]
        new_ipaddress = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.IPV4][PxJSON.VALUE]
        new_netmask = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.NETMASK][PxJSON.VALUE]
        new_gateway = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.GATEWAY][PxJSON.VALUE]
        new_vlan = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.VLAN][PxJSON.VALUE]

        condition = True

        while condition:
            if not self.ipmisupported:
                # VM guest?
                self.logger.info("IPMI is not supported.  Not applying changes.")
                obj.setsuccess()
                status = 0
                break

            systemstatus, systemjson = self.querysystem()
            systemobj = PxJSON("")
            systemobj.setjson(json.loads(systemjson))
            if systemstatus != 0 or not systemobj.issuccess():
                obj.internal(systemobj.getinternal())
                break

            self.logger.info("systemreply={0}".format(systemobj.getjson()))

            if self.checkattribute(systemobj.getjsondict(), PxJSON.CONNECTIONTYPE, new_connectiontype):
                self.logger.info("Setting={}".format(new_connectiontype))
                args = [self.ipmitool, 'lan', 'set', '1', 'ipsrc', new_connectiontype]
                status, obj, _ = self.runcmd(args, obj)
                if status != 0:
                    break

            if new_ipaddress is not None and PxJSON.CONNECTIONTYPE != PxJSON.CONNECTIONDHCP:
                if self.checkattribute(systemobj.getjsondict(), PxJSON.IPV4, new_ipaddress):
                    args = [self.ipmitool, 'lan', 'set', '1', 'ipaddr', new_ipaddress]
                    status, obj, _ = self.runcmd(args, obj)
                    if status != 0:
                        break

            if new_netmask is not None and PxJSON.CONNECTIONTYPE != PxJSON.CONNECTIONDHCP:
                if self.checkattribute(systemobj.getjsondict(), PxJSON.NETMASK, new_netmask):
                    args = [self.ipmitool, 'lan', 'set', '1', 'netmask', new_netmask]
                    status, obj, _ = self.runcmd(args, obj)
                    if status != 0:
                        break

            if new_gateway is not None and PxJSON.CONNECTIONTYPE != PxJSON.CONNECTIONDHCP:
                if self.checkattribute(systemobj.getjsondict(), PxJSON.GATEWAY, new_gateway):
                    args = [self.ipmitool, 'lan', 'set', '1', 'defgw', 'ipaddr', new_gateway]
                    status, obj, _ = self.runcmd(args, obj)
                    if status != 0:
                        break

            if new_vlan is not None:
                if self.checkattribute(systemobj.getjsondict(), PxJSON.VLAN, new_vlan):
                    self.logger.info("Setting={}".format(new_vlan))
                    if new_vlan == "0":
                        args = [self.ipmitool, 'lan', 'set', '1', 'vlan', 'id', 'off']
                    else:
                        args = [self.ipmitool, 'lan', 'set', '1', 'vlan', 'id', new_vlan]
                    status, obj, _ = self.runcmd(args, obj)
                    if status != 0:
                        break

            obj.setsuccess()
            status = 0

            condition = False


        self.logger.info("status={0} json={1}".format(status, obj.getjsonpretty()))
        return status, obj

    def getconfiguration(self):
        """
        Fetch IPMI configuration from database

        :param: none
        :returns: status, reply
        :raises: None
        """

        status = 1
        obj = PxJSON("Unable to obtain IPMI configuration")

        try:
            table_name = 'systemsetups'
            res = DataModel().ExecuteRawQueryStatement("SELECT ipmi_connection_type, ipmi_address, ipmi_netmask, ipmi_gateway, ipmi_vlan from {0}".format(table_name))
            reply = {}
            for row in res:
                self.logger.info(row)
                reply[PxJSON.CONNECTIONTYPE] = row['ipmi_connection_type']
                reply[PxJSON.IPV4] = row['ipmi_address']
                reply[PxJSON.NETMASK] = row['ipmi_netmask']
                reply[PxJSON.GATEWAY] = row['ipmi_gateway']
                reply[PxJSON.VLAN] = row['ipmi_vlan'] if row['ipmi_vlan'] != 0 else 'undefined'

            obj.setroute(PxJSON.IPMI_INFO, reply)
            obj.setsuccess()
            status = 0
        except sqlalchemy.exc.OperationalError as ex:
            self.logger.error(traceback.format_exc())
            obj.internal({"exception": str(ex)})
        except Exception as ex:
            self.logger.error(traceback.format_exc())
            obj.internal({"exception": str(ex)})

        self.logger.info("status={0} json={1}".format(status, obj.getjsonpretty()))

        return status, obj.getjson()

    def loadjson(self, jsonstr, obj):
        """ load JSON from string """

        jsondict = None

        try:
            jsondict = json.loads(jsonstr)
        except TypeError as ex:
            self.logger.error(traceback.format_exc())
            obj.internal({"exception": str(ex)})
        except Exception as ex:
            self.logger.error(traceback.format_exc())
            obj.internal({"exception": str(ex)})

        self.logger.info("jsondict={0}".format(jsondict))

        return jsondict

    def validateipmijson(self, data):
        """
        Validate IPMI information

        :param data - dictionary with IPMI information
        :returns: number_of_errors, dictionary
        :raises: None
        """

        err_count = 0
        reply = {PxJSON.IPMI_INFO: {}}
        ipmi_keys = {PxJSON.CONNECTIONTYPE: [PxJSON.CONNECTIONSTATIC, PxJSON.CONNECTIONDHCP], PxJSON.IPV4: [], PxJSON.NETMASK: [], PxJSON.GATEWAY: [], PxJSON.VLAN: []}
        option_keys = {PxJSON.IGNOREUNUSEDKEYS: [PxJSON.TRUE, PxJSON.FALSE]}

        if PxJSON.IPMI_INFO not in (data or {}):
            reply[PxJSON.IPMI_INFO] = {PxJSON.VALUE: None, PxJSON.STATUS: PxJSON.MISSING_KEY}
            err_count += 1

        ignore_unused_keys = False
        if PxJSON.OPTIONS in (data or {}) and PxJSON.IGNOREUNUSEDKEYS in (data[PxJSON.OPTIONS] or {}):
            if data[PxJSON.OPTIONS][PxJSON.IGNOREUNUSEDKEYS] == PxJSON.TRUE:
                ignore_unused_keys = True

        for key in (data or {}):
            if key == PxJSON.IPMI_INFO:
                if data[key] is None:
                    reply[key] = {PxJSON.VALUE: data[key], PxJSON.STATUS: PxJSON.MISSING_VALUE}
                    err_count += 1
                elif not data[key] or not isinstance(data[key], dict):
                    reply[key] = {PxJSON.VALUE: data[key], PxJSON.STATUS: PxJSON.INVALID_VALUE}
                    err_count += 1
                else:
                    #iterate through keys
                    for ipmi_info_key in data[key]:
                        #check if valid key
                        if ipmi_info_key in ipmi_keys:
                            # check if value is supported
                            if data[key][ipmi_info_key] and isinstance(data[key][ipmi_info_key], str) and (not ipmi_keys[ipmi_info_key] or data[key][ipmi_info_key] in ipmi_keys[ipmi_info_key]):
                                reply[key][ipmi_info_key] = {PxJSON.VALUE: data[key][ipmi_info_key], PxJSON.STATUS: PxJSON.VALID}
                            else:
                                reply[key][ipmi_info_key] = {PxJSON.VALUE: data[key][ipmi_info_key], PxJSON.STATUS: PxJSON.INVALID_VALUE}
                                err_count += 1
                        else:
                            reply[key][ipmi_info_key] = {PxJSON.VALUE: data[key][ipmi_info_key], PxJSON.STATUS: PxJSON.INVALID_KEY}
                            if not ignore_unused_keys:
                                err_count += 1

                    #check if required keys exist
                    for ipmi_info_key in ipmi_keys:
                        if ipmi_info_key not in reply[key]:
                            reply[key][ipmi_info_key] = {PxJSON.VALUE: None, PxJSON.STATUS: PxJSON.MISSING_KEY}
                            err_count += 1

                    #check for invalid DHCP values
                    if PxJSON.CONNECTIONTYPE in reply[key] and reply[key][PxJSON.CONNECTIONTYPE][PxJSON.VALUE] == PxJSON.CONNECTIONDHCP:
                        for curkey in [PxJSON.IPV4, PxJSON.GATEWAY, PxJSON.NETMASK, PxJSON.VLAN]:
                            self.logger.info("curkey={0}".format(curkey))
                            if curkey in reply[key]:
                                self.logger.info("reploy={0}".format(reply[key][curkey][PxJSON.VALUE]))
                                if reply[key][curkey][PxJSON.VALUE] and curkey != PxJSON.VLAN:
                                    reply[key][curkey][PxJSON.STATUS] = PxJSON.INVALID_VALUE
                                    err_count += 1
                                elif reply[key][curkey][PxJSON.STATUS] == PxJSON.INVALID_VALUE:
                                    # value is None that is ok.  Reset error
                                    reply[key][curkey][PxJSON.STATUS] = PxJSON.VALID
                                    err_count -= 1

            elif key == PxJSON.OPTIONS:
                if key not in reply:
                    reply[key] = {}
                if data[key] is None:
                    reply[key] = {PxJSON.VALUE: data[key], PxJSON.STATUS: PxJSON.MISSING_VALUE}
                    if not ignore_unused_keys:
                        err_count += 1
                elif not data[key] or not isinstance(data[key], dict):
                    reply[key] = {PxJSON.VALUE: data[key], PxJSON.STATUS: PxJSON.INVALID_VALUE}
                    err_count += 1
                else:
                    #iterate through keys
                    for options_key in data[key]:
                        #check if valid key
                        if options_key in option_keys:
                            # check if value is supported
                            if data[key][options_key] and isinstance(data[key][options_key], str) and (not option_keys[options_key] or data[key][options_key] in option_keys[options_key]):
                                reply[key][options_key] = {PxJSON.VALUE: data[key][options_key], PxJSON.STATUS: PxJSON.VALID}
                            else:
                                reply[key][options_key] = {PxJSON.VALUE: data[key][options_key], PxJSON.STATUS: PxJSON.INVALID_VALUE}
                                err_count += 1
                        else:
                            reply[key][options_key] = {PxJSON.VALUE: data[key][options_key], PxJSON.STATUS: PxJSON.INVALID_KEY}
                            if not ignore_unused_keys:
                                err_count += 1
            else:
                reply[key] = {PxJSON.VALUE: data[key], PxJSON.STATUS: PxJSON.INVALID_KEY}
                if not ignore_unused_keys:
                    err_count += 1

        if PxJSON.IPMI_INFO not in (data or {}):
            reply[PxJSON.IPMI_INFO] = {PxJSON.VALUE: None, PxJSON.STATUS: PxJSON.MISSING_KEY}
            err_count += 1

        status = 0

        if err_count > 0:
            status = 1
        self.logger.info("status={0} data={1} reply={2}".format(status, data, reply))

        return status, reply

    def setconfiguration(self, jsonstr, applyconfiguration):
        """
        Set IPMI configuration in database
        :param jsonstr - JSON formatted string
        :param applyconfiguration - apply configuration to system as well?
        :returns: status, PxJSON
        :raises: None
        """

        status = 1
        obj = PxJSON("Unable to set IPMI configuration")

        status, jsondict = self.validateipmijson(self.loadjson(jsonstr, obj))

        obj.setroute(PxJSON.IPMI_INFO, jsondict[PxJSON.IPMI_INFO])

        if status == 0:
            status, obj = self.setconfiguration2(obj)

        if status == 0 and applyconfiguration:
            status, obj = self.changesystem2(obj)

        return status, obj.getjson()


    def setconfiguration2(self, obj):
        """
        Set IPMI configuration in database

        :param obj - PxJSON object
        :returns: status, PxJSON
        :raises: None
        Any attribute set to None, is set to null in the database.
        """

        status = 1
        tmpobj = PxJSON("Unable to set IPMI configuration")

        tmpstatus, tmpjson = self.getconfiguration()
        tmpobj.setjson(json.loads(tmpjson))

        if tmpstatus != 0 or not tmpobj.issuccess():
            obj.internal(tmpobj.getinternal())
            return status, obj

        old_connectiontype = tmpobj.getroute(PxJSON.IPMI_INFO)[PxJSON.CONNECTIONTYPE]
        old_ipaddress = tmpobj.getroute(PxJSON.IPMI_INFO)[PxJSON.IPV4]
        old_netmask = tmpobj.getroute(PxJSON.IPMI_INFO)[PxJSON.NETMASK]
        old_gateway = tmpobj.getroute(PxJSON.IPMI_INFO)[PxJSON.GATEWAY]
        old_vlan = tmpobj.getroute(PxJSON.IPMI_INFO)[PxJSON.VLAN]

        new_connectiontype = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.CONNECTIONTYPE][PxJSON.VALUE]
        new_ipaddress = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.IPV4][PxJSON.VALUE]
        new_netmask = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.NETMASK][PxJSON.VALUE]
        new_gateway = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.GATEWAY][PxJSON.VALUE]
        new_vlan = obj.getroute(PxJSON.IPMI_INFO)[PxJSON.VLAN][PxJSON.VALUE]

        if old_connectiontype != new_connectiontype:
            self.logger.info("Changing {0}: {1} to {2}".format(PxJSON.CONNECTIONTYPE, old_connectiontype, new_connectiontype))

        if old_ipaddress != new_ipaddress:
            stat, tmpobj = self.testip(new_ipaddress)
            if stat:
                self.logger.error("{0} already exists".format(new_ipaddress))
                return 1, tmpobj
            self.logger.info("Changing {0}: {1} to {2}".format(PxJSON.IPV4, old_ipaddress, new_ipaddress))


        if old_netmask != new_netmask:
            self.logger.info("Changing {0}: {1} to {2}".format(PxJSON.NETMASK, old_netmask, new_netmask))

        if old_gateway != new_gateway:
            self.logger.info("Changing {0}: {1} to {2}".format(PxJSON.GATEWAY, old_gateway, new_gateway))

        if old_vlan != new_vlan:
            self.logger.info("Changing {0}: {1} to {2}".format(PxJSON.VLAN, old_vlan, new_vlan))

        try:
            table_name = 'systemsetups'
            DataModel().ExecuteRawQueryStatement2("UPDATE {0} SET ipmi_connection_type=?, ipmi_address=?, ipmi_netmask=?, ipmi_gateway=?, ipmi_vlan=?".format(table_name), (new_connectiontype, new_ipaddress, new_netmask, new_gateway, new_vlan))

            obj.setsuccess()
            status = 0
        except sqlalchemy.exc.OperationalError as ex:
            self.logger.error(traceback.format_exc())
            obj.internal({"exception": str(ex)})
        except Exception as ex:
            self.logger.error(traceback.format_exc())
            obj.internal({"exception": str(ex)})

        self.logger.info("status={0} json={1}".format(status, obj.getjsonpretty()))

        return status, obj

    def applyconfiguration(self):
        """ call getconfiguration() and changesystem2() """

        obj = PxJSON("Unable to apply IPMI configuration")
        tmpobj = PxJSON("Unable to apply IPMI configuration")

        tmpstatus, tmpjson = self.getconfiguration()
        jsondict = json.loads(tmpjson)
        tmpobj.setjson(jsondict)

        if tmpstatus != 0 or not tmpobj.issuccess():
            obj.internal(tmpobj.getinternal())
            return tmpstatus, obj

        configuration = {}
        for key in jsondict[PxJSON.IPMI_INFO]:
            configuration[key] = {PxJSON.VALUE: jsondict[PxJSON.IPMI_INFO][key]}
        obj.setroute(PxJSON.IPMI_INFO, configuration)

        status, obj = self.changesystem2(obj)

        return status, obj.getjson()

    def deleteevents(self, ids=None, outputfile=None):
        status = 0
        table_name = 'ipmievents'
        # delet all event records if no ids provided
        sqlcommand = "DELETE from {0}".format(table_name)
        # delete selected event records
        if ids is not None:
           sqlcommand += " WHERE id IN ({0})".format(ids)
   
        if outputfile is not None:
            reply = "{'" + table_name + "':["
        else:
            reply = ''

        try:
            ipmidata = DataModel().ExecuteRawQueryStatement(sqlcommand)
        except sqlalchemy.exc.OperationalError as ex:
            self.logger.error(traceback.format_exc())
            if outputfile is not None:
                reply = reply + '{"exception":' + str(ex) + '}'
                status = 1
        except Exception as ex:
            self.logger.error(traceback.format_exc())
            if outputfile is not None:
                reply = reply + '{"exception":' + str(ex) + '}'
                status = 1

        if outputfile is not None:
            reply = reply + "\n]}"
        self.logger.info("status=0 json={0}".format(reply))
        return status, reply

    def sel_timeupdated(self):
        # Update the timeupdated table with domain ipmievents for datetime now.
        domain_name = 'ipmievents'
        table_name = 'timeupdated'
        sqlcommand = ("UPDATE {0} SET datetime=DATETIME('now') WHERE domain='{1}'"
                        .format(table_name, domain_name))
        try:
            ipmidata = DataModel().ExecuteRawQueryStatement(sqlcommand)
        except sqlalchemy.exc.OperationalError as ex:
            self.logger.error(traceback.format_exc())
            print('{"exception":' + str(ex) + '}')
            return 1
        except Exception as ex:
            self.logger.error(traceback.format_exc())
            print('{"exception":' + str(ex) + '}')
            return 1
        return 0

    def captureevents(self):
        """
        Capture BMC/IPMI System Event Log entries (SEL) and put into common DB.

        :param: none
        :returns: status, reply
        :raises: None

        Make sure IPMI is present.
        Get the IPMI sel logs.
        There may be more than 1 line (usually come in doubles).
        Insert into the common database. That routine eliminates duplicates.
        When finished, clear all SEL logs.
        """

        # Only do this if ipmi is in the kernel and active. (Ignore VM's)
        if not self.ipmisupported:
            return 0, ''

        reply = "Unable to capture IPMI/BMC System Event Logs (SEL)"

        # Get the BMC/IPMI System Event Logs (SEL).
        try:
            # stderr=subprocess.PIPE and ignoring sel_lines.stderr -- tosses the output.
            sel_lines = subprocess.check_output(['/usr/bin/ipmitool', 'sel', 'list'], stderr=subprocess.PIPE)
        except subprocess.CalledProcessError as ex:
            self.logger.error(traceback.format_exc())
            reply = reply + '{"exception":' + str(ex) + '}'
            return 1, reply

        # If None, leave.
        if sel_lines is None:
            return 0, ''

        # Convert output to usable format.
        sel_lines = sel_lines.decode('utf-8').splitlines()

        # If nothing in it, leave -- nothing new to enter into common database.
        if sel_lines == []:
            return 0, ''

        # Decode the SEL logs, place into format for SQL table, and put there.
        for a in sel_lines:
            # Split line into list on space pipe space.
            selarr = a.split(' | ')
            # Date and time are separated by ' | ', combine and separate with space.
            dt = selarr[1] + ' ' + selarr[2]
            # Get 'date time' in SQL format datetime.
            selarr[2] = parse(dt)
            # List slicing to remove elements 0 and 1.
            selarr = selarr[2:]
            done = False

            # Get events stored in common database.
            table_name = 'ipmievents'
            sqlcommand = ("SELECT * FROM {0} WHERE (datetime='{1}' and event='{2}' and details='{3}' and status='{4}')"
                            .format(table_name, selarr[0], selarr[1], selarr[2], selarr[3]))
            try:
                ipmidata = DataModel().ExecuteRawQueryStatement(sqlcommand)
            except sqlalchemy.exc.OperationalError as ex:
                self.logger.error(traceback.format_exc())
                reply = reply + '{"exception":' + str(ex) + '}'
                return 1, reply
            except Exception as ex:
                self.logger.error(traceback.format_exc())
                reply = reply + '{"exception":' + str(ex) + '}'
                return 1, reply
            for l in ipmidata:
                done = True
                break

            # New entry, insert into database.
            if not done:
                query = "INSERT INTO ipmievents (datetime, event, details, status) " \
                                        "VALUES (   '{0}', '{1}',   '{2}',  '{3}');" \
                                        .format(selarr[0], selarr[1], selarr[2], selarr[3])

                try:
                    DataModel().ExecuteRawQueryStatement(query)
                except sqlalchemy.exc.OperationalError as ex:
                    self.logger.error(traceback.format_exc())
                    reply = reply + '{"exception":' + str(ex) + '}'
                    return 1, reply
                except Exception as ex:
                    self.logger.error(traceback.format_exc())
                    reply = reply + '{"exception":' + str(ex) + '}'
                    return 1, reply

        # Clear all SEL logs. Note: minor race condition from read to clear is possible.
        try:
            output = subprocess.check_output(['/usr/bin/ipmitool', 'sel', 'clear'], stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as ex:
            print("error in ipmitool sel clear")        # DEBUG
            self.logger.error(traceback.format_exc())
            reply = reply + '{"exception":' + str(ex) + '}'
            return 1, reply

        # Everything is good at this point.
        return 0, ''

    def listevents(self, outputfile):
        """
        Get common DB entries for table ipmievents, stored by captureevents.

        :param: outputfile - JSON output file, if None - special format to stdout.
        :returns: status, reply
        :raises: None
        """

        table_name = 'ipmievents'
        if outputfile is not None:
            reply = "{'" + table_name + "':["
        else:
            reply = ''

        sqlcommand = "SELECT id, datetime, event, details, status from {0}".format(table_name)
        try:
            ipmidata = DataModel().ExecuteRawQueryStatement(sqlcommand)
        except sqlalchemy.exc.OperationalError as ex:
            self.logger.error(traceback.format_exc())
            if outputfile is not None:
                reply = reply + '{"exception":' + str(ex) + '}'
        except Exception as ex:
            self.logger.error(traceback.format_exc())
            if outputfile is not None:
                reply = reply + '{"exception":' + str(ex) + '}'

        # Print out the ipmievents data in the common database.
        first = True
        ipmidata_keys = ipmidata.keys()
        for entry in ipmidata:
            if outputfile is not None:
                reply = reply + ("\n  {" if first else "},\n  {")
                first = second = False
                for v in ipmidata_keys:
                    reply = ((reply + ",") if second else reply) + "'" + str(v) + "':'" + str(entry[v]) + "'"
                    second = True
                self.logger.info(entry)
            else:
                print(entry[0], '|', entry[1], '|', entry[2], '|', entry[3], '|', entry[4])
        if outputfile is not None:
            if ipmidata is not None:
                reply = reply + '}'

        if outputfile is not None:
            reply = reply + "\n]}"
        self.logger.info("status=0 json={0}".format(reply))
        return 0, reply

def processarguments():
    """ main function """

    parser = argparse.ArgumentParser('CLI for IPMI administration')
    parser.add_argument('--jsoninstr', '-j', type=str, help="string in JSON format for input. Required for set operations")
    parser.add_argument('--jsoninfile', '-i', type=str, help="filename in JSON format for input. Required for set operations")
    parser.add_argument('--jsonoutfile', '-o', type=str, help="filename in JSON format for output. Required for nearly all operations. Note: stdout")

    group1 = parser.add_argument_group('Database Configuration', 'get/set database IPMI settings')
    group1.add_argument('--getconfiguration', '-g', action='store_true', help="get current IPMI configuration")
    group1.add_argument('--setconfiguration', '-s', action='store_true', help="set current IPMI configuration")
    group1.add_argument('--applyconfiguration', '-a', action='store_true', help="apply database settings to system")

    group2 = parser.add_argument_group('System Configuration', 'get/set system IPMI settings')
    group2.add_argument('--querysystem', '-q', action='store_true', help="query IPMI system information")
    group2.add_argument('--changesystem', '-c', action='store_true', help="change IPMI system information")

    group3 = parser.add_argument_group('System Event Logs (SEL)', 'get/list BMC/IPMI SEL to/from common DB')
    group3.add_argument('--captureevents', action='store_true', help="get System Event Logs (SEL) entries from IPMI/BMC, place into common DB, and then clear from BMC/IPMI")
    group3.add_argument('--listevents', action='store_true', help="list System Event Log entries in the common DB. No -o option gives different output format to stdout")
    group3.add_argument('--deleteevents', '-d', action='store_true', help='Delete all event records, or selectively with --eventids')
    group3.add_argument('--eventids', '-e', type=str, help="Optional modifier for --deleteevents, provides a comma separated list of event ids to remove from db")

    args = parser.parse_args()

    # Must have one of these.
    count = (args.deleteevents + args.getconfiguration + args.setconfiguration + args.applyconfiguration + args.querysystem +
             args.changesystem + args.captureevents + args.listevents)
    if count == 0:
        print("Choose one of '--getconfiguration', '--setconfiguration', '--applyconfiguration', '--querysystem', '--changesystem', '--captureevents', 'or', '--listevents'")
        parser.print_help()
        return None

    # '--eventids' may only be combined with '--deleteevents'
    if args.eventids:
      if count > 0 and not args.deleteevents:
            print("'--eventids' may only be combined with '--deleteevents'")
            parser.print_help()
            return None
    
    # Only --setconfiguration and --applyconfiguration may be combined.
    if args.applyconfiguration:
        count -= args.applyconfiguration
        if count > 0 and not args.setconfiguration:
            print("'--applyconfiguration' may only be combined with '--setconfiguration'")
            parser.print_help()
            return None

    # Note: count=0 if only --applyconfiguration is set.
    if count != 1 and not args.applyconfiguration:
        print("Choose only one of '--getconfiguration', '--setconfiguration', '--querysystem', '--changesystem', '--captureevents', or '--listevents'")
        parser.print_help()
        return None

    # --setconfiguration and --applyconfiguration require input string or file.
    if not (args.jsoninstr or args.jsoninfile) and (args.setconfiguration or args.changesystem):
        print("'--setconfiguration' and '--changesystem' require '--jsoninstr' or '--jsoninfile'")
        parser.print_help()
        return None

    # No input string or file for --getconfiguration, --querysystem, --captureevents, --listevents.
    if (args.jsoninstr or args.jsoninfile) and (args.getconfiguration or args.querysystem or
        args.captureevents or args.listevents):
        print("'--jsoninstr' and '--jsoninfile' --may only be used with '--setconfiguration' and '--changesystem'")
        parser.print_help()
        return None

    # No output file for --captureevents.
    if args.captureevents:
        if args.jsonoutfile:
            print("There is no output file specification for --captureevents")
            parser.print_help()
            return None
    # If no jsonoutfile for --listevents, print semi-normally.
    elif args.listevents and args.jsonoutfile is None:
        pass
    elif args.deleteevents and args.jsonoutfile is None:
        pass
    # Must have an output file for everything else.
    elif not args.jsonoutfile:
        print("Must have output file for selected option")
        parser.print_help()
        return None

    return args

def main():
    """ main function """
    logger_name = 'com.parseclabs.ipmi'
    log_pathname = '/px/log/ipmi.log'
    PxLogger.setloggerinfo(logger_name, log_pathname)
    logger = PxLogger.getlogger()

    logger.info("%s", sys.argv)

    args = processarguments()

    if args is None:
        return 1

    status = 0
    reply = None
    jsonstr = None
    ipmi = PxIPMI()

    if args.jsoninstr:
        jsonstr = args.jsoninstr
    elif args.jsoninfile:
        try:
            with open(args.jsoninfile, 'r') as infile:
                jsonstr = infile.read()
        except OSError as ex:
            logger.error(traceback.format_exc())
            print(str(ex))
            if status == 0:
                status = ex.errno
            return(status)

    if args.getconfiguration:
        status, reply = ipmi.getconfiguration()
    elif args.setconfiguration:
        status, reply = ipmi.setconfiguration(jsonstr, args.applyconfiguration)
    elif args.applyconfiguration:
        status, reply = ipmi.applyconfiguration()
    elif args.querysystem:
        status, reply = ipmi.querysystem()
    elif args.changesystem:
        status, reply = ipmi.changesystem(jsonstr)
    elif args.captureevents:
        status, reply = ipmi.captureevents()
        if reply is not None and reply != '':
            print("reply=", reply)
        status = ipmi.sel_timeupdated()
        exit(status)                # NOTE: early exit for captureevents, no output what-so-ever.
    elif args.listevents:
        status, reply = ipmi.listevents(args.jsonoutfile)
        # stdout format is different if -o argument not given -- and already done.
        if args.jsonoutfile is None:
            exit(status)            # NOTE: early return for listevents with special stdout format - no STATUS.
    elif args.deleteevents:
        if args.eventids:
            status, reply = ipmi.deleteevents(args.eventids)
        else:
            status, reply = ipmi.deleteevents()

        if reply is not None and reply != '':
            print("reply=". reply)
        exit(status)

    try:
        if args.jsonoutfile == 'stdout':
            print(reply)
        else:
            with open(args.jsonoutfile, 'w') as outfile:
                outfile.write(reply)

    except OSError as ex:
        logger.error(traceback.format_exc())
        if status == 0:
            status = ex.errno

    # Do not output the print of exit status
    if args.listevents:
        exit(status)                # NOTE: early return for listevents -- no EXIT STATUS printing.

    return(status)

if __name__ == '__main__':
    STATUS = main()
    print("EXIT STATUS {}".format(STATUS))
    exit(STATUS)

#-----------------------------------------------------------------------------
# vim: noai ts=8 sts=4 et sw=4 ft=python
