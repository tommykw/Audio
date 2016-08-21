//
// Created by tomitakenji on 2016/08/19.
//

#include <android/log.h>
#include "opensl_io.h"

#define BUFFERFRAMES 1024
#define VECSAMPS_MONO 64
#define VECSAMPS_STEREO 128
#define SR 44100

static int on;
void start_process() {
    OPENSL_STREAM *p;
    int samps, i, j;
    float inbuffer[VECSAMPS_MONO], outbuffer[VECSAMPS_STEREO];
    p = android_OpenAudioDevice(SR, 1,2,BUFFERFRAMES);
}