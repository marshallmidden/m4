#!/usr/bin/env python3
import logging
import os
import sys

import click
import click_log

os.setuid(0)
sys.path.append('/px/bin')

from datamodel import DataModel
from setenvlang import setenvlang

setenvlang()

cli_logger = logging.getLogger(__name__)
CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

@click.command(context_settings=CONTEXT_SETTINGS)
@click.version_option(version='4.2.0')
@click_log.simple_verbosity_option()
@click_log.init(__name__)
def listhbas():
    """
    \b
    NAME
        listhbas - will show a list of hbas
    SYNOPSIS
        listhbas [OPTIONS]
    DESCRIPTION
        Will list out available hbas seen by the LightSpeed
        No options are available for this command
    """
    targetHBAStable = DataModel()
    cmd = "SELECT id,wwn from target_hbas"
    q = targetHBAStable.GetListOfQuery(cmd)
    cli_logger.info('List of available target HBAs: ')
    cli_logger.info('---------------------------------------------------------------------------')
    for row in q:
        cli_logger.info('ID: {0} | HBA: {1}'.format(row[0], row[1]))
        cli_logger.info('---------------------------------------------------------------------------')

if __name__ == '__main__':
    try:
        listhbas(prog_name='listhbas')
    except KeyboardInterrupt:
        pass
