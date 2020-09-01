#ifndef SRC_DECODABLE_AM_CTC_H_
#define SRC_DECODABLE_AM_CTC_H_
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
#include "decoder/decodable-itf.h"
#include "inference.h"

class DecodableCTC:public athena::DecodableInterface {
 public:
    DecodableCTC(BaseFloat scale, int blank_id, BaseFloat minus_blank,
            bool ctc_prune, BaseFloat ctc_threshold,
            BaseFloat prior_scale = 0.0,
            BaseFloat * prior_log_scores = NULL):
        scale_(scale),
        blank_id_(blank_id),
        minus_blank_(minus_blank),
        ctc_prune_(ctc_prune),
        ctc_threshold_(ctc_threshold), num_frames_calculated_(0) {
            prior_log_scores_ = prior_log_scores;
            prior_scale_ = prior_scale;
        }
    int Reset() {            // call this function when decoder reset
        num_frames_calculated_ = 0;
    }
    int CalCTCScores(void* am_handler, inference::Input* speech) {

        likes_.clear();
        inference::INFStatus status=inference::GetEncoderOutput(am_handler, speech, &likes_);
        if(inference::STATUS_ERROR == status){
            std::cerr<<"Inference Failed";
            return inference::STATUS_ERROR;
        }
        nrow_=likes_.logits.size();
        ncol_=likes_.logits[0].size();
        num_frames_calculated_+=nrow_;

        return inference::STATUS_OK;//return 0;
    }
    virtual int32 NumFramesReady() const {
        return num_frames_calculated_;
    }
    virtual bool IsLastFrame(int32 frame) const {
        assert(frame < NumFramesReady());
        return (frame == NumFramesReady() - 1);
    }
    virtual bool IsBlankFrame(int32 frame) const {
        assert(frame < NumFramesReady());
        frame=frame-(num_frames_calculated_-nrow_);
        double blkscore=-likes_.logits[frame][blank_id_-1];
        if(blkscore< this->ctc_threshold_){ 
            return true;
        }
        else { 
            return false;
        }
    }
    virtual bool IsCTCPrune() const {
        return this->ctc_prune_;
    }
    virtual BaseFloat LogLikelihood(int32 frame, int32 idx) {
        /*
         * return the index label's score in frame
         */
        assert(blank_id_ >= 1);
        assert(minus_blank_ >= 0);
        assert(frame < nrow_);
        assert(idx < ncol_);

        frame=frame-(num_frames_calculated_-nrow_);
        double score = 0.0;
        if(prior_log_scores_){
            score=likes_.logits[frame][idx-1] - (prior_scale_*prior_log_scores_[idx-1]);
        }else{
            score=likes_.logits[frame][idx-1];
        }
        if(idx==blank_id_){
            return scale_*(score-minus_blank_);
        }
        else{
            return scale_ * score;
        }
    }
    virtual int32 NumIndices() const {
        return ncol_;
    }

 private:
    
    inference::encoder_output likes_;
    int nrow_;
    int ncol_;
    /*
       for stream input, decodable object must remember the total frames that have been
       calculated,and then get am scores from likes_ treating num_frames_calculated_ as offsets.
       Thus the decodable object should not be created more than one time for speech until the decoder reset. 
       if there is only one decodable object for one decoder,then decodable object must call Reset() when decoder call Reset()
       */

    int num_frames_calculated_;
    BaseFloat scale_;
    int blank_id_;
    BaseFloat minus_blank_;
    bool ctc_prune_;
    BaseFloat ctc_threshold_;
    BaseFloat *prior_log_scores_;
    BaseFloat prior_scale_;
};
#endif  // SRC_DECODABLE_AM_CTC_H_
