# Y50-subwoofer
Small daemon for Linux that automatically adjustes Lenovo Y50 subwoofer parameters in background.

By default subwoofer in this laptop does not work on Linux at all. This application automatically turns it on when needed and sets it volume accorting to the master channel volume.

Main features:
  - Turns subwoofer on when headphones are not plugged;
  - Turns subwoofer off when headphones are plugged;
  - Adjustes subwoofer volume when master channel volume changes.

### Version
0.1

### Installation
To build you need only C compiler and development files for ALSA library (**gcc** and **alsa-lib-devel**).

```sh
$ git clone https://github.com/polter-rnd/y50-subwoofer.git
$ cd y50-subwoofer
$ make install
$ service y50-subwoofer start
```
