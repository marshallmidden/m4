#!/usr/bin/env python3

# this script is meant be called only at boot time by systemd init
# DO NOT RUN unless you're prepared to HBA discovery repopulate the DB
# with new information
#
# Script that will discover and document all HBA's in the system.
# other HBA's are listed as initiators.

# **************************************************************************************************
#
# This class will try to discover all HBAs (Host Bus Adopters) currently present in the system.
# The Script will then try to populate the database with the information. File /px/etc/FC-Slot_List
# lists the physical slot numbers of Qlogic cards that are to be used as Target Mode HBAs. ALL other
# Fibre Channel cards and vendors are assumed to be in initiator mode.
#
# This script will announce the HBA's to LIO for target mode usage.
#
# *************************************************************************************************

import sys, json, os, uuid
import subprocess
from datamodel import DataModel
from pxlogging import PxLogger


class HBADiscovery(object):
    def __init__(self):
        logger_name = 'com.parseclabs.hbadiscovery'
        log_pathname = '/px/log/hbadiscovery.log'
        PxLogger.setloggerinfo(logger_name, log_pathname)
        self.logger = PxLogger.getlogger()
        self.nodeID = "1"

    def __get_hba_wwns(self, op):
        # Return FC card WWNs that are to be used for target.
        cmd = "/px/bin/get-slot-list.py {}".format(op)
        res = subprocess.check_output(cmd, shell=True, close_fds=True, universal_newlines=True).strip()
        if res is not None and str(res) != '':
            wwns = res.splitlines()
            if wwns is not None:
                return wwns
        return []

    def __get_iscsi_wwn(self):
        # This function will fetches the iscsi wwn give by the OS from iqn.1993-08.org.debian:XXXXXXXXX
        try:
            self.logger.info("ENTERED __get_iscsi_wwn")
            file_path = '/etc/iscsi/initiatorname.iscsi'
            with open(file_path, "r") as f:
                lines = f.readlines()
            for line in lines:
                if "iqn" in line:
                    pos = line.find(':')
                    iscsi_wwn = line[pos + 1:]
                    iscsi_wwn = iscsi_wwn.strip()
                    pos = line.find("=")
                    iscsi_init = line[pos + 1:]
                    iscsi_init = iscsi_init.strip()
                    self.logger.info("LEAVING __get_iscsi_wwn. ID {0}, iscsiInitiatorString={1}".format(iscsi_wwn, iscsi_init))
                    return iscsi_wwn, iscsi_init
            self.logger.warn("Unable to locate ID will return -1")
            return -1, -1
        except Exception as ex:
            self.logger.error("An exception occurred while getting iscsiInitator ID: {0}".format(ex))
            return False, False

    def __get_nvme_wwn(self):
        try:
            self.logger.info("ENTERED __get_nvme_wwn")
            file_path = '/etc/nvme/hostnqn'
            with open(file_path, "r") as f:
                lines = f.readlines()
            for line in lines:
                if "nqn" in line:
                    nqn = line.strip()
                    self.logger.info("LEAVING __get_nvme_wwn. {0}".format(nqn))
                    return nqn,nqn
            self.logger.warn("Unable to locate NVMe wwn will return -1")
            return -1, -1
        except Exception as ex:
            self.logger.error("An exception occurred while getting NVMe wwn: {0}".format(ex))
            return False, False

    def __get_target_hba_wwns(self):
        target_wwns = self.__get_hba_wwns('target')
        for wwn in target_wwns:
            self.logger.info("Found target HBA -- WWN is {0}".format(wwn))

        try:
            self.logger.info("Fetching target iqn ID")
            idnum, _ = self.__get_iscsi_wwn()
            if idnum != -1 and idnum is not False:
                target_wwn="iqn.2016-12.com.parseclabs:target.{0}".format(idnum.strip())
                target_wwns.append(target_wwn)
                self.logger.info("Found target iqn ID {0}".format(idnum))
            self.logger.info("LEAVING __get_initiator_hba_wwns")
            return target_wwns
        except Exception as ex:
            self.logger.error("An exception occurred while populating initiator HBA list: {0}".format(ex))
            return None


    # this gets both FC and iscsi wwns
    def __get_initiator_hba_wwns(self):
        initiator_wwns = self.__get_hba_wwns('initiator')
        for wwn in initiator_wwns:
            self.logger.info("Found Initiator HBA -- WWN is {0}".format(wwn))

        try:
            self.logger.info("Fetching iSCSI Initator iqn")
            _, iqn = self.__get_iscsi_wwn()
            if iqn != -1 and iqn is not False:
                initiator_wwns.append(iqn)
                self.logger.info("Found iSCSI initiator ID {0}".format(iqn))
            self.logger.info("Fetching NVMe host nqn")
            _, nqn = self.__get_nvme_wwn()
            if nqn != -1 and nqn is not False:
                initiator_wwns.append(nqn)
                self.logger.info("Found NVMe host nqn {0}".format(nqn))
            self.logger.info("LEAVING __get_initiator_hba_wwns")
            return initiator_wwns
        except Exception as ex:
            self.logger.error("An exception occurred while populating initiator HBA list: {0}".format(ex))
            return None

    def updateInitiatorHBAs(self):
        # This function will update all HBA related information. There are two types present:
        # a) Target HBA's
        #    QLE which will operate in Target mode. Target HBA's will need to be configured in database
        #    and LIO config.
        # b) Initiator HBA's database entries only. (Some could be QLE.)
        self.logger.info("ENTERED updateInitiatorHBAs")
        try:
            table = 'initiator_hbas'
            db = DataModel()
            self.logger.info("clearing table: {0}".format(table))
            cmd = 'DELETE FROM {0} WHERE nodeId={1}'.format(table, self.nodeID)
            db.ExecuteRawQueryStatement(cmd)
            self.logger.info("Now adding the provided wwns.")
            for wwn in self.__get_initiator_hba_wwns():
                self.logger.info("Adding the wwn {0} to the table".format(wwn))
                cmd = ("INSERT INTO {0} (wwn, created_at, updated_at, nodeId) VALUES ('{1}', (select datetime('now')), "
                       "(select datetime('now')), {2});".format(table, wwn, self.nodeID))
                self.logger.info("Executing the command: {0}".format(cmd))
                db.ExecuteRawQueryStatement(cmd)
            return True
        except Exception as ex:
            self.logger.error("An exception occurred while updating the initiator table: {0}".format(ex))
            return False

    def updateTargetHBAs(self):
        self.logger.info("ENTERED updateTargetHBAs")
        try:
            db = DataModel()
            table = 'target_hbas'

            self.logger.info("clearing table: {0}".format(table))
            cmd = 'DELETE FROM {0} WHERE nodeId={1}'.format(table, self.nodeID)
            db.ExecuteRawQueryStatement(cmd)
            for wwn in self.__get_target_hba_wwns():
                cmd = ("INSERT INTO {0} (wwn, created_at, updated_at, nodeId) VALUES ('{1}',"
                   "(select datetime('now')), (select datetime('now')), {2});".format(table, wwn, self.nodeID))
                self.logger.info("Executing the following command: {0}".format(cmd))
                db.ExecuteRawQueryStatement(cmd)

            self.logger.info("Exiting updateTargetHBAs")
            return True
        except Exception as ex:
            self.logger.error("An exception occurred while adding new wwn to TargetHBA table: {0}".format(ex))
            return False

if __name__ == '__main__':
    hba = HBADiscovery()

    if not hba.updateInitiatorHBAs():
        sys.exit(1)

    if not hba.updateTargetHBAs():
        sys.exit(1)

sys.exit(0)
