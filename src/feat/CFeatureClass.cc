#include "CFeatureClass.h"

FeatureClass::FeatureClass()
{
	SetInitValue();
}
FeatureClass::~FeatureClass()
{
    Release();
}
FeatureClass::FeatureClass(int sampleFrequency, feature_conf featConf )
{
    SetInitValue();
    m_featType = featConf.feat_type;
    m_preEmph  = featConf.pre_emph;
    m_numChans = featConf.num_chans;
    m_nCepsDim = featConf.nCepsDim;
    m_nCepsLifter = featConf.nCepsLifer;
    m_usePower = featConf.use_power;
    m_logPower = featConf.log_power;
    
    if ( sampleFrequency == 16000 ) {
        m_srcSampRate = 625;
        m_frmSize = 400;
        m_frmStep = 160;
        m_nfft = 512;
        m_loFBankFreq = -1.0f;
        m_hiFBankFreq = -1.0f;
    }

    switch ( m_featType )
    {
        case FBANK:
            m_featDim = m_numChans;
            break;
        case FBANK_D_A:
            m_featDim = m_numChans*3;
            break;
        case MFCC_E_D_A_Z:
            m_featDim = (m_nCepsDim+1)*3;
            break;
        case MFCC:
            m_featDim = m_nCepsDim+1;
            break;
        case FFT:
            m_featDim = m_nfft/2;
            break;
    }

}


int FeatureClass::GetFeaDim()
{
    return m_featDim;
}

int  FeatureClass::GetDelWin()
{
    return m_delWin;
}

int  FeatureClass::GetAccWin()
{
    return m_accWin;
}


int FeatureClass::Initialize ()
{
    
    if( -1 == GenHamWindow() )
    {
        printf( "[%s:%d] Fail to init ham Window.\n", __FILE__, __LINE__ );
        return -1;
    }
    m_brMap = new int[m_nfft];
    if ( -1 == InitBitReverse( m_brMap, m_nfft ) )
    {
        printf( "[%s,%d] Fail to init Bit Rerverse.\n", __FILE__, __LINE__ );
        return -1;
    }

    if ( MFCC_E_D_A_Z == m_featType || MFCC == m_featType )
    {
        m_dctCoef = new float[m_numChans*m_nCepsDim];
        m_mfcc    = new float[m_nCepsDim];
        if ( -1 == InitDCT( m_dctCoef, m_numChans, m_nCepsDim, false ) )
        {
            printf( "[%s:%d] Fail to init DCT Coef.\n", __FILE__, __LINE__ );
            return -1;
        }
        
        m_lifetCoef = new float[m_nCepsDim];
        if ( -1 == InitLiferCoef( m_lifetCoef, m_nCepsLifter, m_nCepsDim ) )
        {
            printf( "[%s:%d] Fail to init Lifer Coef.\n", __FILE__, __LINE__ );
            return -1;
        }
    }
    
    m_fbInfo = InitFBank( m_frmSize, m_srcSampRate, m_numChans, 
            m_loFBankFreq, m_hiFBankFreq, m_usePower, 
            true, false);

    m_s = new float[m_frmSize];
    m_fbank = CreateVector (m_numChans);


    if (m_hamWin == NULL || m_s == NULL || m_fbank == NULL )
    {
        printf( "[%s:%d] Fail to get filterbank information in ExtractStaticFeatrues.\n",
          __FILE__, __LINE__);
        return -1;
    }
    if (m_fbInfo.x == NULL)
    {
        printf( "[%s:%d] Fail to get filterbank information in ExtractStaticFeatrues.\n",
            __FILE__, __LINE__);
        return -1;        
    }    
	
    return 0;
}

