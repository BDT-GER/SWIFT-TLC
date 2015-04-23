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

""" ValueStore File Interface for Swift Object Server"""

import os, stat
import xattr
from swift.obj import diskfile
from swift.common.exceptions import DiskFileNotExist

class VSDiskFileManager(diskfile.DiskFileManager):
    
    def __init__(self, conf, logger):
        diskfile.DiskFileManager.__init__(self, conf, logger)
        
    def get_diskfile_from_audit_location(self, audit_location):
        dev_path = self.get_dev_path(audit_location.device, mount_check=False)
        
        df = VSDiskFile.from_hash_dir(self,
            audit_location.path, dev_path,
            audit_location.partition)
        return df

class VSDiskFileReader(diskfile.DiskFileReader):
    
    def __init__(self, fp, data_file, obj_size, etag, threadpool,
                 disk_chunk_size, keep_cache_size, device_path, logger,
                 quarantine_hook, use_splice, pipe_size, keep_cache=False):
        diskfile.DiskFileReader.__init__(self, fp, data_file, obj_size, etag, threadpool,
                 disk_chunk_size, keep_cache_size, device_path, logger,
                 quarantine_hook, use_splice, pipe_size, keep_cache)
        
    def __iter__(self):
        # construct path to storage file
        storage_file = self._data_file
        # first check if file exists
        if os.path.isfile(storage_file):
            status = None
            corrupted = 0;
            # file exists, now check extended attributes
            try:
                status = xattr.get(storage_file, 'user.vs.online')
                file_stat = os.stat(storage_file)
                try:
                    corrupted_ea = xattr.get(storage_file, 'user.vs.corrupted')
                    corrupted = ord(corrupted_ea[0])
                except Exception, ex:
                    pass
            except IOError:
                pass
            # status meta-data must exist
            if status is None:
                self._logger.info("status for %s is none." % self._data_file)
            # user.vsvfs.online, 1 for online, 2 for offline.
            if (status is not None and (ord(status[0]) == 1 or corrupted == 1)):
                # do normal auditing
                self._logger.info("Performing normal object audit of file %s" % self._data_file)
                for chunk in super(VSDiskFileReader, self).__iter__():
                    if chunk:
                        yield chunk
                return
             
        # file not completely in cache, skip auditing
        self._logger.info("Skipping object audit of file %s because storage file %s doesn't exist with full content "
                           % (self._data_file, storage_file))
        # close file when done
        if not self._suppress_file_closing:
            self.close()
        # return before yielding anything
        # yield is required because this is a generator
        #return
        #yield            
        
        
class VSDiskFile(diskfile.DiskFile):
    
    def __init__(self, mgr, device_path, threadpool, partition,
                 account=None, container=None, obj=None, _datadir=None,
                 policy_idx=0, use_splice=False, pipe_size=None):
        diskfile.DiskFile.__init__(self, mgr, device_path, threadpool, partition,
                 account, container, obj, _datadir, policy_idx, use_splice, pipe_size)
    
    @classmethod
    def from_hash_dir(cls, mgr, hash_dir_path, device_path, partition):
        return cls(mgr, device_path, None, partition, _datadir=hash_dir_path)
        
    def open(self):
        # TODO: check if file in cache
        # if not in cache, return DiskFileNotExist
        # This might break things in future when non-existing files
        # are replicated as well
        # Better continue and use the __iter__ functionality of the
        # DiskFileReader object to handle caching
        df = super(VSDiskFile, self).open()
        return df
            
    def reader(self, keep_cache=False,
               _quarantine_hook=lambda m: None):
        """
        Return a :class:`swift.common.swob.Response` class compatible
        "`app_iter`" object as defined by
        :class:`swift.obj.diskfile.DiskFileReader`.

        For this implementation, the responsibility of closing the open file
        is passed to the :class:`swift.obj.diskfile.DiskFileReader` object.

        :param keep_cache: caller's preference for keeping data read in the
                           OS buffer cache
        :param _quarantine_hook: 1-arg callable called when obj quarantined;
                                 the arg is the reason for quarantine.
                                 Default is to ignore it.
                                 Not needed by the REST layer.
        :returns: a :class:`swift.obj.diskfile.DiskFileReader` object
        """
        # TODO: Check if file is in cache, if not, return exception
        # This might break things in future when non-existing files
        # are replicated as well
        # Better continue and use the __iter__ functionality of the
        # DiskFileReader object to handle caching
        #random.seed()
        #if random.random() > 0.5:
        #    raise DiskFileNotExist
        
        # File in cache, so return a reader object
        dr = VSDiskFileReader(
            self._fp, self._data_file, int(self._metadata['Content-Length']),
            self._metadata['ETag'], self._threadpool, self._disk_chunk_size,
            self._mgr.keep_cache_size, self._device_path, self._logger,
            use_splice=self._use_splice, quarantine_hook=_quarantine_hook,
            pipe_size=self._pipe_size, keep_cache=keep_cache)
        # At this point the reader object is now responsible for closing
        # the file pointer.
        self._fp = None
        return dr
