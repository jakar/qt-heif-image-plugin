ARG os=debian
ARG os_codename=unstable

FROM ${os}:${os_codename}

ARG os
ARG os_release
ARG os_codename

# skip tzdata prompt
ARG tz=Etc/UTC

RUN set -ex; \
    ln -snf "/usr/share/zoneinfo/${tz}" /etc/localtime; \
    echo "${tz}" > /etc/timezone; \
    apt-get update; \
    if [ ${os} = ubuntu ] && dpkg --compare-versions "${os_release}" le 16.04; \
    then \
      apt-get install -y software-properties-common; \
      add-apt-repository -y ppa:strukturag/libheif; \
      apt-get update; \
    fi; \
    if [ ${os} = ubuntu ] && dpkg --compare-versions "${os_release}" le 14.04; \
    then \
      add-apt-repository -y ppa:strukturag/libde265; \
      apt-get update; \
      apt-get install -y cmake3; \
    else \
      apt-get install -y cmake; \
    fi; \
    apt-get install -y \
      debhelper \
      git-buildpackage \
      libheif-dev \
      pkg-config \
      qtbase5-dev \
      ; \
    mkdir -p /src /var/debkeys; \
    apt-get clean; \
    true

VOLUME /src
VOLUME /var/debkeys

# relative name of root project dir; bind parent of this to /src
ENV plugin_dir=qt-heif-image-plugin

WORKDIR /src/${plugin_dir}
CMD ["/bin/bash", "debian/dockerscript.sh"]

# vim:sw=2
