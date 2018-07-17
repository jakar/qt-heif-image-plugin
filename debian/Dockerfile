ARG os=ubuntu
ARG os_release=xenial
FROM ${os}:${os_release}

ARG os
ARG os_release

# skip tzdata prompt
ARG tz=Etc/UTC

# relative name of root project dir; bind parent of this to /src
ENV plugin_dir=qt-heif-image-plugin

RUN set -ex; \
    ln -snf "/usr/share/zoneinfo/${tz}" /etc/localtime; \
    echo "${tz}" > /etc/timezone; \
    if [ "$os" = "ubuntu" ]; then \
      apt-get update; \
      apt-get install -y software-properties-common; \
      add-apt-repository -y ppa:strukturag/libheif; \
    fi; \
    apt-get update; \
    apt-get install -y \
      cmake \
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

WORKDIR /src/${plugin_dir}
CMD ["/bin/bash", "debian/dockerscript.sh"]