int FeatureClass::Release ()
{
    if (m_fbInfo.cf != NULL) 
        delete [] m_fbInfo.cf;
	
    if (m_fbInfo.loChan != NULL)
        delete [] m_fbInfo.loChan;
	
    if (m_fbInfo.loWt != NULL)   
        delete [] m_fbInfo.loWt;
	
    if (m_fbInfo.x != NULL) {
        free (m_fbInfo.x);
        //delete [] m_fbInfo.x;
    }
	
    if (m_fbank != NULL) 
        free (m_fbank);
    if (m_brMap != NULL )
        delete [] m_brMap;
    if ( m_hamWin != NULL )  
        delete [] m_hamWin;  
	
    if ( m_s != NULL ) 
        delete [] m_s;

    if ( m_dctCoef != NULL )
        delete [] m_dctCoef;

    if ( m_mfcc != NULL )
        delete [] m_mfcc;

    if ( m_lifetCoef != NULL )
        delete [] m_lifetCoef;

    if (m_fft_mean != NULL) {
        delete [] m_fft_mean;
	    delete [] m_fft_stdvar;
    }
    SetInitValue();

    return 0;
}

Vector FeatureClass::CreateVector(int size)
{
    if (size <= 0) 
    {
        printf( "[%s:%d] Illegal params passed into CreateVector.",
                __FILE__, __LINE__);
        return NULL;
    }
    Vector v;
    int *i;
   
    v = (Vector)malloc(sizeof(float)*(size+1));
    if ( v == NULL)
   	{
        printf( "[%s:%d] Fail to alloc memory for float array.",
                __FILE__, __LINE__);
        return NULL;   		
   	}
    i = (int *) v; *i = size;
    return v;
}

/* EXPORT->CreateShortVec:  Allocate space for short array v[1..size] */
ShortVec FeatureClass::CreateShortVec(int size)
{

    if (size <= 0)
    {
        printf( "[%s:%d] Illegal params passed into CreateShortVec.",
                __FILE__, __LINE__);
        return NULL;
    }
    short *v;
   
    v = (ShortVec)malloc(sizeof(short)*(size+1));
    if ( v == NULL)
   	{
        printf( "[%s:%d] Fail to alloc memory for short array.",
                __FILE__, __LINE__);
        return NULL;   		
   	}
    *v = (short)size;
    return (ShortVec)v;
}


/* EXPORT->VectorSize: returns number of components in v */
int FeatureClass::VectorSize(Vector v)
{
    if ( v == NULL)
   	{
        printf( "[%s:%d] Illegal params passed into extract_featurey.",
                __FILE__, __LINE__);
        return -1;   		
   	}

    int *i;

    i = (int *) v;
    return *i;
}

/* EXPORT->ZeroVector: Zero the elements of v */
int FeatureClass::ZeroVector(Vector v)
{
    if ( v == NULL)
   	{
        printf( "[%s:%d] Illegal params passed into extract_featurey.",
                __FILE__, __LINE__);
        return -1;   		
   	}
   int i,n;
   
   n=VectorSize(v);
   if (n == -1)
   	{
    	  printf( "[%s:%d] Failed in InitU0FZeroMean in extract_feature.",
                __FILE__, __LINE__);
        return -1;
   	}
   for (i=1;i<=n;i++) v[i]=0.0;
   return 0;
}


/* EXPORT->GetWaveInFrame: Get next nFrames from w and store in buf */
int FeatureClass::GetWaveInFrame( short *data, int frIdx, int nSamples, float *buf )
{
    if( data == NULL || buf == NULL )
    {
        printf( "[%s:%d] Illegal params passed into GetWaveInFrame.\n",
                __FILE__, __LINE__);
        return -1;
    }
   
    if ( frIdx + m_frmSize > nSamples )
    {
        printf( "[%s:%d] Attempt to read past end of buffer.\n",
                __FILE__, __LINE__);
        return -1;
    }

    for ( int k = 0; k < m_frmSize; k++ )
        *buf++ = (float)data[frIdx+k];
    
    return 0;
}

/* EXPORT->Mel: return mel-frequency corresponding to given FFT index */
float FeatureClass::Mel( int k, float fres )
{
    return 1127 * log(1 + k*fres);
}
	

