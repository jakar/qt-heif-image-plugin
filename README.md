# qt-heif-image-plugin: HEIF Image Plugin for Qt
This is a Qt plugin for [HEIF] images, wrapping [libheif]. It enables existing
Qt applications to open and save HEIF images, such as those from Apple devices.

[HEIF]: https://en.wikipedia.org/wiki/High_Efficiency_Image_File_Format
[libheif]: https://github.com/strukturag/libheif

Currently, support is limited to basic reading and writing of the primary
image. Metadata is not handled at all.

## Dependencies
- Qt 5 (Core and GUI modules)
- libheif ([source](https://github.com/strukturag/libheif),
  [PPA](https://launchpad.net/~strukturag/+archive/ubuntu/libheif))
- libde265 (transitive dependency of libheif;
  [source](https://github.com/strukturag/libde265),
  [PPA](https://launchpad.net/~strukturag/+archive/ubuntu/libde265))

## Compilation
Build with CMake:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Then, to install:
```
$ sudo make install
```

The built library will be installed to `$QTDIR/plugins/imageformats`. Beware
that `CMAKE_INSTALL_PREFIX` will not be honored. Qt only searches in select
directories for plugins.

Alternatively, to use with a specific application, place the plugin in
`$APPDIR/imageformats`, where `$APPDIR` is the directory containing the
application's binary.

## Usage
Any application that (directly or indirectly) uses `QImageReader` to open image
files should automatically be able to use this plugin.

So far, this has been successfully used with
[LXImage-Qt](https://github.com/lxqt/lximage-qt).
