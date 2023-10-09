# Changelog

## 0.3.4 - 2023-10-08
- Added support for loading RGB888 HEIF images.

## 0.3.3 - 2018-09-16
- Refactored for qtimageformats.
- Removed usage of C++ exceptions.
- Reduced libheif requirement to version 1.1.

## 0.3.2 - 2018-07-23
- Fixed error when building against older Qt.
- Minor code cleanup.

## 0.3.1 - 2018-07-22
- Removed an unnecessary copy when reading an image.
- Reduced language requirement to C++11 (from C++14).

## 0.3.0 - 2018-07-18
- Added ability to read multi-image files.
- Added helpers to assist with `deb` packaging.

## 0.2.1 - 2018-07-13
- Changed style to match Qt conventions.
- Changed format-checking code to be more robust.
- Reduced libheif requirement to version 1.2.

## 0.2.0 - 2018-06-26
- Added support for writing primary image.
- Fixed a large memory leak when creating a new image during read.
- Increased libheif requirement to version 1.3.

## 0.1.0 - 2018-06-04
- Initial release.