/**Initialize FilterBank. Need free FBankInfo fb.cf, fb.loChan, fb.loWt, fb.x vector from outside.**/
FBankInfo FeatureClass::InitFBank(int frameSize, int sampPeriod, int numChans,
                    float lopass, float hipass, bool usePower, bool takeLogs,
                    bool doubleFFT )
{
 	
    FBankInfo fb;
    fb.cf = NULL;
    fb.loChan = NULL;
    fb.loWt = NULL;
    fb.x = NULL;

    if( frameSize == 0 || sampPeriod == 0 || numChans == 0 )
    {
        printf( "[%s:%d] Illegal params passed into InitFBank.\n", __FILE__, __LINE__);
        return fb;
    }


    // Save sizes to cross-check subsequent usage
    fb.frameSize  = frameSize;
    fb.numChans   = numChans;
    fb.sampPeriod = sampPeriod; 
    fb.usePower   = usePower;
    fb.takeLogs   = takeLogs;

    // Calculate required FFT size
    fb.fftN = m_nfft;//256;   
    /*while (frameSize>fb.fftN)
    fb.fftN *= 2;*/
    if ( doubleFFT ) 
        fb.fftN *= 2;

    int Nby2    = fb.fftN / 2;
    fb.fres     = 1.0E7 / ( sampPeriod * fb.fftN * 700.0 );
    int maxChan = numChans + 1;

    // set lo and hi pass cut offs if any 
    fb.klo = 2;
    fb.khi = Nby2; // apply lo/hi pass filtering

    float mlo = 0; 
    float mhi = Mel( Nby2, fb.fres );
    if ( lopass >= 0.0 )
    {
        mlo = 1127 * log( 1 + lopass/700.0 );
        fb.klo = (int) ((lopass * sampPeriod * 1.0e-7 * fb.fftN) + 2.5);
        if ( fb.klo < 2 )
            fb.klo = 2;
    }

    if ( hipass >= 0.0 )
    {
        mhi = 1127 * log( 1 + hipass/700.0 );
        fb.khi = (int) ((hipass * sampPeriod * 1.0e-7 * fb.fftN) + 0.5);
        if ( fb.khi > Nby2 )
            fb.khi = Nby2;
    }

    // Create vector of fbank centre frequencies
    fb.cf = new float[maxChan];
    if ( fb.cf == NULL )
    {
        printf( "[%s:%d] Fail to create vector fb.cf.\n", __FILE__, __LINE__ );
        return fb;
    }

    float ms = mhi - mlo;

    for ( int chan = 0; chan < maxChan; chan++ ) {
        fb.cf[chan] = ((float)(chan+1)/(float)maxChan)*ms + mlo;
	//printf("%f\n", fb.cf[chan]);
    }

    // Create loChan map, loChan[fftindex] -> lower channel index 
    fb.loChan = new short[Nby2];//CreateShortVec(Nby2);
    if ( fb.loChan == NULL )
    {
        printf( "[%s:%d] Fail to create vector fb.loChan.\n", __FILE__, __LINE__ );
        return fb;
    }

    for ( int k = 0, chan = 0; k < Nby2; k++ )
    {
        float melk = Mel( k, fb.fres );
        if ( k+1 < fb.klo || k+1 > fb.khi )
            fb.loChan[k] = -1;
        else
        {
            while ( fb.cf[chan] < melk  && chan <= maxChan-1 )
                ++chan;
            fb.loChan[k] = chan-1;
        }
    }

    // Create vector of lower channel weights   
    fb.loWt = new float[Nby2];
    if ( fb.loWt == NULL )
    {
        printf( "[%s:%d] Fail to create vector fb.Wt.\n", __FILE__, __LINE__ );
        return fb;
    }
    for ( int k = 0; k < Nby2; k++ )
    {
        int chan = fb.loChan[k];
        if ( k+1 < fb.klo || k+1 > fb.khi )
            fb.loWt[k]=0.0;
        else
        {
            if ( chan >= 0 ) 
                fb.loWt[k] = ((fb.cf[chan+1] - Mel(k,fb.fres)) / (fb.cf[chan+1] - fb.cf[chan]));
            else
                fb.loWt[k] = (fb.cf[0]-Mel(k,fb.fres)) / (fb.cf[0] - mlo);
        }
    }

    // Create workspace for fft
    fb.x = CreateVector( fb.fftN );
    if ( fb.x == NULL )
    {
        printf( "[%s:%d] Fail to create vector fb.x.\n",
                __FILE__, __LINE__);
        return fb;
    }

    return fb;
}


	
/* EXPORT->PreEmphasise: pre-emphasise signal in s */
int FeatureClass::PreEmphasise ( float *s, float preE )
{
    if( s == NULL )
    {
        printf( "[%s:%d] Illegal params passed into PreEmphasise.\n",
                __FILE__, __LINE__);
        return -1;
    }	
   
    for ( int i = m_frmSize - 1; i >= 1; i-- )
        s[i] -= s[i-1]*preE;

    s[0] *= 1.0-preE;

    return 0;
}

