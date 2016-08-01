//
// Created by tomitakenji on 2016/07/31.
//

#ifndef AUDIO_OPENSL_IO_H
#define AUDIO_OPENSL_IO_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct threadLock_{
  pthread_mutex_t m;
  pthread_cond_t c;
  unsigned char s;
} threadLock;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct opensl_stream {
  SLObjectItf engineObject;
  SLEngineItf engineEngine;

  SLObjectItf outputMixObject;
  SLObjectItf bgPlayerObject;
  SLPlayItf bgPlayerPlay;
  SLAndroidSimpleBufferQueueItf bgPlayerBufferQueue;
  SLEffetSendItf bgPlayerEffectSend;

  SLObjectItf recorderObject;
  SLRecordItf recorderRecord;
  SLAndroidSimpleBufferQueueItf recorderBufferQueue;
}

#endif //AUDIO_OPENSL_IO_H
