#!/bin/bash
#
# Script to build package in container.
#

set -ex

export HOME=/tmp/dockerhome
mkdir -v $HOME

gpg --import /var/debkeys/key.asc

gbp_version="$(gbp --version | cut -c5-)"
if dpkg --compare-versions "$gbp_version" ge 0.8.15
then
  # removes debian dir; uses current branch
  gbp_tree=SLOPPY
else
  # keeps debian dir; uses "upstream" branch
  gbp_tree=BRANCH
fi

if [[ "$1" == -S ]]
then
  # only sign source diff; source package must exist
  debuild -S -sd
else
  # build binary and source packages
  gbp buildpackage --git-upstream-tree=$gbp_tree --git-ignore-branch
  debuild -S -sa
fi
