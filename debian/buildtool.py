#!/usr/bin/env python3

import argparse
import datetime
import contextlib
import functools
import os
import shutil
import subprocess

from pathlib import Path

IMAGE_PREFIX = "pluginbuild"

DISTRO_RELEASES = {
    # debian releases
    "stretch": "9",
    "buster": "10",
    "bullseye": "11",
    "bookworm": "12",
    # ubuntu releases
    "trusty": "14.04",
    "xenial": "16.04",
    "bionic": "18.04",
    "cosmic": "18.10",
    "disco": "19.04",
    "focal": "20.04",
    "jammy": "22.04",
    }

SCRIPT_DIR = Path(__file__).resolve().parent

def runproc(*args, **kw):
    subprocess.run(filter(None, args), check=True, **kw)

@contextlib.contextmanager
def chdir(path):
    orig_path = Path.cwd()
    os.chdir(str(path))
    yield
    os.chdir(str(orig_path))

def image_name(dist_id, dist_codename):
    return f"{IMAGE_PREFIX}:{dist_id}-{dist_codename}"

def run_container(dist_id, dist_codename, key_dir, sign_only):
    abs_key_dir = Path(key_dir).expanduser().resolve()
    timestamp = "{:%Y%m%d-%H%M%S}".format(datetime.datetime.now())

    cmd_args = ["bash", "debian/dockerscript.sh", "-S" if sign_only else None]
    runproc(
        "docker", "run", "--rm", "-it",
        "--name", f"{IMAGE_PREFIX}-{timestamp}",
        "-u", f"{os.getuid()}:{os.getgid()}",
        "-v", f"{abs_key_dir}:/var/debkeys",
        "-v", f"{SCRIPT_DIR.parent.parent}:/src",
        "-e", f"plugin_dir={SCRIPT_DIR.parent.name}",
        "-e", "DEBEMAIL",
        "-e", "EMAIL",
        image_name(dist_id, dist_codename),
        *cmd_args,
        )

def build_image(dist_id, dist_codename, tz):
    def build_args():
        args = [
            f"os={dist_id}",
            f"os_codename={dist_codename}",
            f"os_release={DISTRO_RELEASES[dist_codename]}"
                if dist_codename != "unstable" else None,
            f"tz={tz}",
            ]
        for arg in filter(None, args):
            yield "--build-arg"
            yield arg

    with chdir(SCRIPT_DIR):
        runproc(
            "docker", "build", "--pull",
            "-t", image_name(dist_id, dist_codename),
            *build_args(),
            "."
            )

def add_changelog(version, dist_id, dist_codename, dist_rev):
    version_suffix = ""
    if dist_codename != "unstable":
        dist_release = DISTRO_RELEASES[dist_codename]
        version_suffix = f"~{dist_id}{dist_release}.{dist_rev}"

    with chdir(SCRIPT_DIR.parent):
        runproc(
            "git", "commit", "--allow-empty",
            "-m", "Package for distribution",
            )
        runproc(
            "gbp", "dch",
            "-N", f"{version}{version_suffix}",
            f"--distribution={dist_codename}",
            "--release",
            "--spawn-editor=never",
            "--dch-opt=--force-bad-version",
            "--commit",
            )

def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        )
    parser.add_argument(
        "-B", "--build-image",
        action="store_true",
        help="build docker image",
        )
    parser.add_argument(
        "-N", "--no-run",
        action="store_true",
        help="do not run container",
        )
    parser.add_argument(
        "-S", "--sign-diff-only",
        action="store_true",
        help="sign source diff only",
        )
    parser.add_argument(
        "-P", "--ppa",
        metavar="VERSION",
        help="add changelog for ppa (eg, 0.1-1~ppa0)",
        )
    parser.add_argument(
        "-r", "--dist-rev",
        default="0",
        help="set distribution specific version part",
        )
    parser.add_argument(
        "--tz",
        default="Etc/UTC",
        help="set time zone",
        )
    parser.add_argument(
        "key_dir",
        help="directory containing signing key",
        )
    parser.add_argument(
        "dist_id",
        help="distro name (eg, debian or ubuntu)",
        )
    parser.add_argument(
        "dist_codename",
        help="distro release (eg, unstable or xenial)",
        )
    args = parser.parse_args()

    if args.ppa:
        add_changelog(version=args.ppa,
                      dist_id=args.dist_id,
                      dist_codename=args.dist_codename,
                      dist_rev=args.dist_rev)

    if args.build_image:
        build_image(dist_id=args.dist_id,
                    dist_codename=args.dist_codename,
                    tz=args.tz)

    if not args.no_run:
        run_container(dist_id=args.dist_id,
                      dist_codename=args.dist_codename,
                      key_dir=args.key_dir,
                      sign_only=args.sign_diff_only)

if __name__ == "__main__":
    main()
