#include <kaldi/decodable-itf.h>
#include <cstdlib>
#include <cstring>

class DecodableMatrixScaled: public kaldi::DecodableInterface {
    public:
        DecodableMatrixScaled( kaldi::BaseFloat scale,int blank_id,kaldi::BaseFloat minus_blank,
                bool ctc_prune,kaldi::BaseFloat ctc_threshold, kaldi::BaseFloat prior_scale,
                kaldi::BaseFloat* prior_log_scores): 
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
            KALDI_ASSERT(frame < NumFramesReady());
            return (frame == NumFramesReady() - 1);
        }

        virtual bool IsBlankFrame(int32 frame) const {
            KALDI_ASSERT(frame < NumFramesReady());
            return false;
        }
        virtual bool IsCTCPrune() const{
            return this->ctc_prune_;
        }

        virtual kaldi::BaseFloat LogLikelihood(int32 frame, int32 tid) {

            KALDI_ASSERT(blank_id_>=1);
            KALDI_ASSERT(minus_blank_>=0);
            return 1.1;
        }
        virtual int32 NumIndices() const { return 100;  }

    private:
        int num_frames_calculated_;

        kaldi::BaseFloat scale_;
        int blank_id_;
        kaldi::BaseFloat minus_blank_;
        bool ctc_prune_;
        kaldi::BaseFloat ctc_threshold_;

        kaldi::BaseFloat* prior_log_scores_;
        kaldi::BaseFloat prior_scale_;
        
};


