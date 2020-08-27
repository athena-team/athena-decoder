#ifndef KALDI_ITF_OPTIONS_ITF_H_
#define KALDI_ITF_OPTIONS_ITF_H_ 1
#include "kaldi/kaldi-types.h"
#include <kaldi/kaldi-error.h>
#include <kaldi/kaldi-utils.h>

namespace kaldi {

class OptionsItf {
 public:
  
  virtual void Register(const std::string &name,
                bool *ptr, const std::string &doc) = 0; 
  virtual void Register(const std::string &name,
                int32 *ptr, const std::string &doc) = 0; 
  virtual void Register(const std::string &name,
                uint32 *ptr, const std::string &doc) = 0; 
  virtual void Register(const std::string &name,
                float *ptr, const std::string &doc) = 0; 
  virtual void Register(const std::string &name,
                double *ptr, const std::string &doc) = 0; 
  virtual void Register(const std::string &name,
                std::string *ptr, const std::string &doc) = 0; 
  
  virtual ~OptionsItf() {}
};

}  // namespace Kaldi

#endif  // KALDI_ITF_OPTIONS_ITF_H_


