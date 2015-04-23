# -*- coding: utf-8 -*-
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

from setuptools import setup

setup(
    name='vs_auditor',
    version='0.1',
    description='ValueStore backend for OpenStack Swift Auditor',
    license='Apache License (2.0)',
    packages=['vs_auditor'],
    classifiers=[
        'License :: OSI Approved :: Apache Software License',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: Python :: 2.6',
        'Environment :: No Input/Output (Daemon)'],
    install_requires=['swift', ],
    entry_points={
        'paste.app_factory': [
            'vs_object=vs_auditor.vs_server:app_factory'],
    },
)
