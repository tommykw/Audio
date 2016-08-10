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
        default:
            return -1;
        }

        const SLInterfaceID ids[] = {SL_IID_VOLUME};
        const SLboolean req[] = {SL_BOOLEAN_FALSE};
        result = (*p->engineEngine)->CreateOutputMix(p->engineEngine, &(p->outputMixObject), 1, ids, req);
        if (result != SL_RESULT_SUCCESS) goto end_openaudio;

        result = (*p->outputMixObject)->Realize(p->outputMixObject, SL_BOOLEAN_FALSE);

        int speakers;
        if (channels > 1) {
            speakers = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        } else {
            speakers = SL_SPEAKER_FRONT_CENTER;
            SLDataFormat_PCM format_pcm = {
                SL_DATAFORMAT_PCM, channels, sr,
                SL_PCMSAMPLEFORMAT_FIXED_16, SLPCMSAMPLEFORMAT_FIXED_16,
                speakers, SL_BYTEORDER_LITTLEENDIAN
            };
        }
        SLDataSource audioSrc = {&loc_bufq, &format_pcm};

        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, p->outputMixObject};
        SLDataSink audioSink = {&loc_outmix, NULL};

        const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE}
        const SLboolean req[1] = {SL_BOOLEAN_TRUE}
        result = (*p->enqineEngine)->CreateAudioRecorder(
            p->engineEnqine, &(p->recorderObject), &audioSrc, &audioSnk, 1, id, req);

        if (SL_RESULT_SUCCESS != result) goto end_recopen;

        result = (*p->recorderObject)->Realize(p->recorderObject, SL_BOOLEAN_FALSE);
        if (SL_RESULT_SUCCESS != result) goto end_recopen;

        result = (*p->recorderObject)->GetInterface(p->recoderObject, SL_IID_RECORD, &(p->recorderRecord));
        if (SL_RESULT_SUCCESS != result) goto end_recopen;

        result = (*p->recorderObject)->GetInterface(p->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &(p->recorderBufferQueue));
        if (SL_RESULT_SUCCESS != result) goto end_recopen;

        result = (*p->recoderBufferQueue)->RegisterCallback(p->recorderBufferQueue, bgRecorderCallback, p);
        if (SL_RESULT_SUCCESS != result) goto end_recopen;
        result = (*p->recorderRecord)->SetRecordState(p->recorderRecord, SL_RECORDSTATE_RECORDING);





    }
  }
}