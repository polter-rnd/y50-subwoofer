/*
 * Utility for managing Lenovo Y50 subwoofer (based on hda-verb)
 * Version 0.1
 *
 * Copyright (c) 2016 Pavel Artsishevsky <polter.rnd@gmail.com>
 * Copyright (c) 2008 Takashi Iwai <tiwai@suse.de>
 *
 * Licensed under GPL v2 or later.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <alsa/asoundlib.h>
#include <alsa/control.h>

#include "hda_hwdep.h"



// Verb types used to setup LFE output
#define AC_VERB_SET_AMP_GAIN_MUTE      0x300
#define AC_VERB_SET_POWER_STATE        0x705
#define AC_VERB_SET_PIN_WIDGET_CONTROL 0x707

// Y50-specific defines
#define Y50_CTL_NAME "hw:1"
#define Y50_HWDEP_DEVICE "/dev/snd/hwC1D0"
#define Y50_LFE_VOLUME_MAX 85

// ALSA control names
#define MASTER_VOLUME_NAME    "Master Playback Volume"
#define HEADPHONE_SWITCH_NAME "Headphone Playback Switch"



// Global plugged flag
int headphones_plugged = 0;



static int open_ctl(const char *name, snd_ctl_t **ctlp)
{
    snd_ctl_t *ctl;

    if (snd_ctl_open(&ctl, name, SND_CTL_READONLY) < 0) {
        fprintf(stderr, "Cannot open ctl %s\n", name);
        return 1;
    }

    if (snd_ctl_subscribe_events(ctl, 1) < 0) {
        fprintf(stderr, "Cannot open subscribe events to ctl %s\n", name);
        snd_ctl_close(ctl);
        return 1;
    }

    *ctlp = ctl;
    return 0;
}



static int open_hwdep(const char* name, int* pfd) {
    int fd = open(name, O_RDWR);
    if (fd < 0) {
        perror("open");
        fprintf(stderr, "Cannot open hwdep device %s\n", name);
        return 1;
    }

    int version = 0;
    if (ioctl(fd, HDA_IOCTL_PVERSION, &version) < 0) {
        perror("ioctl(PVERSION)");
        fprintf(stderr, "Looks like an invalid hwdep device...\n");

        close(fd);
        return 1;
    }
    if (version < HDA_HWDEP_VERSION) {
        fprintf(stderr, "Invalid version number 0x%x\n", version);
        fprintf(stderr, "Looks like an invalid hwdep device...\n");

        close(fd);
        return 1;
    }

    *pfd = fd;

    return 0;
}



static void set_headphones_plugged(int* plugged)
{
    snd_mixer_t *handle;
    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, Y50_CTL_NAME);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_t *headphone_sid;
    const char *headphone_selem_name = "Headphone";

    snd_mixer_selem_id_alloca(&headphone_sid);
    snd_mixer_selem_id_set_index(headphone_sid, 0);
    snd_mixer_selem_id_set_name(headphone_sid, headphone_selem_name);

    snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, headphone_sid);
    snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_MONO, plugged);

    snd_mixer_close(handle);
}



static long get_master_volume()
{
    snd_mixer_t *handle;
    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, Y50_CTL_NAME);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_t *master_sid;
    const char *master_selem_name = "Master";

    snd_mixer_selem_id_alloca(&master_sid);
    snd_mixer_selem_id_set_index(master_sid, 0);
    snd_mixer_selem_id_set_name(master_sid, master_selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, master_sid);

    long min, max;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

    long volume;
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &volume);

    snd_mixer_close(handle);

    return volume * 100 / max;
}



static void set_lfe_volume(int fd, long volume)
{
    char lfe_volume = Y50_LFE_VOLUME_MAX * volume / 100;

    const int verbs[] = {
        HDA_VERB(0x03, AC_VERB_SET_AMP_GAIN_MUTE,
                 (lfe_volume > 0) ? (0xa0 << 8) | lfe_volume : 0),
        HDA_VERB(0x03, AC_VERB_SET_AMP_GAIN_MUTE,
                 (lfe_volume > 0) ? (0x90 << 8) | lfe_volume : 0),
        HDA_VERB(0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL,
                 (lfe_volume > 0) ? 0x25 : 0),
        0
    };

    for (int i = 0; verbs[i] > 0; ++i) {
        struct hda_verb_ioctl val;
        val.verb = verbs[i];
        if (ioctl(fd, HDA_IOCTL_VERB_WRITE, &val) < 0) {
            perror("ioctl");
            return;
        }
    }
}



static void lfe_initialize(int fd)
{
    struct hda_verb_ioctl val;

    const int verbs_init[] = {
        HDA_VERB(0x17, AC_VERB_SET_POWER_STATE,        0),
        HDA_VERB(0x1a, AC_VERB_SET_POWER_STATE,        0),
        HDA_VERB(0x17, AC_VERB_SET_AMP_GAIN_MUTE,      0xb000),
        HDA_VERB(0x17, AC_VERB_SET_PIN_WIDGET_CONTROL, 0x40),
        0
    };

    for (int i = 0; verbs_init[i] > 0; ++i) {
        val.verb = verbs_init[i];
        if (ioctl(fd, HDA_IOCTL_VERB_WRITE, &val) < 0) {
            perror("ioctl");
            return;
        }
    }
}




static int handle_event(int hwdep_fd, snd_ctl_t *ctl)
{
    snd_ctl_event_t *event;
    unsigned int mask;
    int err;

    snd_ctl_event_alloca(&event);
    err = snd_ctl_read(ctl, event);
    if (err < 0) {
        fprintf(stderr, "Cannot read event from ctl %s\n", snd_ctl_name(ctl));
        return err;
    }

    if (snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM) {
        return 0;
    }

    // Filter events
    mask = snd_ctl_event_elem_get_mask(event);
    if (mask & SND_CTL_EVENT_MASK_VALUE) {
        const char *event_name = snd_ctl_event_elem_get_name(event);

        if (!strcmp(event_name, HEADPHONE_SWITCH_NAME)) {
            // If headphones were plugged, mute LFE
            set_headphones_plugged(&headphones_plugged);
            if (headphones_plugged) {
                set_lfe_volume(hwdep_fd, 0);
            }
        } else if (!headphones_plugged && !strcmp(event_name, MASTER_VOLUME_NAME)) {
            // If headphones are unplugged and master volume changed, adjust LFE volume
            set_lfe_volume(hwdep_fd, get_master_volume());
        }
    }

    return 0;
}



int main(int argc, char **argv)
{
    int hwdep_fd;
    snd_ctl_t *ctl;

    (void) argc;
    (void) argv;

    // Open control and hwdep devices
    if (open_hwdep(Y50_HWDEP_DEVICE, &hwdep_fd) != 0
            || open_ctl(Y50_CTL_NAME, &ctl) != 0) {
        return 1;
    }

    // Initialize subwoofer
    lfe_initialize(hwdep_fd);

    // Set initial values
    set_headphones_plugged(&headphones_plugged);
    set_lfe_volume(hwdep_fd, headphones_plugged ? 0 : get_master_volume());

    // Poll events
    while (1) {
        struct pollfd fds;
        snd_ctl_poll_descriptors(ctl, &fds, 1);

        if (poll(&fds, 1, -1) <= 0) {
            perror("poll");
            return EXIT_FAILURE;
        }

        unsigned short revents;
        snd_ctl_poll_descriptors_revents(ctl, &fds, 1, &revents);
        if (revents & POLLIN) {
            handle_event(hwdep_fd, ctl);
        }
    }

    // Close resources
    close(hwdep_fd);
    snd_ctl_close(ctl);

    return EXIT_SUCCESS;
}
