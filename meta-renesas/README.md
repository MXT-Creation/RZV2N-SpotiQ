# meta-renesas

This is a Yocto build layer(version:dunfell) that provides support for the RZ/V2N Groups of 64bit Arm-based MPUs.
Currently the following boards and MPUs are supported:

- Board: RZ/V2N SMARC Evaluation Kit / MPU: R9A09G056 (RZ/V2N)
- Board: RZ/V2N Development Evaluation Kit / MPU: R9A09G056 (RZ/V2N)

## Patches

To contribute to this layer you should email patches to renesas-rz@renesas.com. Please send .patch files as email attachments, not embedded in the email body.

## Dependencies

This layer depends on:

    URI: git://git.yoctoproject.org/poky
    layers: meta, meta-poky, meta-yocto-bsp
    branch: scarthgap
    revision: 7117d115eab7351ecf21388ec720a3bb5f4a9b30

    URI: git://git.openembedded.org/meta-openembedded
    layers: meta-oe, meta-python, meta-multimedia
    branch: scarthgap
    revision: 3c293e14492f01e22a64004e2330fb620c27578a

    URI: https://git.yoctoproject.org/meta-arm
    layers: meta-arm
    branch: scarthgap
    revision: a8fe9d22eaefc294f91096c6a32663e2f4ab3b10

## Build Instructions

Assume that $WORK is the current working directory.
The following instructions require a Poky installation (or equivalent).

Below git configuration is required:
```bash
    $ git config --global user.email "you@example.com"
    $ git config --global user.name "Your Name"
```

Download proprietary graphics and multimedia drivers from Renesas.
To download Multimedia and Graphics library and related Linux drivers, please use the following link:

    English: https://www.renesas.com/us/en/products/microcontrollers-microprocessors/rz-mpus/rzg-linux-platform/rzg-marketplace/verified-linux-package/rzg-verified-linux-package
    Japanese: https://www.renesas.com/jp/ja/products/microcontrollers-microprocessors/rz-mpus/rzg-linux-platform/rzg-marketplace/verified-linux-package/rzg-verified-linux-package

Please choose correct packages that matches with your MPU.
Graphic drivers are required for Wayland. Multimedia drivers are optional.
After downloading the proprietary package, please decompress them then put meta-rz-features folder at $WORK.

Below is the combination of Codec/Graphics library with BSP released versions:

**1. RZ/V2N**

|BSP Version|Codec Version|Graphics Version|
|:---------:|:-----------:|:--------------:|
|2.0.0| <TBD> | <TBD> |

You can get all Yocto build environment from Renesas, or download all Yocto related public source to prepare the build environment as below.
```bash
    $ git clone https://git.yoctoproject.org/git/poky
    $ cd poky
    $ git checkout 7117d115eab7351ecf21388ec720a3bb5f4a9b30
    $ cd ..
    $
    $ git clone https://github.com/openembedded/meta-openembedded
    $ cd meta-openembedded
    $ git checkout 3c293e14492f01e22a64004e2330fb620c27578a
    $ cd ..
    $
    $ git clone https://github.com/openembedded/meta-arm
    $ cd meta-openembedded
    $ git checkout a8fe9d22eaefc294f91096c6a32663e2f4ab3b10
    $ cd ..
    $
    $ git clone  https://github.com/renesas-rz/meta-renesas.git
    $ cd meta-renesas
    $ git checkout <tag>
    $ cd ..
```
\<tag\> can be selected in any tags of meta-renesas.

**1. Build procedure (Recommended):**
- Initialize a build using the 'oe-init-build-env' script in Poky and point TEMPLATECONF to platform conf path. e.g.:
   ```bash
   $ TEMPLATECONF=$PWD/meta-renesas/meta-rz-boards/conf/templates/rz-conf/ source poky/oe-init-build-env build
   ```
- To build optional features (Codec or Graphics), add necessary layers:
   ```bash
   # For Codec
   $ bitbake-layers add-layer ../meta-rz-features/meta-rz-codecs

   # For Graphics
   $ bitbake-layers add-layer ../meta-rz-features/meta-rz-graphics

   ```
- Build the target file system image using bitbake:
   ```bash
   $ MACHINE=<board> bitbake core-image-<target>
   ```
\<platform\> and \<board\> can be selected in below table:

|Renesas MPU| platform |        board           |
|:---------:|:--------:|:----------------------:|
|RZ/V2N     |rzv2n     |rzv2n-evk, rzv2n-dev  |

\<target\> for these builds:
* RZ/V2N: bsp, weston

After completing the images for the target machine will be available in the output
directory _'tmp/deploy/images/\<supported board name\>'_.

Images generated:
* Image (generic Linux Kernel binary image file)
* DTB for target machine
* core-image-\<target\>-\<machine name\>.tar.bz2 (rootfs tar+bzip2)
* core-image-\<target\>-\<machine name\>.ext4  (rootfs ext4 format)
* core-image-\<target\>-\<machine name\>.wic.gz  (rootfs wic gz format)
* core-image-\<target\>-\<machine name\>.wic.bmap  (rootfs wic block map format)

## Build Instructions for SDK

Use bitbake -c populate_sdk for generating the toolchain SDK:
For 64-bit target SDK (aarch64):
```bash
    $ bitbake core-image-weston -c populate_sdk
```
The SDK can be found in the output directory _'tmp/deploy/sdk'_

    poky-glibc-x86_64-core-image-weston-aarch64-toolchain-x.x.sh

Usage of toolchain SDK: Install the SDK to the default: _/opt/poky/x.x_
For 64-bit target SDK:
```bash
    $ sh poky-glibc-x86_64-core-image-weston-aarch64-toolchain-x.x.sh
```
For 64-bit application use environment script in _/opt/poky/x.x_
```bash
    $ source /opt/poky/x.x/environment-setup-aarch64-poky-linux
```

* WIC image: deploy disk images format. It is enabled by default in local.conf. To disable it, please comment out or set 0 to below setting:
  ```
  WKS_SUPPORT ?= "1"
  ```
  If you do not want to use default wic image file, please update "WKS_DEFAULT_FILE" or "WKS_FILE" to your desirable file.
