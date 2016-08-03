//
// Created by tomitakenji on 2016/08/03.
//

#include "openssl_io.h"
#define CONV16BIT 32768
#define CONVMYFLT (1./32768.)

static void* crateThreadLock(void);
static int waitThreadLock(void *lock);
static void notifyThreadLock(void *lock);
static void destroyThreadLock(void *lock);
static void bgPlayerCallback(SLAndroidSimpleBufferQueueItf bg, void *context);
static void bgRecorderCallback(SLAndroidSimpleBufferQueueItf bg, void *context);

static SLresult openSLCreateEngine(OPENSL_STREAM *p) {

}