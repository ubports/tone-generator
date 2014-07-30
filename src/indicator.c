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

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <log/log.h>
#include <trace/trace.h>

#include "ausrv.h"
#include "stream.h"
#include "tone.h"
#include "indicator.h"

#define MAX_TONE_LENGTH (1 * 60 * 1000000)

/* Special case for shorter tones, because we don't want to keep a silent
 * stream around for a long time after playing the actual tone. */
#define MAX_SHORT_TONE_LENGTH (1 * 5 * 1000000)

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

static char     *ind_stream = STREAM_INDICATOR;
static int       standard   = STD_CEPT;
static void     *ind_props  = NULL;
static uint32_t  vol_scale  = 100;

int indicator_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}


void indicator_play(struct ausrv *ausrv, int type, uint32_t vol, int dur)
{
    struct stream *stream  = stream_find(ausrv, ind_stream);
    uint32_t       timeout = dur ? dur : MAX_TONE_LENGTH;
    
    if (stream != NULL) {
        dtmf_stop(ausrv);
        indicator_stop(ausrv, PRESERVE_STREAM);
    }
    else {
        stream = stream_create(ausrv, ind_stream, NULL, 0,
                               tone_write_callback,
                               tone_destroy_callback,
                               ind_props,
                               NULL);

        if (stream == NULL) {
            LOG_ERROR("%s(): Can't create stream", __FUNCTION__);
            return;
        }
    }

    vol = (vol_scale * vol) / 100;
    
    switch (type) {
        
    case TONE_DIAL:
        switch (standard) {
        case STD_CEPT:
            tone_create(stream, type, 425, vol, 1000000, 1000000, 0,0);
            break;
        case STD_ANSI:
        case STD_ATNT:
            tone_create(stream, type, 350, (vol*7)/10, 1000000, 1000000, 0,0);
            tone_create(stream, type, 440, (vol*7)/10, 1000000, 1000000, 0,0);
            break;
        case STD_JAPAN:
            tone_create(stream, type, 400, vol, 1000000, 1000000, 0,0);
            break;
        }
        timeout = MAX_TONE_LENGTH;
        break;
        
    case TONE_BUSY:
        switch (standard) {
        case STD_CEPT:
            tone_create(stream, type, 425, vol, 1000000, 500000, 0,dur);
            break;
        case STD_ANSI:
        case STD_ATNT:
            tone_create(stream, type, 480, (vol*7)/10, 1000000, 500000, 0,dur);
            tone_create(stream, type, 620, (vol*7)/10, 1000000, 500000, 0,dur);
            break;
        case STD_JAPAN:
            tone_create(stream, type, 400, vol, 1000000, 500000, 0,dur);
            break;
        }
        break;
        
    case TONE_CONGEST:
        switch (standard) {
        case STD_CEPT:
            tone_create(stream, type, 425, vol, 400000, 200000, 0,dur);
            break;
        case STD_ANSI:
        case STD_ATNT:
            tone_create(stream, type, 480, (vol*7)/10, 500000, 250000, 0,dur);
            tone_create(stream, type, 620, (vol*7)/10, 500000, 250000, 0,dur);
            break;
        case STD_JAPAN:
            /*
             * this is non-standard, but
             * we play busy tone instead of being silent
             */
            tone_create(stream, type, 400, vol, 1000000, 500000, 0,dur);
            break;
        }
        break;
        
    case TONE_RADIO_ACK:
        switch (standard) {
        case STD_CEPT:
        case STD_ANSI:
        case STD_ATNT:
            tone_create(stream, type, 425, vol, 200000, 200000, 0,200000);
            timeout = MAX_SHORT_TONE_LENGTH;
            break;
        case STD_JAPAN:
            tone_create(stream, type, 400, vol, 3000000, 1000000, 0,0);
            /* The Japan standard tone is repeating, so I guess we need to wait 60s anyway. */
            timeout = MAX_TONE_LENGTH;
            break;
        }
        break;
        
    case TONE_RADIO_NA:
        switch (standard) {
        case STD_CEPT:
        case STD_ANSI:
        case STD_ATNT:
            tone_create(stream, type, 425, vol, 400000, 200000, 0,1200000);
            break;
        case STD_JAPAN:
            break;
        }
        timeout = MAX_SHORT_TONE_LENGTH;
        break;
        
    case TONE_ERROR:
        switch (standard) {
        case STD_CEPT:
        case STD_ANSI:
        case STD_ATNT:
            tone_create(stream, type,  900, vol, 2000000, 333333, 0,dur);
            tone_create(stream, type, 1400, vol, 2000000, 332857, 333333,dur);
            tone_create(stream, type, 1800, vol, 2000000, 300000, 666190,dur);
            break;
        case STD_JAPAN:
            /*
             * this is non-standard, but
             * we play busy tone instead of being silent
             */
            tone_create(stream, type, 400, vol, 1000000, 500000, 0,dur);
            break;
        }
        break;
        
    case TONE_WAIT:
        switch (standard) {
        case STD_CEPT:
            tone_create(stream,type, 425, vol, 800000,200000, 0,1000000);
            tone_create(stream,type, 425, vol, 800000,200000, 4000000,1000000);
            break;
        case STD_ANSI:
            tone_create(stream,type, 440, vol, 300000,300000, 0,300000);
            tone_create(stream,type, 440, vol, 10000000,100000, 10000000,0);
            tone_create(stream,type, 440, vol, 10000000,100000, 10200000,0);
            break;
        case STD_ATNT:
            tone_create(stream, type, 440, vol, 4000000, 200000, 0, 0);
            tone_create(stream, type, 440, vol, 4000000, 200000, 500000, 0);
            break;
        case STD_JAPAN:
            break;
        }
        timeout = MAX_TONE_LENGTH;
        break;
        
    case TONE_RING:
        switch (standard) {
        case STD_CEPT:
            tone_create(stream, type, 425, vol, 5000000, 1000000, 0,0);
            break;
        case STD_ANSI:
        case STD_ATNT:
            tone_create(stream, type, 440, (vol*7)/10, 6000000, 2000000, 0,0);
            tone_create(stream, type, 480, (vol*7)/10, 6000000, 2000000, 0,0);
            break;
        case STD_JAPAN:
            break;
        }
        timeout = MAX_TONE_LENGTH;
        break;
        
    default:
        LOG_ERROR("%s(): invalid type %d", __FUNCTION__, type);
        break;
    }

    stream_set_timeout(stream, timeout);
}

void indicator_stop(struct ausrv *ausrv, int kill_stream)
{
    struct stream *stream = stream_find(ausrv, ind_stream);
    struct tone   *tone;
    struct tone   *hd;

    TRACE("%s(kill_stream=%s) stream=%s", __FUNCTION__, 
          kill_stream ? "true":"false", stream ? stream->name:"<no-stream>");
    
    if (stream != NULL) {
        if (kill_stream) 
            stream_destroy(stream);
        else {
            /* destroy all but DTMF tones */
            for (hd = (struct tone *)&stream->data;  hd;  hd = hd->next) {
                while ((tone=hd->next) != NULL && !tone_chainable(tone->type))
                    tone_destroy(tone, KILL_CHAIN);
            }
        }
    }
}

void indicator_set_standard(int std)
{
    if (std <= STD_UNKNOWN || std >= STD_MAX)
        LOG_ERROR("%s(): invalid standard %d", __FUNCTION__, std);
    else
        standard = std;
}

void indicator_set_properties(char *propstring)
{
    ind_props = stream_parse_properties(propstring);
}

void indicator_set_volume(uint32_t volume)
{
    vol_scale = volume;
}


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
