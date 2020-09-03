// Copyright (C) 2019 ATHENA DECODER AUTHORS; Xiangang Li; Yang Han; Long Yuan
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ==============================================================================

#ifndef _TOOL_READCONF_H1_
#define _TOOL_READCONF_H1_

#include "basic.h"

ConfKey_ ConfKey( char *strKey )
{
    if ( 0 == strcmp("TARGETKIND", strKey ) )
        return TARGETKIND;
    if ( 0 == strcmp( "SAMPLERATE", strKey ) )
        return SAMPLERATE;
    if ( 0 == strcmp( "PACKAGESIZE", strKey ) )
        return PACKAGESIZE;
    if ( 0 == strcmp( "NUMCHANS", strKey ) )
        return NUMCHANS;
    if ( 0 == strcmp( "PREEMCOEFY", strKey ) )
        return PREEMCOEFY;
    if ( 0 == strcmp( "PREENERGY", strKey ) )
        return PREENERGY;
    if ( 0 == strcmp( "USE_POWER", strKey) )
        return USE_POWER;
    if ( 0 == strcmp( "LOG_POWER", strKey) )
        return LOG_POWER;
    if ( 0 == strcmp( "CEPSDIM", strKey) )
        return CEPSDIM;
    if ( 0 == strcmp( "CEPSLIFTER", strKey) )
        return CEPSLIFTER;
    if ( 0 == strcmp( "INITWIN", strKey) )
        return INITWIN;
    if ( 0 == strcmp( "HEADNUMFRAME", strKey) )
        return HEADNUMFRAME;
    if ( 0 == strcmp( "BETA", strKey) )
        return BETA;
    if ( 0 == strcmp( "IFHEAD", strKey ) )
        return IFHEAD;
    if ( 0 == strcmp( "IFSWAP", strKey ) )
        return IFSWAP;
    if ( 0 == strcmp( "IFNOISE", strKey ) )
        return IFNOISE;
    if ( 0 == strcmp( "IFCONVNOISE", strKey ) )
        return IFCONVNOISE;
    if ( 0 == strcmp( "ISPULSE", strKey ) )
        return ISPULSE;
    if ( 0 == strcmp( "IFDRC", strKey ) )
        return IFDRC;
    if ( 0 == strcmp( "IFAPM", strKey ) )
        return IFAPM;
    if ( 0 == strcmp( "DRC_VERSION", strKey ) )
        return DRC_VERSION;
    if ( 0 == strcmp( "APM_VERSION", strKey ) )
        return APM_VERSION;
    if ( 0 == strcmp( "NOISEFILELIST", strKey ) )
        return NOISEFILELIST;
    if ( 0 == strcmp( "IFMIXBAND", strKey ) )
        return IFMIXBAND;
    if ( 0 == strcmp( "MIXBAND_COMPENSATE_FILE", strKey ) )
        return MIXBAND_COMPENSATE_FILE;
    if ( 0 == strcmp("NORM_FFT_BEFORE_MEL", strKey))
	return NORM_FFT_BEFORE_MEL;
    if ( 0 == strcmp("FFT_NORM_FILE", strKey))
        return FFT_NORM_FILE;
    if ( 0 == strcmp("AMPLITUDE_SCALE", strKey))
        return AMPLITUDE_SCALE;
    if ( 0 == strcmp("BV_AS_INPUT", strKey))
        return BV_AS_INPUT;
    if ( 0 == strcmp("DOWNSAMPLING", strKey))
        return DOWNSAMPLING;
    if ( 0 == strcmp("DATA_OFFSET", strKey))
        return DATA_OFFSET;
    if ( 0 == strcmp("WAVE_IN", strKey))
        return WAVE_IN;
    if ( 0 == strcmp("CONTEXT_LEFT", strKey))
        return CONTEXT_LEFT;
    if ( 0 == strcmp("CONTEXT_RIGHT", strKey))
        return CONTEXT_RIGHT;
    if ( 0 == strcmp("IFREVERB", strKey))
        return IFREVERB;
    if ( 0 == strcmp("REVERB_RATE", strKey))
        return REVERB_RATE;
    if ( 0 == strcmp("REVERB_SMALL_ONLY", strKey))
        return REVERB_SMALL_ONLY;
    if ( 0 == strcmp("ADJUST_SPEED", strKey))
        return ADJUST_SPEED;
    return UNKNOWN_CONF_KEY;
}

FeatureType_ FeatureType( char *strVal )
{
    if ( 0 == strcmp( "FBANK", strVal ) )
        return FBANK;
    if ( 0 == strcmp( "FBANK_D_A", strVal ) )
        return FBANK_D_A;
    if ( 0 == strcmp( "PLP_ONLINE", strVal ) )
        return PLP_ONLINE;
    if ( 0 == strcmp( "PLP_OFFLINE", strVal ) )
        return PLP_OFFLINE;
    if ( 0 == strcmp( "MFCC_E_D_A_Z", strVal ) )
        return MFCC_E_D_A_Z;
    if ( 0 == strcmp( "MFCC", strVal ) )
        return MFCC;
    if ( 0 == strcmp( "FFT", strVal ) ) 
        return FFT;
    return UNKNOWN_FType;
}

