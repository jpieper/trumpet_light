#!/usr/bin/python

# Copyright 2013 Josh Pieper, jjp@pobox.com.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import subprocess
import sys

def get_revision_and_modified():
    '''Return a tuple of (version, modified).  Version is an integer
    that is either the subversion revision if one is available, or the
    MSB 8 bytes of the git has.  modified is true if the current
    source is modified from revision control.

    All checks are done on the current working directory.
    '''

    git_version = None
    svn_version = None

    version = None
    modified = False

    try:
        git_version = subprocess.check_output("git rev-parse HEAD", shell=True)
        subprocess.check_output(
            "git update-index -q --ignore-submodules --refresh", shell=True)

        result = subprocess.call("git diff-files --quiet --ignore-submodules",
                                 shell=True)
        if result != 0:
            modified = True

        result = subprocess.call(
            "git diff-index --cached --quiet --ignore-submodules HEAD",
            shell=True)
        if result != 0:
            modified = True
    except:
        # We must not be git.
        pass


    if git_version is not None:
        # Try for git-svn.
        try:
            svn_version = subprocess.check_output(
                "git svn find-rev %s" % git_version, shell=True).strip()
            if svn_version == '':
                svn_version = None
        except:
            pass
    else:
        # Try for straight up svn.
        try:
            version_txt = subprocess.check_output("svnversion --no-newline .",
                                                  shell=True)
            if ':' in version_txt:
                version_txt = version_txt.split(':')[-1]
            while not version_txt[-1].isdigit():
                modified = True
                version_txt = version_txt[:-1]
            svn_version = version_txt

        except:
            raise RuntimeError("no version control information found")

    if svn_version is not None:
        version = int(svn_version)
    elif git_version:
        version = int(git_version[0:16], 16)
    else:
        raise RuntimeError("no version control information found")

    return version, modified

def main():
    print 'running revision_update'
    version, modified = get_revision_and_modified()

    text = r'''#define VCS_REVISION 0x%Xll
#define VCS_MODIFIED %d
''' % (version, 1 if modified else 0)

    try:
        current = open(sys.argv[1], 'rb').read()
    except IOError:
        current = ''

    if current != text:
        open(sys.argv[1], 'wb').write(text)



if __name__ == '__main__':
    main()