/* GenHamWindow: generate precomputed Hamming window function */
int FeatureClass::GenHamWindow()
{

    if ( m_hamWin == NULL ) 
        m_hamWin = new float[m_frmSize];

    if ( m_hamWin == NULL)
    {
        printf( "[%s:%d] Fail to create hamWin in GenHamWindow.\n", __FILE__, __LINE__ );
        return -1;
    }

    float a = TPI / ( m_frmSize - 1);
    for ( int i = 0; i < m_frmSize; i++ )
        m_hamWin[i] = 0.54 - 0.46 * cos(a*i);

    return 0;
}

int FeatureClass::Ham ( float *s )
{
    if( s == NULL || NULL == m_hamWin )
    {
        printf( "[%s:%d] Illegal params passed into Ham.", __FILE__, __LINE__ );
        return -1;
    }	

    for ( int i = 0; i < m_frmSize; i++ )
        s[i] *= m_hamWin[i];
    
    return 0;
}

int FeatureClass::InitBitReverse( int *br_map, int n )
{
    if ( NULL == br_map )
    {
        printf( "[%s:%d] Illegal params passed into InitBitReverse.\n", __FILE__, __LINE__);
        return -1;
    }	
    
    int nL = (int)(log((float) n)/log(2.0));
    //printf("%d\n", nL);
    for( int i = 0; i < n/2; i++ )
    {
        int r = 0, ii, p = ii = 2*i;
        for ( int j = 0; j < nL; j++ )//used for 256
        {
            r <<= 1;
            if ( ii & 1 )
                r |= 1;
            ii >>= 1;
        }
        br_map[p]   = 2*r + 1;
        br_map[++p] = 2*r + 2;
    }
    //printf("%d\n", br_map[2]);
    return 0;
}

/* EXPORT-> FFT: apply fft/invfft to complex s */
int FeatureClass::FFT_Cal( Vector s, int invert )
{
    if( s == NULL )
    {
        printf( "[%s:%d] Illegal params passed into FFT.\n", __FILE__, __LINE__);
        return -1;
    }	

    int n  = m_nfft;
    int nn = n / 2;
    int j = 1;

    int limit = 2;

    while ( limit < n )
    {
        int inc      = 2 * limit;
        double theta = TPI / limit;

        if (invert)
            theta = -theta;

        double x   = sin(0.5 * theta);
        double wpr = -2.0 * x * x;
        double wpi = sin(theta); 
        double wr  = 1.0;
        double wi  = 0.0;
      
        for ( int ii = 1; ii <= limit/2; ii++ )
        {
            int m = 2 * ii - 1;
            for ( int jj = 0; jj <= (n - m) / inc; jj++ ) 
            {
                int i = m + jj * inc;
                j = i + limit;
                double xre = wr * s[j] - wi * s[j + 1];
                double xri = wr * s[j + 1] + wi * s[j];
                s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
                s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
            }
            double wx = wr;
            wr = wr * wpr - wi * wpi + wr;
            wi = wi * wpr + wx * wpi + wi;
        }
        limit = inc;
   }
   if (invert)
      for ( int i = 1; i <= n; i++ ) 
         s[i] = s[i] / nn;
   return 0;
}

