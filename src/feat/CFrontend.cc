#include "CFrontend.h"
#include "tool_readConf.h"
#include <malloc.h>


Frontend::Frontend ()
{
    m_pcm         = NULL;
    m_pFE         = NULL;	
    //m_plpFrontend = NULL;    
    reset();

}

int Frontend::Init(const char *feat_conf ) {
    if ( -1 == ReadConf( feat_conf, &m_conf ) ) {
        printf("[%s:%d] failed to Read Config file.\n", __FILE__, __LINE__ );
        return -1;
    }
    m_featType        = m_conf.feat_type;
    if(m_conf.if_downsampling == 1 && m_conf.samplerate == 16000) {
        m_sampleFrequency = 8000;
    } else {
        m_sampleFrequency = m_conf.samplerate;
    }

    if ( m_featType != PLP_ONLINE && m_featType != PLP_OFFLINE ) {
        m_pFE = new FeatureClass( m_sampleFrequency, m_conf );
        if( NULL == m_pFE ) {
            printf("[%s:%d]	failed to create FeatureClass instance.\n", __FILE__, __LINE__);
            return -1;
        }

        if ( -1 == m_pFE->Initialize() ) {
            printf("[%s:%d] failed to init FeatureClass instance.\n", __FILE__, __LINE__);
            return -1;
        }
        m_nFeaDim	=	m_pFE->GetFeaDim(); 
    } else {
        //m_plpFrontend = new PLPFrontend();
        //m_plpFrontend->Initialize(&m_conf);
        m_nFeaDim = 39;
    }

    return 0;
}

Frontend::~Frontend	()
{
    if ( m_pFE != NULL) {
        delete m_pFE;
        m_pFE = NULL;
    }
    /*
    if ( m_plpFrontend != NULL ) {
        delete m_plpFrontend;
        m_plpFrontend = NULL;
    }
    */
}

int Frontend::SetBuff(short *wavbuf, int featbuff_size) {
    if ( NULL == wavbuf || featbuff_size < 0 ) {
        printf("[%s:%d] Failed to set buffer.\n", __FILE__, __LINE__);
        return -1;
    }

    m_pcm = wavbuf;
    /*
    if ( NULL != m_plpFrontend ) {
        m_plpFrontend->AdjustFeatureBuff(featbuff_size);
        m_plpFrontend->m_pcm = wavbuf;
        m_plpFrontend->m_pPLP->m_preEn = m_conf.preEn;
    } else {
        m_pcm = wavbuf;
    }
    */
    return 0;
}
void Frontend::reset()
{
		m_doneWaveSamples	=	0;
		m_undoneWaveSamples	=	0;
		m_doneFrames = 0;
		m_doneStaticFrames = 0;
		m_doneDynamicAccFrames	=	0;
		m_doneDynamicDelFrames	=	0;
        m_preEmax = 0.0;
        /*
        if ( m_plpFrontend != NULL )
            m_plpFrontend->reset();
        */
}

int Frontend::copyFirstLastFrame(  float* pbuf, int nFrameLength, int is_last_pack )
{
    float *pTmp = pbuf - MAX_EDGE_NUM * m_nFeaDim;

    if ( m_doneFrames == 0 )
    {
        for( int i = 0; i < MAX_EDGE_NUM; i++ )
            for( int j = 0; j < m_nFeaDim; j++ ) 
                pTmp[i*m_nFeaDim+j] = pbuf[j]; 
    }

    if ( 1 == is_last_pack )
    {
        for( int i = 0; i < MAX_EDGE_NUM; i++ )
            for( int j = 0; j < m_nFeaDim; j++ ) 
                pbuf[(nFrameLength+i)*m_nFeaDim+j] = pbuf[(nFrameLength-1)*m_nFeaDim+j];
    }

    return 0;
}

