#ifndef SRC_DECODABLE_AM_CTC_H_
#define SRC_DECODABLE_AM_CTC_H_
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
#include "decoder/decodable-itf.h"

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
    int CalCTCScores() {
    /*
     * calculate scores by ctc am models
     * do some calculation
     */
        nrow_ = 10;
        ncol_ = 10;
        for(int i=0;i<nrow_;i++){
            std::vector<float> scores_one_frame;
            for(int j=0;j<ncol_;j++){
                scores_one_frame.push_back(-(i*0.2+j*0.03));
            }
            ctc_log_scores.push_back(scores_one_frame);
        }
        return 0;
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
        return false;
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
        return ctc_log_scores[frame][idx];
    }
    virtual int32 NumIndices() const {
        return 10;
    }

 private:
    
    std::vector<std::vector<float> > ctc_log_scores;
    int nrow_;
    int ncol_;
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