/* EXPORT-> Realft: apply fft to real s */
int FeatureClass::Realft ( Vector s )
{
    if( s == NULL )
    {
        printf( "[%s:%d] Illegal params passed into Realft.\n", __FILE__, __LINE__);
        return -1;
    }	

    int n = m_nfft/2;
    int n2 = n/2;
    double theta = PI / n;

    if ( FFT_Cal( s, false ) == -1 )
    {
        printf( "[%s:%d] Fail to do FFT in Realft.\n", __FILE__, __LINE__);
        return -1;
    }

    double x   = sin(0.5 * theta);
    double yr2 = -2.0 * x * x;
    double yi2 = sin(theta);
    double yr  = 1.0 + yr2;
    double yi  = yi2;

    for ( int i = 2; i <= n2; i++ )
    {
        int i1 = i + i - 1;
        int i2 = i1 + 1;
        int i3 = n + n + 3 - i2;
        int i4 = i3 + 1;

        double wrs = yr;
        double wis = yi;

        double xr1 = (s[i1] + s[i3])/2.0;
        double xi1 = (s[i2] - s[i4])/2.0;
        double xr2 = (s[i2] + s[i4])/2.0;
        double xi2 = (s[i3] - s[i1])/2.0;

        s[i1] = xr1 + wrs * xr2 - wis * xi2;
        s[i2] = xi1 + wrs * xi2 + wis * xr2;
        s[i3] = xr1 - wrs * xr2 + wis * xi2;
        s[i4] = -xi1 + wrs * xi2 + wis * xr2;
        double yr0 = yr;
        yr = yr * yr2 - yi  * yi2 + yr;
        yi = yi * yr2 + yr0 * yi2 + yi;
    }

    s[1] = s[1] + s[2];
    s[2] = 0.0;
    //s[2] = s[1] - 2*s[2];

    return 0;
}

// EXPORT->Wave2FFT:  Perform filterbank analysis on speech s
int FeatureClass::Wave2FFT( Vector s, float *te, FBankInfo info )
{
    if( s == NULL || info.x == NULL )
    {
        printf( "[%s:%d] Illegal params passed into Wave2FFT.\n",
                __FILE__, __LINE__);
        return -1;
    }	

   
    // Check that info record is compatible
    if ( info.frameSize != m_frmSize )
    {
       printf( "[%s:%d] Incompatible params passed into Wave2FFT.\n",
               __FILE__, __LINE__);
       return -1;
    }
    // Compute frame energy if needed
    if ( te != NULL )
    {
      *te = 0.0;  
      for ( int k = 0; k < info.frameSize; k++ ) 
         *te += (s[k]*s[k]);
    }

    // Apply FFT
    for ( int k = 0; k < info.frameSize; k++ ) 
      info.x[m_brMap[k]] = s[k];    // copy to workspace 
      //info.x[k+1] = s[k];    // copy to workspace 

    for ( int k = info.frameSize; k < info.fftN; k++ ) 
      info.x[m_brMap[k]] = 0.0;   // pad with zeroes
      //info.x[k+1] = 0.0;   // pad with zeroes

    if ( Realft( info.x ) == -1 ) // take fft
    {
            printf( "[%s:%d] Fail to do FFT in Realft.\n",
                __FILE__, __LINE__);
        return -1;
    }	   	
 
   return 0;
}

