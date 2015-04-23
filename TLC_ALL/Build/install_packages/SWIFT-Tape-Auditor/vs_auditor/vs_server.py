# Copyright (C) 2014 BDT Media Automation GmbH
#
# Author: Stefan Hauser <stefan.hauser@bdt.de>
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

""" ValueStore Object Server for Swift """

from swift.obj import server
from vs_auditor.vs_diskfile import VSDiskFileManager

class ObjectController(server.ObjectController):
    """
    Implements the WSGI application for the Swift ValueStore Object Server.
    """

    def setup(self, conf):
        self._diskfile_mgr = VSDiskFileManager(conf, self.logger)

        # This is populated by global_conf_callback way below as the semaphore
        # is shared by all workers.
        if 'replication_semaphore' in conf:
            # The value was put in a list so it could get past paste
            self.replication_semaphore = conf['replication_semaphore'][0]
        else:
            self.replication_semaphore = None
        self.replication_failure_threshold = int(
            conf.get('replication_failure_threshold') or 100)
        self.replication_failure_ratio = float(
            conf.get('replication_failure_ratio') or 1.0)


def app_factory(global_conf, **local_conf):
    """paste.deploy app factory for creating WSGI object server apps"""
    conf = global_conf.copy()
    conf.update(local_conf)
    return ObjectController(conf)
