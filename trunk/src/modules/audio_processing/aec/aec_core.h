/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * Specifies the interface for the AEC core.
 */

#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AEC_MAIN_SOURCE_AEC_CORE_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AEC_MAIN_SOURCE_AEC_CORE_H_

#include <stdio.h>

#include "signal_processing_library.h"
#include "typedefs.h"

#define FRAME_LEN 80
#define PART_LEN 64 // Length of partition
#define PART_LEN1 (PART_LEN + 1) // Unique fft coefficients
#define PART_LEN2 (PART_LEN * 2) // Length of partition * 2
#if (DITECH_VERSION==DITECH_ORIGINAL)
#define NR_PART 12
#endif
#if (DITECH_VERSION==DITECH_RELEASE_VERSION || DITECH_VERSION==2)
#define NR_PART 6 // Number of partitions
#endif
#define FILT_LEN (PART_LEN * NR_PART) // Filter length
#define FILT_LEN2 (FILT_LEN * 2) // Double filter length
#define FAR_BUF_LEN 12*64*4//(FILT_LEN2 * 2)
#define PREF_BAND_SIZE 24

// Delay estimator constants, used for logging.
enum { kMaxDelayBlocks = 60 };
enum { kLookaheadBlocks = 15 };
enum { kHistorySizeBlocks = kMaxDelayBlocks + kLookaheadBlocks };

typedef float complex_t[2];
// For performance reasons, some arrays of complex numbers are replaced by twice
// as long arrays of float, all the real parts followed by all the imaginary
// ones (complex_t[SIZE] -> float[2][SIZE]). This allows SIMD optimizations and
// is better than two arrays (one for the real parts and one for the imaginary
// parts) as this other way would require two pointers instead of one and cause
// extra register spilling. This also allows the offsets to be calculated at
// compile time.

// Metrics
enum {offsetLevel = -100};

typedef struct {
    float sfrsum;
    int sfrcounter;
    float framelevel;
    float frsum;
    int frcounter;
    float minlevel;
    float averagelevel;
} power_level_t;

typedef struct {
    float instant;
    float average;
    float min;
    float max;
    float sum;
    float hisum;
    float himean;
    int counter;
    int hicounter;
} stats_t;

typedef struct {
    int farBufWritePos, farBufReadPos;

	int knownDelay;
#if (DITECH_VERSION==1)
#endif
#if (DITECH_VERSION==2)
	int adaptIsOff;//nsinha this variable will control if adaptation of filter needs to de done.is turned off on discontinuity of time gap while calling.		
	
	int knownDelay_background;
	int last_knownDelay_background;
	float xfBuf_background[2][NR_PART * PART_LEN1]; // farend fft buffer
	float wfBuf_background[2][NR_PART * PART_LEN1]; // filter fft

	float xBuf_background[PART_LEN2]; // farend
    float dBuf_background[PART_LEN2]; // nearend
    float eBuf_background[PART_LEN2]; // error
	void *farFrBuf_background,*nearFrBuf_background,*outFrBuf_background;
	int   xfBufBlockPos_background;
	float background_lt_filteredop_power;
	float foreground_lt_filteredop_power;
	float lt_input_power;
	float framePowerAtProbableEchoDelay_shortTerm;
#endif
    int inSamples, outSamples;
    int delayEstCtr;

    void *farFrBuf, *nearFrBuf, *outFrBuf;

    void *nearFrBufH;
    void *outFrBufH;

    float xBuf[PART_LEN2]; // farend
    float dBuf[PART_LEN2]; // nearend
    float eBuf[PART_LEN2]; // error

    float dBufH[PART_LEN2]; // nearend

    float xPow[PART_LEN1];
    float dPow[PART_LEN1];
    float dMinPow[PART_LEN1];
    float dInitMinPow[PART_LEN1];
    float *noisePow;

    float xfBuf[2][NR_PART * PART_LEN1]; // farend fft buffer
    float wfBuf[2][NR_PART * PART_LEN1]; // filter fft
    complex_t sde[PART_LEN1]; // cross-psd of nearend and error
    complex_t sxd[PART_LEN1]; // cross-psd of farend and nearend
    complex_t xfwBuf[NR_PART * PART_LEN1]; // farend windowed fft buffer

    float sx[PART_LEN1], sd[PART_LEN1], se[PART_LEN1]; // far, near and error psd
    float hNs[PART_LEN1];
    float hNlFbMin, hNlFbLocalMin;
    float hNlXdAvgMin;
    int hNlNewMin, hNlMinCtr;
    float overDrive, overDriveSm;
    float targetSupp, minOverDrive;
    float outBuf[PART_LEN];
    int delayIdx;

    short stNearState, echoState;
    short divergeState;

    int xfBufBlockPos;
#if (DITECH_VERSION==DITECH_ORIGINAL)

    short farBuf[FILT_LEN2 * 2];
#endif

#if (DITECH_VERSION==2 || DITECH_VERSION==DITECH_RELEASE_VERSION)
	short farBuf[FAR_BUF_LEN];
#endif

    short mult; // sampling frequency multiple
    int sampFreq;
    WebRtc_UWord32 seed;

    float mu; // stepsize
    float errThresh; // error threshold

    int noiseEstCtr;

    power_level_t farlevel;
    power_level_t nearlevel;
    power_level_t linoutlevel;
    power_level_t nlpoutlevel;

    int metricsMode;
    int stateCounter;
    stats_t erl;
    stats_t erle;
    stats_t aNlp;
    stats_t rerl;

    // Quantities to control H band scaling for SWB input
    int freq_avg_ic;         //initial bin for averaging nlp gain
    int flag_Hband_cn;      //for comfort noise
    float cn_scale_Hband;   //scale for comfort noise in H band

    int delay_histogram[kHistorySizeBlocks];
    int delay_logging_enabled;
    void* delay_estimator;

#ifdef WEBRTC_AEC_DEBUG_DUMP
    FILE *farFile;
    FILE *nearFile;
    FILE *outFile;
    FILE *outLinearFile;
	FILE *filterFile0;
	FILE *filterFile1;
#endif
} aec_t;