/* EXPORT->Wave2FBank:  Perform filterbank analysis on speech s */
int FeatureClass::Wave2FBank( Vector fbank, float *te, FBankInfo info)
{
    if( fbank == NULL || info.x == NULL )
    {
        printf( "[%s:%d] Illegal params passed into Wave2FBank.\n",
                __FILE__, __LINE__);
        return -1;
    }	

    float melfloor = 1.0;
    int k, bin;
    float t1,t2;   // real and imag parts 
    float ek;      // energy of k'th fft channel

    if (info.numChans != VectorSize(fbank))
    {
        printf( "[%s:%d] Incompatible params passed into Wave2FBank.\n",
                __FILE__, __LINE__);
        return -1;
    }
 
    // Fill filterbank channels 
    ZeroVector(fbank); 
    info.x[2] = 0;
    for (k = info.klo; k <= info.khi; k++) // fill bins
    {
        t1 = info.x[2*k-1];
        t2 = info.x[2*k];
        if (info.usePower)
            ek = t1*t1 + t2*t2;
        else
            ek = sqrt(t1*t1 + t2*t2);

        bin = info.loChan[k-1] +1;
        t1  = info.loWt[k-1]*ek;

        if ( bin > 0 )
            fbank[bin] += t1;
        if ( bin < info.numChans )
            fbank[bin+1] += ek - t1;
    }

    // Take logs
    if (info.takeLogs)
        for (bin=1; bin<=info.numChans; bin++) 
        { 
            t1 = fbank[bin];
            if (t1<melfloor) t1 = melfloor;
            fbank[bin] = log(t1);
        }

    return 0;
}

/* ------------------- Feature Level Operations -------------------- */

int FeatureClass::Regress(float *dataIn, float *dataOut, int vSize, int n, int step )
{
    if(dataIn==NULL || dataOut==NULL || vSize==0 || n <= 0 || step==0 )
    {
        printf("[%s:%d] Illegal params passed into Regress.", __FILE__, __LINE__);
        exit(1);
    }
   
    float sigmaT2 = 0.0;
    for ( int t = 1; t <= m_delWin; t++ ) 
        sigmaT2 += t*t;
    sigmaT2 *= 2.0;
    for (int i=0;i< n;i++)
    {
        float *fp1 = dataIn; 
        float *fp2 = dataOut;
        for ( int j = 0;j< vSize; j++ )
        {
            float *back = fp1;
            float *forw = fp1; 
            float sum = 0.0;
            for ( int t = 1;t <= m_delWin; t++ ) 
            {
                back -= step;
                forw += step;
                sum  += t * (*forw - *back);
            }
            *fp2 = sum / sigmaT2;
            ++fp1; ++fp2;
        }
        dataIn  += step;
        dataOut += step;
    }
    return 0;
}

int FeatureClass::InitDCT( float *dctCoef, int numChans, int cepDim, bool swapc0 )
{
    if ( NULL == dctCoef || 0 == numChans || 0 == cepDim )
    {
        printf( "[%s:%d] Illegal params passed into InitDCT.\n", __FILE__, __LINE__ );
        return -1;
    }
    
    float base  = (float)PI / numChans;
    float scale = sqrt( 2.0 / numChans );

    for ( int freqX = 0; freqX < numChans; ++freqX )
    {
        for ( int cepX = 1; cepX <= cepDim; ++cepX )
        {
            //float alpha = ( cepX == 0 ) ? ( 1.0/sqrt(2.0) ) : 1.0;
            dctCoef[freqX*cepDim + cepX-1] = scale * cos( base * cepX * ( freqX + 0.5 ) );
        }
    }

    if ( swapc0 ) 
    {
        for ( int freqX = 0; freqX < numChans; ++freqX )
        {
            float c0 = dctCoef[ freqX*cepDim];
            for ( int cepX = 1; cepX < cepDim; ++ cepX )
                dctCoef[freqX*cepDim + cepX - 1] = dctCoef[freqX*cepDim + cepX];
            dctCoef[freqX*cepDim + cepDim-1] = c0;
        }
    }

    return 0;

}

int FeatureClass::FBANK2MFCC( float *mfcc, float *dctCoef, float *fbank, int numChans, int cepDim )
{
    if ( NULL == mfcc || NULL == dctCoef || NULL == fbank || 0 == numChans || 0 == cepDim )
    {
        printf( "[%s:%d] Illegal params passed into FBANK2MFCC.\n", __FILE__, __LINE__ );
        return -1;
    }

   
    for ( int cepX = 0; cepX < cepDim; ++cepX )
    {
        mfcc[cepX] = 0.0;
        for ( int freqX = 0; freqX < numChans; ++freqX )
            mfcc[cepX] += dctCoef[freqX*cepDim + cepX]*fbank[freqX+1];
    }

    return 0;
}

