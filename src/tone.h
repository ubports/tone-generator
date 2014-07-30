/*************************************************************************
This file is part of tone-generator

Copyright (C) 2010 Nokia Corporation.

This library is free software; you can redistribute
it and/or modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation
version 2.1 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
USA.
*************************************************************************/

#ifndef __TONEGEND_TONE_H__
#define __TONEGEND_TONE_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

#define PRESERVE_CHAIN   0
#define KILL_CHAIN       1

/*
 * predefined tone types
 */
#define TONE_UNDEFINED       0
#define TONE_DIAL            1
#define TONE_BUSY            2
#define TONE_CONGEST         3
#define TONE_RADIO_ACK       4
#define TONE_RADIO_NA        5
#define TONE_ERROR           6
#define TONE_WAIT            7
#define TONE_RING            8
#define TONE_DTMF_IND_L      9
#define TONE_DTMF_IND_H      10
#define TONE_DTMF_L          11
#define TONE_DTMF_H          12
#define TONE_NOTE_0          13
#define TONE_SINGEN_END      14

#define BACKEND_UNKNOWN      0
#define BACKEND_SINGEN       1


struct stream;
union  envelop;

struct singen {
    int64_t        m;
    int64_t        n0;
    int64_t        n1;
    int64_t        offs;
};


struct tone {
    struct tone       *next;
    struct stream     *stream;
    struct tone       *chain;
    int                type;
    uint32_t           period;   /* period (ie. play+pause) length */
    uint32_t           play;     /* how long to play the sine */
    uint64_t           start;
    uint64_t           end;
    int                backend;
    union {
        struct singen  singen;
    };
    int                reltime; /* relative time to be passed to env. func's */
    union envelop     *envelop;
};


int tone_init(int, char **);
struct tone *tone_create(struct stream *, int, uint32_t, uint32_t,
                         uint32_t, uint32_t, uint32_t, uint32_t);
void tone_destroy(struct tone *, int);
int tone_chainable(int);
uint32_t tone_write_callback(struct stream *, int16_t *, int);
void tone_destroy_callback(void *);


#endif /* __TONEGEND_TONE_H__ */


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
