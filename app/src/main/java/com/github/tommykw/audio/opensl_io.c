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
  SLresult result;
  result = slCreateEngine(&(p->engineObject), 0, NULL, 0, NULL, NULL);
  if (result != SL_RESULT_SUCCESS) goto engine_end;

  result = (*p->engineObject)->GetInterface(p->engineObject, SL_IID_ENGINE, &(p->engineEngine));
  if (result != SL_RESULT_SUCCESS) goto engine_end;

  result = (*p->engineObject)->GetInterface(p->engineObject, SL_IID_ENGINE, &(p->engineEngine));
  if (result != SL_RESULT_SUCCESS) goto engine_end;

  engine_end;
  return result;
}

static SLresult openSLPlayOpen(OPENSL_STREAM *p) {
  SLresult result;
  SLunit32 sr = p->sr;
  SLunit32 channels = p->outchannels;

  if (channels) {
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq =
        {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2}

    switch (sr) {
        case 8000:
            sr = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            sr = SL_SAMPLINGRATE_11_025;
            break;
        case 16000:
            sr = SL_SAMPLINGRATE_16;
            break;
    }
  }
}