#if (DITECH_VERSION==2)
//all data in terms of 16k
#define DNS_RATE 8
#define HUNDRED_MS_IN_16K_DNS 200
#define DNS_FRAME_LEN FRAME_LEN/DNS_RATE
typedef struct {
    int farBufWritePos, farBufReadPos;


	int adaptIsOff;//nsinha this variable will control if adaptation of filter needs to de done.is turned off on discontinuity of time gap while calling.		
    int inSamples, outSamples;
    int delayEstCtr;

    void *farFrBuf, *nearFrBuf, *outFrBuf;


    float xBuf[PART_LEN2]; // farend
    float dBuf[PART_LEN2]; // nearend
    float eBuf[PART_LEN2]; // error

    float dBufH[PART_LEN2]; // nearend

    float xPow[PART_LEN1];
    float dPow[PART_LEN1];
    float dMinPow[PART_LEN1];
    float dInitMinPow[PART_LEN1];
    float *noisePow;

    float xfBuf[2][NR_PART * PART_LEN1]; // farend fft buffer
    float wfBuf[2][NR_PART * PART_LEN1]; // filter fft
 
    float outBuf[PART_LEN];
    int delayIdx;

    short stNearState, echoState;
    short divergeState;

    int xfBufBlockPos;

    float farBuf[HUNDRED_MS_IN_16K_DNS+DNS_FRAME_LEN];
	float instantaneous_cor_buff[HUNDRED_MS_IN_16K_DNS];
	float long_term_cor_buff[HUNDRED_MS_IN_16K_DNS];
	float long_term_corr_buff_along_states[HUNDRED_MS_IN_16K_DNS];
	float long_term_corr_buff_stats_cntr[HUNDRED_MS_IN_16K_DNS];
	int known_delay;
	int known_delay_less_confidence;
	int last_known_delay;
	int processed_known_delay;

    short mult; // sampling frequency multiple
    int sampFreq;
    WebRtc_UWord32 seed;

    float mu; // stepsize
    float errThresh; // error threshold

    int noiseEstCtr;

    power_level_t farlevel;
    power_level_t nearlevel;
    power_level_t linoutlevel;
    power_level_t nlpoutlevel;

    int metricsMode;
    int stateCounter;
    stats_t erl;
    stats_t erle;
    stats_t aNlp;
    stats_t rerl;

  
    int delay_histogram[kHistorySizeBlocks];
  
    void* delay_estimator;
	int knownDelay;

} statistical_aec_t;

#endif
typedef void (*WebRtcAec_FilterFar_t)(aec_t *aec, float yf[2][PART_LEN1]);
extern WebRtcAec_FilterFar_t WebRtcAec_FilterFar;
typedef void (*WebRtcAec_ScaleErrorSignal_t)(aec_t *aec, float ef[2][PART_LEN1]);
extern WebRtcAec_ScaleErrorSignal_t WebRtcAec_ScaleErrorSignal;
typedef void (*WebRtcAec_FilterAdaptation_t)
  (aec_t *aec, float *fft, float ef[2][PART_LEN1]);
extern WebRtcAec_FilterAdaptation_t WebRtcAec_FilterAdaptation;
typedef void (*WebRtcAec_OverdriveAndSuppress_t)
  (aec_t *aec, float hNl[PART_LEN1], const float hNlFb, float efw[2][PART_LEN1]);
extern WebRtcAec_OverdriveAndSuppress_t WebRtcAec_OverdriveAndSuppress;

int WebRtcAec_CreateAec(aec_t **aec);
int WebRtcAec_FreeAec(aec_t *aec);
int WebRtcAec_InitAec(aec_t *aec, int sampFreq);
void WebRtcAec_InitAec_SSE2(void);

#if (DITECH_VERSION==2)
int WebRtcAec_CreateAecStatistical(statistical_aec_t **aecInst);
int WebRtcAec_FreeAecStatistical(statistical_aec_t *stats_aec);
#endif

void WebRtcAec_InitMetrics(aec_t *aec);
#if (DITECH_VERSION==1)
void WebRtcAec_ProcessFrame(aec_t *aec, const short *farend,
                       const short *nearend, const short *nearendH,
                       short *out, short *outH,
                       int knownDelay);
#endif
#if (DITECH_VERSION==2)
void WebRtcAec_ProcessFrame(aec_t *aec, const short *farend,
                       const short *nearend, const short *nearendH,
                       short *out, short *outH,
                       int knownDelay,short vadState);
void WebRtcAec_ProcessFrame_Statistical(statistical_aec_t *aec_s, const short *farend,
                       const short *nearend,aec_t *aec);
#endif



void foreground_background_aec_analysis(aec_t *aec);
void BufferFar_Arraywise(aec_t *aec, const short *farend, int farLen);
void FetchFar_Arraywise(aec_t *aec, short *farend, int farLen, int knownDelay);
float frame_power(float *a,int size);
float frame_power_short(short *a,int size);

#endif // WEBRTC_MODULES_AUDIO_PROCESSING_AEC_MAIN_SOURCE_AEC_CORE_H_

