
#ifndef KALDI_ITF_DECODABLE_ITF_H_
#define KALDI_ITF_DECODABLE_ITF_H_ 1

#include <kaldi/kaldi-types.h>
#include <kaldi/kaldi-error.h>

namespace kaldi {

class DecodableInterface {
 public:
  virtual BaseFloat LogLikelihood(int32 frame, int32 index) = 0;

  virtual bool IsLastFrame(int32 frame) const = 0;

  virtual bool IsBlankFrame(int32 frame) const {
      return false;
  } 
  virtual bool IsCTCPrune() const {
      return false;
  } 

  virtual int32 NumFramesReady() const {
    KALDI_ERR << "NumFramesReady() not implemented for this decodable type.";
    return -1;
  }
  virtual int32 NumIndices() const = 0;

  virtual ~DecodableInterface() {}
};
}  

#endif  
