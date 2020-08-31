#ifndef _BASIC_H1_
#define _BASIC_H1_
/* Boolean type definition */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cfloat>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cstdarg>

using namespace std;

typedef int Boolean;
typedef short *ShortVec;   /* short vector[1..size] */
typedef float *Vector;     /* vector[1..size]   */
typedef double *DVector;   /* double vector[1..size]   */
typedef double **DMatrix;  /* double matrix[1..nrows][1..ncols] */

#define PI  3.14159265358979
#define TPI 6.28318530717959
#define MINLARG (2.45E-308)
#define LZERO   (-1.0E10)
#define FWORD   8   /*  size of a full word = basic alignment quanta */



const int  NUMCEPCOEF = 12;
const int  NUMCEPCOEF_FBANK = 40;
const int  DEFAULT_INIT_WIN = 100;
const int  DEFAULT_AHEAD_FRAME = 0;
const float  DEFAULT_BETA  = 0.99;
const int  DEFAULT_DATA_TYPE = 1;
const int  DEFAULT_SAMPLE_RATE = 8000;
const float  DEFAULT_PRESET_MAX_EN = 20.0;    /**@ brief: add for preEn*/
const int  DEFAULT_FEAT_TRANS = 0;
const int  DEFAULT_FRAMES_TRANS = 1;
const int  DEFAULT_SERVER_VAD = 0;
const int  DEFAULT_FEAT_TYPE = 0;
struct RefGau_t;
struct transMat_fMPE_t;

const int MAX_EDGE_NUM = 10;
typedef enum { FBANK, FBANK_D_A, PLP_ONLINE, PLP_OFFLINE, MFCC_E_D_A_Z, MFCC, FFT, UNKNOWN_FType } FeatureType_;
typedef enum { TARGETKIND, SAMPLERATE, PACKAGESIZE, NUMCHANS, PREEMCOEFY, PREENERGY,
    USE_POWER, LOG_POWER, CEPSDIM, CEPSLIFTER, INITWIN, HEADNUMFRAME, BETA, 
    IFHEAD, IFSWAP, IFNOISE, IFCONVNOISE, IFDRC, ISPULSE, NOISEFILELIST, 
    IFMIXBAND, MIXBAND_COMPENSATE_FILE, NORM_FFT_BEFORE_MEL, FFT_NORM_FILE, 
    AMPLITUDE_SCALE, DOWNSAMPLING, BV_AS_INPUT, UNKNOWN_CONF_KEY, DATA_OFFSET, WAVE_IN, 
    IFAPM, DRC_VERSION, APM_VERSION, CONTEXT_LEFT, CONTEXT_RIGHT, IFREVERB, REVERB_RATE, REVERB_SMALL_ONLY, ADJUST_SPEED} ConfKey_;

struct feature_conf{
   int initWin;         //��ʼ��ֵ�ĳ���
   int aheadNumFrame;   //�����ֵʱ��lookahead
   float beta;                  //˥��ϵ��
   int samplerate;      //������
   float preEn;   			/**@ brief: add for preEn*/
   //�������������
   int   data_type;                //�������ݵ�����,0Ϊ����,1Ϊwav
   //int   feat_type;             //�������ͣ�0Ϊplp��1Ϊֱ��ͼ
   int   do_fMPE;               //�Ƿ���fMPE��1��ʾ����0��ʾ������Ĭ��Ϊ0
   int	 do_server_vad;			//�Ƿ���server�˵�vad��1��ʾ����0��ʾ������Ĭ��Ϊ0
   transMat_fMPE_t* res_fMPE;   //fMPE����Դ
   
   FeatureType_ feat_type;
   float pre_emph;
   int num_chans;
   int nCepsDim;
   int nCepsLifer;

   int use_power;//1-true; 0-false
   int log_power;//1-true; 0-false
   int package_size;	//its value is set for training, decoder has the package size of its own
   int if_head;//1-true; 0-false
   int if_swap;//1-true; 0-false
   int if_noise;//1-true; 0-false, add noise
   int if_conv_noise;// 1-true, conv noise
   int if_mixband;//1-true;0-false
   //int if_agc;      //1: Do AGC
   int if_drc;      //1: do drc
   int if_apm;      //1: do apm
   float drc_version;
   float apm_version;
   //added by liwei
   int if_pulse; //0-false, 1-1m filter, 2-2m filter, 3-3m filter, 4-4m filter; 
   //added by liwei
   int norm_fft_before_mel; //1-true, 0-false
   char mixband_compensate_file[512];
   char noise_file[512];
   char fft_norm_file[512];
   float amplitude_scale;
   
   int bv_as_input; // extract feat from bv fomat speech. 1-true , 0-false
   int if_downsampling; // 1-true, downsampling from 16k to 8k
   int data_offset;      
   int wave_in;

   int context_left;
   int context_right;

   int if_reverb; // if add reverberation
   float reverb_rate;
   int reverb_small_only; // if add small room reverberation only

   float adjust_speed;

   feature_conf():initWin(0), aheadNumFrame(0), beta(0), samplerate(0), preEn(0), data_type(0), \
		do_fMPE(0), do_server_vad(0), res_fMPE(NULL), pre_emph(0), num_chans(0), nCepsDim(0), nCepsLifer(0), use_power(0), \
		log_power(0), package_size(3000), if_head(0), if_swap(0), if_noise(0), if_conv_noise(0), if_mixband(0), if_drc(0), \
        if_apm(0), apm_version(0.0), if_pulse(0), norm_fft_before_mel(0), amplitude_scale(1.0), bv_as_input(0), if_downsampling(0), data_offset(0), \
        wave_in(0), context_left(20), context_right(20), if_reverb(0), reverb_rate(0.0), reverb_small_only(0), adjust_speed(1.0) {}
};

struct FBankInfo{
   int frameSize;       /* speech frameSize */
   int numChans;        /* number of channels */
   long sampPeriod;     /* sample period */
   int fftN;            /* fft size */
   int klo,khi;         /* lopass to hipass cut-off fft indices */
   Boolean usePower;    /* use power rather than magnitude */
   Boolean takeLogs;    /* log filterbank channels */
   float fres;          /* scaled fft resolution */
   Vector cf;           /* array[1..pOrder+1] of centre freqs */
   ShortVec loChan;     /* array[1..fftN/2] of loChan index */
   Vector loWt;         /* array[1..fftN/2] of loChan weighting */
   Vector x;            /* array[1..fftN] of fftchans */
   Vector y;            /* array[1..fftN] of fftchans for AntiNoise*/
    
   FBankInfo():frameSize(0), numChans(0), sampPeriod(0), fftN(0), klo(0), khi(0), usePower(0), takeLogs(0), fres(0){}

};



#endif
