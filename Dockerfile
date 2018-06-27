ARG os_release=xenial
FROM ubuntu:${os_release}

# user to run as
ARG uid=1000

# relative name of root project dir; bind parent of this to /src
ARG src_root_dir=qt-heif-image-plugin

RUN apt-get update && apt-get install -y software-properties-common && \
    add-apt-repository -y ppa:strukturag/libde265 && \
    add-apt-repository -y ppa:strukturag/libheif && \
    apt-get update && \
    apt-get install -y \
      cmake \
      debhelper \
      libheif-dev \
      pkg-config \
      qtbase5-dev \
      && \
    adduser --quiet --disabled-password --gecos "" --uid ${uid} builduser && \
    mkdir -p /src && \
    \
    apt-get clean && \
    true

USER builduser:builduser

VOLUME /src
WORKDIR /src/${src_root_dir}

CMD ["/usr/bin/dpkg-buildpackage", "-us", "-uc", "-b"]
