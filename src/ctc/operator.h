#ifndef InfHandle_H
#define InfHandle_H

#include "ctc.h"

struct InfHandle{
    Ctc* handle_;

    ~InfHandle(){
        if(NULL!=handle_){
            delete handle_;
            handle_=NULL;
        }
    }
};


#endif
