#ifndef _FEATURECLASS_H1_
#define _FEATURECLASS_H1_

#include "basic.h"


class FeatureClass
{
    private:
        FeatureType_ m_featType;
        int m_featDim;
        
        int m_srcSampRate ;
        int m_frmSize;
        int m_frmStep;
        
        float m_loFBankFreq ;
        float m_hiFBankFreq ;
        int m_nfft;
        int m_nCepsDim;
        int m_numChans ;
        int m_delWin ;
        int m_accWin ;

        float m_preEmph ;
        int  m_usePower ;
        int  m_logPower;
        bool  m_useHam ;
        float *m_s;
        float *m_hamWin;
        Vector m_fbank;
        FBankInfo m_fbInfo;
        float *m_dctCoef;
        float *m_mfcc;

        int   m_nCepsLifter;
        float *m_lifetCoef;
        int *m_brMap;
	
	int m_normfft;
	float *m_fft_mean;
	float *m_fft_stdvar;
	char m_normfft_file[512];
    private: 
        /* EXPORT->CreateVector:  Allocate space for vector v[1..size] */
        Vector CreateVector(int size);

        /* EXPORT->CreateShortVec:  Allocate space for short array v[1..size] */
        ShortVec CreateShortVec(int size);

        /* EXPORT->VectorSize: returns number of components in v */
        int VectorSize(Vector v);

        /* EXPORT->ZeroVector: Zero the elements of v */
        int ZeroVector(Vector v);

        /* EXPORT->GetWaveInFrame: Get next nFrames from w and store in buf */
        int GetWaveInFrame(short *data, int frIdx, int nSamples, float *buf);

        /* EXPORT->Mel: return mel-frequency corresponding to given FFT index */
        float Mel(int k,float fres);

        /**Initialize FilterBank. Need free FBankInfo fb.cf, fb.loChan, fb.loWt, fb.x vector from outside.**/
        FBankInfo InitFBank(int frameSize, int sampPeriod, int numChans,
                float lopass, float hipass, bool usePower, bool takeLogs,
                bool doubleFFT );

        int PreEmphasise ( float *s, float k);

        /* GenHamWindow: generate precomputed Hamming window function */
        int GenHamWindow();

        int Ham ( float *s );

        /* EXPORT-> FFT: apply fft/invfft to complex s */
        int FFT_Cal(Vector s, int invert);

        /* EXPORT-> Realft: apply fft to real s */
        int Realft (Vector s);

        /* EXPORT->Wave2FFT:  Perform FFT on speech s */
        int Wave2FFT(Vector s, float *te, FBankInfo info);

        /* EXPORT->Wave2FBank:  Perform filterbank analysis on speech s */
        int Wave2FBank( Vector fbank, float *te, FBankInfo info);

        int InitDCT( float *dctCoef, int numChans, int cepDim, bool swapc0 );
        int InitLiferCoef( float *lifetCoef, int nCepsLifer, int nCepsDim );
        int InitBitReverse( int *br_map, int n );
        int WeightCepstrum( float *feat, float *lifetCoef, int nCepsDim );
        int FBANK2MFCC( float *mfcc, float *dctCoef, float *fbank, int numChans, int cepDim );
   
    public:
        FeatureClass ();
        ~FeatureClass ();                
        FeatureClass ( int sampleFrequency, feature_conf featConf );
        int Initialize ();
        int Release ();
        int GetDelWin();
        int GetAccWin();
        int GetFeaDim();
        void SetInitValue();

        int Regress(float *dataIn, float *dataOut, int vSize, int n, int step );
        int NormaliseLogEnergy( float *feat, int nFrame, int step );
        int CepsMeanNormalise( float *feat, int nCepsDim, int nFrame, int step );
        int ExtractStaticFeatrues(short *wav, int numSamples, float *feature, int* flen);
};
#endif 
