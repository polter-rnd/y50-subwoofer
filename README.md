# Y50-subwoofer

// Hi! First i really REALLY thank you for this fix. I tryed to contact you via email from your copr page but you seem to not respond. I have a problem with the subwoofer and i would really like to fix it with your help. So the problem is this: When i suspend my laptop(or hibernate it) the subwoofer stops working again. the service is still running but it doesnt make any changes on subwoofer levels. Im using pulseaudio and i think something is restricting the acces of the service to make changes after suspend. Please help me! Contact me @ todorgeorgiev96@gmail.com


Small daemon for Linux that automatically adjusts Lenovo Y50 subwoofer parameters in background.

By default subwoofer in this laptop does not work on Linux at all. This application automatically turns it on when needed and sets it volume according to the master channel volume.

Main features:
  - Turns subwoofer on when headphones are not plugged;
  - Turns subwoofer off when headphones are plugged;
  - Adjusts subwoofer volume when master channel volume changes.

### Version
0.1.4

### Installation
COPR for Fedora: https://copr.fedorainfracloud.org/coprs/polter/y50-subwoofer

To build you need only C compiler and development files for ALSA library (**gcc** and **alsa-lib-devel**).

```sh
$ git clone https://github.com/polter-rnd/y50-subwoofer.git
$ cd y50-subwoofer
$ make install
$ service y50-subwoofer start
```