int FeatureClass::NormaliseLogEnergy( float *feat, int nFrame, int step )
{
    if( feat == NULL || nFrame <= 0 )
    {
        printf( "[%s:%d] Illegal params passed into NormaliseLogEnergy.\n", __FILE__, __LINE__);
        return -1;
    }

   /* find max log energy */
   float *p = feat;
   float max = 0;;
   for ( int i = 0; i < nFrame; i++ )
   {
      if ( *p > max )  max = *p;
      p += step;                   /* step p to next e val */
   }

   float min = max - 5.0*log(10.0);  /* set the silence floor */
   /* normalise */
   p = feat;
   for ( int i = 0; i < nFrame; i++ )
   {
      if ( *p < min) *p = min;          /* clamp to silence floor */
      *p = 1.0 - (max - *p); //* m_eScale;  /* normalise */
      p += step; 
   }
   return 0;
}

int FeatureClass::CepsMeanNormalise( float *feat, int nCepsDim, int nFrame, int step )
{

    for ( int i = 0; i < nCepsDim; ++i)
    {
        /* find mean over i'th component */
        float sum = 0.0;
        float *fp = feat + i;
        for ( int j = 0; j < nFrame; ++j )
        {
            sum += *fp; 
            fp += step;
        }
        float mean = sum / nFrame;
        /* subtract mean from i'th components */
        fp = feat + i;
        for ( int j = 0; j < nFrame; j++ )
        {
            *fp -= mean;
            fp += step;
        }
    }
    return 0;
}

int FeatureClass::InitLiferCoef( float *lifetCoef, int nCepsLifer, int nCepsDim )
{
    if ( NULL == lifetCoef )
    {
        printf( "[%s:%d] Illegal params passed into InitLiferCoef.\n",
                __FILE__, __LINE__);
        return -1;
    }
    
    float base  = PI / nCepsLifer;
    float scale = nCepsLifer / 2.0;
    for ( int i = 0; i < nCepsDim; ++i )
    {
        lifetCoef[i] = 1.0 + scale * sin( base * (i+1) );
    }

    return 0;
}

int FeatureClass::WeightCepstrum( float *feat, float *lifetCoef, int nCepsDim )
{
    for ( int i = 0; i < nCepsDim; ++i )
    {
        feat[i] = feat[i] * lifetCoef[i];
    }
    return 0;
}