int Frontend::add_diff( float *tmp_feature, int nFrames, int is_last_pack )
{
    int delWin = m_pFE->GetDelWin();
    int accWin = m_pFE->GetAccWin();	
    int numCepCoef = m_nFeaDim/3;
    int nFrames_del, nFrames_acc;
    nFrames_del = m_doneFrames > 0 ? nFrames : nFrames - delWin;
    nFrames_acc = m_doneFrames > 0 ? nFrames : nFrames - accWin - delWin;
    if ( 1 == is_last_pack )
    {
        nFrames_del = m_doneFrames > 0 ? nFrames + delWin : nFrames;
        nFrames_acc = m_doneFrames > 0 ? nFrames + delWin + accWin : nFrames;
    }

    copyFirstLastFrame( tmp_feature, m_doneStaticFrames, is_last_pack );
    //add del feature: do not process last 2 frames
    float *pIn  = tmp_feature + ( m_doneDynamicDelFrames )*m_nFeaDim;
    float *pOut = tmp_feature + numCepCoef + m_doneDynamicDelFrames*m_nFeaDim;
    m_pFE->Regress( pIn, pOut, numCepCoef, nFrames_del, m_nFeaDim );

    copyFirstLastFrame( tmp_feature, m_doneStaticFrames, is_last_pack );

    //add acc feature:	do not process last	4 frames
    pIn  = tmp_feature + numCepCoef +  m_doneDynamicAccFrames*m_nFeaDim;
    pOut = tmp_feature + 2*numCepCoef + m_doneDynamicAccFrames *m_nFeaDim;
    m_pFE->Regress( pIn, pOut, numCepCoef, nFrames_acc, m_nFeaDim );

    m_doneDynamicAccFrames	+=	nFrames_acc;
    m_doneDynamicDelFrames	+=	nFrames_del;

    return 0;
}


int	Frontend::extract_feature( short *wav, int wlen, float *feature, int *flen,	int	is_last_pack )
{
    /*
    if ( m_featType == PLP_ONLINE || m_featType == PLP_OFFLINE ) {
        m_plpFrontend->extract_feature( wav, wlen, feature, flen, is_last_pack );
        return 0;
    }
    */

    if( wav == NULL || wlen < 0 || feature == NULL || flen == NULL )
    {
        printf(	"[%s:%d] Illegal params	passed into	extract_feature.\n", __FILE__, __LINE__);
        return -1;
    }

    if( wlen == 0 && 0 == is_last_pack )
    {
        *flen = m_doneFrames;
        return 0;
    }

    int nSamples = m_undoneWaveSamples + wlen/sizeof(short);
    short *speechData = m_pcm + m_doneWaveSamples;
    float *pbuf = NULL;
    bool IFFirstPack = false;
    if ( 0 == m_doneFrames ) {
        pbuf = feature + MAX_EDGE_NUM*m_nFeaDim;
        IFFirstPack = true;
    } else {
        pbuf = feature + m_doneStaticFrames*m_nFeaDim;
    }

    int	nFrames;
    int curDoneSamples = m_pFE->ExtractStaticFeatrues( speechData, nSamples, pbuf, &nFrames );
    if ( curDoneSamples == -1 )	{
        printf(	"[%s:%d] Failed	in ExtractStaticFeatures in	extract_feature.\n", __FILE__, __LINE__);
        return -1;
    } else if ( curDoneSamples == 0 && is_last_pack == 0 ) {
        printf( "[%s:%d] Too short wav input to extract one frame of feature in extract_feature.\n", __FILE__, __LINE__);
        m_undoneWaveSamples	=	nSamples;
        *flen	=	m_doneFrames;
        return 0;
    }

    m_doneWaveSamples	= m_doneWaveSamples	+	curDoneSamples;
    m_undoneWaveSamples = nSamples - curDoneSamples;
    m_doneStaticFrames  += nFrames;

    if ( m_doneStaticFrames <= 4 && is_last_pack == 0 ) {
        printf( "[%s:%d] Too short feature to do normalisation of feature in extract_feature.\n", __FILE__, __LINE__);
        m_undoneWaveSamples	=	nSamples;
        *flen	=	m_doneFrames;
        return 0;
    }

    
   if ( MFCC_E_D_A_Z == m_featType ) {
        m_pFE->NormaliseLogEnergy( pbuf + m_nFeaDim/3-1, nFrames, m_nFeaDim );
   }

    bool ifDiff = false;
    if ( FBANK_D_A  == m_featType || MFCC_E_D_A_Z == m_featType ) {
        if ( IFFirstPack )
            add_diff( pbuf, nFrames, is_last_pack );
        else
            add_diff( feature, nFrames, is_last_pack );
        ifDiff = true;
    }

    if (  0 == is_last_pack && ifDiff  )
        *flen = m_doneDynamicAccFrames;
    else 
        *flen = m_doneStaticFrames;

    if ( MFCC_E_D_A_Z == m_featType ) {
        m_pFE->CepsMeanNormalise( pbuf, m_nFeaDim/3 - 1, nFrames, m_nFeaDim );
    }

    if ( IFFirstPack ) {
        for ( int i = 0; i < m_doneStaticFrames; i++ )
        {
            for ( int j = 0; j < m_nFeaDim; j++ )
            {
                feature[i*m_nFeaDim + j] = pbuf[ i*m_nFeaDim + j ];
            }
        }
        IFFirstPack = false;
    } else {}

    m_doneFrames = *flen;

    return 0;
}

