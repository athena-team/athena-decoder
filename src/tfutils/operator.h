#ifndef InfHandle_H
#define InfHandle_H

#include "encoder.h"
#include "decoder.h"


struct InfHandle{
    Encoder* enc_handle;
    Decoder* dec_handle;

    ~InfHandle(){
        if(NULL!=enc_handle){
            delete enc_handle;
            enc_handle=NULL;
        }
        if(NULL!=dec_handle){
            delete dec_handle;
            dec_handle=NULL;
        }
    }
};


#endif
