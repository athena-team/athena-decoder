#include <decoder/decodable-itf.h>
#include <cstdlib>
#include <cstring>
#include <assert.h>

class DecodableMatrixScaled: public athena::DecodableInterface {
    public:
        DecodableMatrixScaled( BaseFloat scale,int blank_id,BaseFloat minus_blank,
                bool ctc_prune,BaseFloat ctc_threshold, BaseFloat prior_scale,
                BaseFloat* prior_log_scores): 
            scale_(scale), 
            blank_id_(blank_id),
            minus_blank_(minus_blank),
            ctc_prune_(ctc_prune),
            ctc_threshold_(ctc_threshold),
            num_frames_calculated_(0){
                prior_log_scores_ = prior_log_scores;
                prior_scale_ = prior_scale;
            }

        int Reset(){ // call this function when decoder reset
            num_frames_calculated_=0;
        }
        int CalAMScores(){

            return  0;
        }

        virtual int32 NumFramesReady() const { return num_frames_calculated_; }

        virtual bool IsLastFrame(int32 frame) const {
            assert(frame < NumFramesReady());
            return (frame == NumFramesReady() - 1);
        }

        virtual bool IsBlankFrame(int32 frame) const {
            assert(frame < NumFramesReady());
            return false;
        }
        virtual bool IsCTCPrune() const{
            return this->ctc_prune_;
        }

        virtual BaseFloat LogLikelihood(int32 frame, int32 tid) {

            assert(blank_id_>=1);
            assert(minus_blank_>=0);
            return 1.1;
        }
        virtual int32 NumIndices() const { return 100;  }

    private:
        int num_frames_calculated_;

        BaseFloat scale_;
        int blank_id_;
        BaseFloat minus_blank_;
        bool ctc_prune_;
        BaseFloat ctc_threshold_;

        BaseFloat* prior_log_scores_;
        BaseFloat prior_scale_;
        
};