int FeatureClass::ExtractStaticFeatrues( short *wav, int numSamples, float *feature, int* flen )
{
    if( wav == NULL || numSamples == 0 || feature == NULL )
    {
        printf( "[%s:%d] Illegal params passed into ExtractStaticFeatrues.\n",
                __FILE__, __LINE__);
        return -1;
    }
    
    if( numSamples < m_frmSize )
    {
	    *flen = 0;
        return 0;
    }

    int nFrames = ( numSamples - m_frmSize ) / m_frmStep + 1;
    float *pbuf = feature;
    *flen = nFrames;

    for ( int k = 0; k < nFrames; k++ ) 
    {
        // Get one frame of wave data
        if ( GetWaveInFrame( wav, k*m_frmStep, numSamples, m_s ) == -1 )
        {
            printf( "[%s:%d] Fail to do GetWaveInFrame in ExtractStaticFeatrues.\n",
                    __FILE__, __LINE__);
            return -1;        
        }         	
        float l_energy = 0.0;
        for ( int i = 0; i < m_frmSize; ++i )
            l_energy += m_s[i]*m_s[i];
        // perEmphasise
        if ( m_preEmph > 0.0 ) 
            if ( PreEmphasise( m_s, m_preEmph ) == -1 )
            {
                printf( "[%s:%d] Fail to do PreEmphasise in ExtractStaticFeatrues.\n",
                        __FILE__, __LINE__);
                return -1;        
            }         	
        
        // Hamming Window
        if ( m_useHam ) 
            if ( Ham( m_s ) == -1 )
            {
                printf( "[%s:%d] Fail to do Ham in ExtractStaticFeatrues.\n",
                        __FILE__, __LINE__);
                return -1;        
            }         	

        // FFT
        if ( Wave2FFT( m_s, NULL, m_fbInfo ) == -1 )
        {
            printf( "[%s:%d] Fail to do Wave2FFT in ExtractStaticFeatrues.\n",
                    __FILE__, __LINE__);
            return -1;        
        }										   
        
        if (FFT == m_featType) {
            for (int k = 1; k <= m_featDim; k++) {
                float t1, t2, ek;
                t1 = m_fbInfo.x[2*k-1];
                t2 = m_fbInfo.x[2*k];
                if (m_usePower) {
                    //if (k == 1) {
                    //    ek = t1*t1;// + t2*t2;
                    //} else if (k == m_featDim) {
                    //    ek = m_fbInfo.x[2]*m_fbInfo.x[2];
                    //} else {
                        ek = t1*t1 + t2*t2;
                    //}
                } else {
                    //if (k == 1) {
                    //    ek = sqrt(t1*t1);
                    //} else if (k == m_featDim) {
                    //    ek = sqrt(m_fbInfo.x[2]*m_fbInfo.x[2]);
                    //} else {
                        ek = sqrt(t1*t1 + t2*t2);
                    //}
                }

                if (m_logPower)
                    ek = ( ek < 1.0 ) ? 0.0 : log(ek);
                *feature++ = ek;
            }
            continue;
        }
        // wave to filter bank
        if ( Wave2FBank( m_fbank, NULL, m_fbInfo ) == -1 )
        {
            printf( "[%s:%d] Fail to do Wave2FBank in ExtractStaticFeatrues.\n",
                    __FILE__, __LINE__);
            return -1;        
        }

        if ( FBANK == m_featType || FBANK_D_A == m_featType )
        {
            for ( int i = 1; i <= m_numChans; i++ ) 
                *feature++ = m_fbank[i];
            feature = feature + m_featDim - m_numChans;
            
            continue;
        }

        if ( -1 == FBANK2MFCC( m_mfcc, m_dctCoef, m_fbank, m_numChans, m_nCepsDim ) )
        {
            printf( "[%s:%d] Fail to do FBANK2MFCC in ExtractStaticFeatrues.\n",
                    __FILE__, __LINE__);
            return -1;        
        }
        
        if ( -1 == WeightCepstrum( m_mfcc, m_lifetCoef, m_nCepsDim ) )
        {
            printf( "[%s:%d] Fail to do WeightCepstrum in ExtractStaticFeatrues.\n",
                    __FILE__, __LINE__);
            return -1;        
        }

        if ( MFCC_E_D_A_Z == m_featType || MFCC == m_featType)
        {
            for ( int i = 0; i < m_nCepsDim; i++ ) 
            {
                *(feature + i) =  m_mfcc[i];
            }
            *( feature + m_nCepsDim ) = ( l_energy < MINLARG ) ? LZERO : log(l_energy);
            feature = feature + m_featDim;
        }

    }

    return (nFrames*m_frmStep);

}


void FeatureClass::SetInitValue()
{
    m_fbank       = NULL;
    m_hamWin      = NULL;
    m_s           = NULL;
 
    m_mfcc        = NULL;
    m_dctCoef     = NULL;
    m_brMap       = NULL;
    m_lifetCoef   = NULL;
    m_nCepsLifter = 22;
    m_srcSampRate = 1250;
    m_frmSize     = 200;
    m_frmStep     = 80;
    m_loFBankFreq = 60.0;
    m_hiFBankFreq = 3400.0;
    m_nfft        = 256;
    m_nCepsDim      = 12;
    m_numChans    = 24;

    m_delWin      = 2;
    m_accWin      = 2;

    m_preEmph     = 0.0;

    m_usePower    = 1;
    m_logPower    = 0;
    m_useHam      = true;
    m_normfft = 0;

	m_fft_mean = NULL;
	m_fft_stdvar = NULL;
}
