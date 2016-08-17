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

  if(channels){

    switch(sr){

    case 8000:
      sr = SL_SAMPLINGRATE_8;
      break;
    case 11025:
      sr = SL_SAMPLINGRATE_11_025;
      break;
    case 16000:
      sr = SL_SAMPLINGRATE_16;
      break;
    case 22050:
      sr = SL_SAMPLINGRATE_22_05;
      break;
    case 24000:
      sr = SL_SAMPLINGRATE_24;
      break;
    case 32000:
      sr = SL_SAMPLINGRATE_32;
      break;
    case 44100:
      sr = SL_SAMPLINGRATE_44_1;
      break;
    case 48000:
      sr = SL_SAMPLINGRATE_48;
      break;
    case 64000:
      sr = SL_SAMPLINGRATE_64;
      break;
    case 88200:
      sr = SL_SAMPLINGRATE_88_2;
      break;
    case 96000:
      sr = SL_SAMPLINGRATE_96;
      break;
    case 192000:
      sr = SL_SAMPLINGRATE_192;
      break;
    default:
      return -1;
    }

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
				      SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};

    // configure audio sink
    int speakers;
    if(channels > 1)
      speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    else speakers = SL_SPEAKER_FRONT_CENTER;
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, sr,
				   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
				   speakers, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*p->engineEngine)->CreateAudioRecorder(p->engineEngine, &(p->recorderObject), &audioSrc,
						     &audioSnk, 1, id, req);
    if (SL_RESULT_SUCCESS != result) goto end_recopen;

    // realize the audio recorder
    result = (*p->recorderObject)->Realize(p->recorderObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) goto end_recopen;

    // get the record interface
    result = (*p->recorderObject)->GetInterface(p->recorderObject, SL_IID_RECORD, &(p->recorderRecord));
    if (SL_RESULT_SUCCESS != result) goto end_recopen;

    // get the buffer queue interface
    result = (*p->recorderObject)->GetInterface(p->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
						&(p->recorderBufferQueue));
    if (SL_RESULT_SUCCESS != result) goto end_recopen;

    // register callback on the buffer queue
    result = (*p->recorderBufferQueue)->RegisterCallback(p->recorderBufferQueue, bqRecorderCallback,
							 p);
    if (SL_RESULT_SUCCESS != result) goto end_recopen;
    result = (*p->recorderRecord)->SetRecordState(p->recorderRecord, SL_RECORDSTATE_RECORDING);

  end_recopen:
    return result;
  }
  else return SL_RESULT_SUCCESS;
}

static void openSLDestroyEngine(OPENSL_STREAM *p) {
  if (p->bqPlayerObject != NULL) {
    (*p->bgPlayerObject)->Destroy(p->bgPlayerObject);
    p->bgPlayerObject = NULL;
    p->bgPlayerPlay = NULL;
    p->bgPlayerBufferQueue = NULL;
    p->bgPlayerEffectSend = NULL;
  }

  if (p->recorderObject != NULL) {
    (*p->recorderObject)->Destory(p->recorderObject);
    p->recorderObject = NULL;
    p->recorderRecord = NULL;
    p->recorderBufferQueue = NULL;
  }

  if (p->outputMixObject != NULL) {
    (*p->outputMixObjct)->Destroy(p->outputMixObject);
    p->outputMixObject = NULL;
  }

  if (p->engineObject != NULL) {
    (*p->engineObject)->Destroy(p->engineObject);
    p->engineObject = NULL;
    p->engineEngine = NULL;
  }
}

OPENSL_STREAM *android_OpenAudioDevice(int sr,
                                       int inchannels,
                                       int outchannels,
                                       int bufferframes) {
    OPENSL_STREAM *p;
    p = (OPENSL_STREAM *) calloc(sizeof(OPENSL_STREAM), 1);
    p->inchannels = inchannels;
    p->outchannels = outchannels;
    p->sr = sr;
    p->inlock = createThreadLock();
}

void* createThreadLock(void) {
    threadLock *p;
    p = (threadLock*) malloc(sizeof(thradLock));
    if (p == NULL) {
        return NULL:
    }
    memset(p, 0, sizeof(threadLock));
    if (pthread_mutex_init(&(p->m), (pthread_mutexattr_t*) NULL) != 0) {
        free((void*) p);
        return NULL:
    }
    if (pthread_cond_init(&(p->c), (pthread_condattr_t*) NULL) != 0) {
        pthread_mutex_destroy()
        free((void*) p);
        return NULL;
    }
    p->s = (unsigned char) 1;
    return p;
}

int waitThreadLock(void *lock) {
    threadLock *p;
    int retval = 0;
    p = (threadLock*) lock;
    pthread_mutex_lock(&(p->m));
    while (!p->s) {
        pthread_cond_wait(&(p->c), &(p->m))
    }
    p->s = (unsgined char) 0;
    pthread_mutex_unlock(&(p->m));
}

void notifyThreadLock(void *lock) {
    threadLock *p;
    p = (threadLock*) lock;
    pthread_mutex_lock(&(p->m));
    p->s = (unsigned char) 1;
    pthread_cond_signal(&(p->c));
    pthread_mutex_unlock(&(p->m));
}

void destroyThreadLock(void *lock) {
    threadLock *p;
    p = (threadLock*) lock;
    if (p == NULL) {
        return;
    }
    notifyThreadLock(p);
    pthread_cond_destroy(&(p->c));
    pthraed_mutex_destroy(&(p->m));
    free(p);
}