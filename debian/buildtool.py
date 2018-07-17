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
        image_name(dist_id, dist_codename),
        *cmd_args,
        )

def build_image(dist_id, dist_codename, tz):
    with chdir(SCRIPT_DIR):
        runproc(
            "docker", "build",
            "-t", image_name(dist_id, dist_codename),
            "--build-arg", f"os={dist_id}",
            "--build-arg", f"os_release={dist_codename}",
            "--build-arg", f"tz={tz}",
            "."
            )

def add_changelog(version, dist_codename, dist_rev):
    with chdir(SCRIPT_DIR.parent):
        runproc("git", "commit", "--allow-empty", "-m", "Package for PPA")
        runproc(
            "gbp", "dch",
            "-N", f"{version}~{dist_codename}{dist_rev}",
            f"--distribution={dist_codename}", "-R",
            "--spawn-editor=never",
            )
        runproc("git", "commit", "-a", "-m", "Add PPA changelog")

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
        help="add changelog for ppa (eg, 0.1-1ppa1)",
        )
    parser.add_argument(
        "-r", "--dist-rev",
        default="1",
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