int ReadConf(const char *conf_file, feature_conf *conf )
{
    FILE *fIconf = fopen( conf_file, "r" );
    if ( NULL == fIconf )
    {
        printf( "[%s:%d] Can not open config file.\n", __FILE__, __LINE__ );
        return -1;
    }
    char conf_key[512], conf_value[512];
    while( -1 != fscanf( fIconf, "%s %s", conf_key, conf_value ) )
    {
        switch( ConfKey( conf_key ) )
        {
            case TARGETKIND:
                (*conf).feat_type = FeatureType(conf_value);
                if ( UNKNOWN_FType == (*conf).feat_type )
                {
                    printf( "[%s:%d] Unknown feature type.\n", __FILE__, __LINE__ );
                    return -1;
                }
                break;
            case SAMPLERATE:
                (*conf).samplerate = atoi(conf_value);
                if ( 8000 != (*conf).samplerate && 16000 != (*conf).samplerate )
                {
                    printf( "[%s:%d] samplerate only support for 8000 and 16000.\n", __FILE__, __LINE__ );
                    return -1;
                }
                break;
            case PACKAGESIZE:
                (*conf).package_size = atoi( conf_value );
                break;
            case NUMCHANS:
                (*conf).num_chans = atoi( conf_value );
                break;
            case PREEMCOEFY:
                (*conf).pre_emph = atof( conf_value );
                break;
            case PREENERGY:
                (*conf).preEn = atof( conf_value );
                break;
            case CEPSDIM:
                (*conf).nCepsDim = atoi(conf_value); 
                break;
            case CEPSLIFTER:
                (*conf).nCepsLifer = atoi(conf_value);
                break;
            case USE_POWER:
                (*conf).use_power = atoi(conf_value);
                break;
            case LOG_POWER:
                (*conf).log_power = atoi(conf_value);
                break;
            case INITWIN:
                (*conf).initWin = atoi(conf_value);
                break;
            case HEADNUMFRAME:
                (*conf).aheadNumFrame = atoi(conf_value);
                break;
            case BETA:
                (*conf).beta = atof(conf_value);
                break;
            case IFHEAD:
                (*conf).if_head = atoi(conf_value);
                break;
            case IFSWAP:
                (*conf).if_swap = atoi(conf_value);
                break;
            case IFNOISE:
                (*conf).if_noise = atoi(conf_value);
                break;
            case IFCONVNOISE:
                (*conf).if_conv_noise = atoi(conf_value);
                break;
            case ISPULSE:
                (*conf).if_pulse = atoi(conf_value);
                break;
            case IFDRC:
                (*conf).if_drc = atoi(conf_value);
                break;
            case IFAPM:
                (*conf).if_apm = atoi(conf_value);
                break;
            case DRC_VERSION:
                (*conf).drc_version = atof(conf_value);
                break;
            case APM_VERSION:
                (*conf).apm_version = atof(conf_value);
                break;
            case NOISEFILELIST:
                strcpy((*conf).noise_file, conf_value);
                break;
            case IFMIXBAND:
                (*conf).if_mixband = atoi(conf_value);
                break;
            case MIXBAND_COMPENSATE_FILE:
                strcpy((*conf).mixband_compensate_file, conf_value);
                break;
            case NORM_FFT_BEFORE_MEL:
                (*conf).norm_fft_before_mel = atoi(conf_value);
                break;
            case FFT_NORM_FILE:
                strcpy((*conf).fft_norm_file, conf_value);
                break;
            case AMPLITUDE_SCALE:
                (*conf).amplitude_scale = atof(conf_value);
                break;
            case BV_AS_INPUT:
                (*conf).bv_as_input = atoi(conf_value);
                break;
            case DOWNSAMPLING:
                (*conf).if_downsampling = atoi(conf_value);
                break;
            case DATA_OFFSET:
                (*conf).data_offset = atoi(conf_value);
                break;
            case WAVE_IN:
                (*conf).wave_in = atoi(conf_value);
                break;
            case CONTEXT_LEFT:
                (*conf).context_left = atoi(conf_value);
                break;
            case CONTEXT_RIGHT:
                (*conf).context_right = atoi(conf_value);
                break;
            case IFREVERB:
                (*conf).if_reverb = atoi(conf_value);
                break;
            case REVERB_RATE:
                (*conf).reverb_rate = atof(conf_value);
                break;
            case REVERB_SMALL_ONLY:
                (*conf).reverb_small_only = atof(conf_value);
                break;
            case ADJUST_SPEED:
                (*conf).adjust_speed = atof(conf_value);
                break;
            default:
                printf( "[%s:%d] Unknown conf type: %s.\n", __FILE__, __LINE__, conf_key );
                return -1;
        }
    }

	fclose(fIconf);
    return 0;
}
#endif //_TOOL_ReadConf_H_
