"""Beam search decoder for Seq2Seq model

Python version decoder which treat Seq2Seq models(attention, transformer, CTC)
as acoustic model. Record inner states of Seq2Seq models in token and execute beam
search or argmax decoding algorithm.
"""
import numpy as np
from absl import logging

class Token:
    """Token used in token passing decode algorithm.
    The token can be linked by another token or None.
    The token record the cost and inner packed states
    used in model
    """
    def __init__(self, acoustic_cost, prev_tok=None,
            cur_label=None, inner_packed_states=None):
        self.prev_tok = prev_tok
        self.cur_label = cur_label
        self.inner_packed_states = inner_packed_states
        if prev_tok is not None:
            self.cost = prev_tok.cost +  acoustic_cost
        else:
            self.cost = acoustic_cost
        self.rescaled_cost = -1.0 

class BeamSearchDecoder:
    """Beam search decoder for seq2seq models"""
    def __init__(self, max_active=4, min_active=0, beam=30.0,
            sos=3650, eos=3650, max_seq_len=100, max_active_local=4):
        """Init decoder

        Args:
            max_active: max active token one step
            min_active: min active token one step
            beam: beam for search
            sos: id of start symbol of sentence
            eos: id of end symbol of sentence
            max_seq_len: max length of sentence
            max_active_local: max active token from one token
        """
        self.cur_toks = []
        self.prev_toks = []
        self.completed_token_pool = []
        self.num_steps_decoded = -1
        self.max_active = max_active
        self.min_active = min_active
        assert(self.max_active >= self.min_active)
        self.beam = beam
        self.sos = sos
        self.eos = eos
        self.max_seq_len = max_seq_len
        self.max_active_local = max_active_local

    def init_decoding(self, initial_packed_states):
        """Init decoding states for every input utterance

        Args:
            initial_packed_states: initial packed states for callback function
        """
        self.cur_toks = []
        self.prev_toks = []
        self.completed_token_pool = []
        self.num_steps_decoded = 0
        tok = Token(0.0, None, [self.sos], initial_packed_states)
        self.cur_toks.append(tok)

    def decode(self, encoder_outputs, initial_packed_states, inference_one_step_fn):
        """using seq2seq model and WFST graph to decode input utterance

        Args:
            encoder_outputs: outputs of encoder for encoder-decoder structure model.
                             or just CTC outputs for ctc model.
            inference_one_step_fn: callback function of seq2seq model. the function
                                   take encoder_outputs,input label and inner states,
                                   then produce logscores and inner states.
                                   the function's signature is: logscores, packed_inner_states =
                                   fn(encoder_outputs, label_input, packed_inner_states)
            initial_packed_states: initial packed states for inference_one_step_fn callback function
        """
        self.init_decoding(initial_packed_states)
        while not self.end_detect():
            self.prev_toks = self.cur_toks
            self.cur_toks = []
            self.process_emitting(encoder_outputs, inference_one_step_fn)

    def end_detect(self):
        """determine whether to stop propagating"""
        if self.cur_toks and self.num_steps_decoded < self.max_seq_len:
            return False
        else:
            return True

    def process_emitting(self, encoder_outputs, inference_one_step_fn):
        """Process one step emitting states using callback function

        Args:
            encoder_outputs: encoder outputs
            inference_one_step_fn: callback function
        """
        cand_seqs = []
        inner_packed_states_array = []
        for tok in self.prev_toks:
            cand_seqs.append(tok.cur_label)
            inner_packed_states_array.append(tok.inner_packed_states)

        weight_cutoff = self.get_cutoff()
        
        all_log_scores, inner_packed_states_array = inference_one_step_fn(encoder_outputs,
                cand_seqs, inner_packed_states_array)

        for idx, tok in enumerate(self.prev_toks):
            if tok.cost <= weight_cutoff:
                if self.eos == np.argmax(all_log_scores[idx]):
                    self.deal_completed_token(tok,all_log_scores[idx][self.eos])
                    continue
                log_costs = np.array([-score for score in all_log_scores[idx]])
                
                if self.max_active_local is None:
                    reserved_idx = [idx for idx in range(len(log_costs))]
                else:
                    k = self.max_active_local
                    if k > len(log_costs):
                        k = len(log_costs)
                    reserved_idx = np.argpartition(log_costs, k-1)[:k-1]
                for next_idx in reserved_idx:
                    label = next_idx
                    if label == self.eos:
                        self.deal_completed_token(tok, all_log_scores[idx][self.eos])
                    else:
                        new_tok = Token(log_costs[next_idx], tok, tok.cur_label+[label],
                                inner_packed_states_array[idx])
                        self.cur_toks.append(new_tok)
        self.num_steps_decoded += 1                

    def get_cutoff(self):
        """get cutoff used in this step

        Returns:
            beam_cutoff: beam cutoff
        """
        costs = np.array([tok.cost for tok in self.prev_toks])
        best_cost = np.min(costs)
        beam_cutoff = best_cost + self.beam
        min_active_cutoff = float('inf')
        max_active_cutoff = float('inf')
        if len(costs) > self.max_active:
            k = self.max_active
            max_active_cutoff = costs[np.argpartition(costs, k-1)[k-1]]
        if max_active_cutoff < beam_cutoff:
            return max_active_cutoff
        if len(costs) > self.min_active:
            k = self.min_active
            if k == 0:
                min_active_cutoff = best_cost
            else:
                min_active_cutoff = costs[np.argpartition(costs, k-1)[k-1]]
        if min_active_cutoff > beam_cutoff:
            return min_active_cutoff
        else:
            return beam_cutoff

    def deal_completed_token(self, tok, eos_score):
        """deal completed token and rescale scores

        Args:
            state: the completed token
            eos_score: acoustic score of eos
        """
        tok.rescaled_cost = (tok.cost + (-eos_score))/self.num_steps_decoded
        self.completed_token_pool.append(tok)

    def get_best_path(self):
        """get decoding result in best completed path

        Returns:
            ans: id array of decoding results
        """
        if not self.completed_token_pool:
            logging.warning('do not encounter eos during decoding, return best uncompleted token ')
            best_uncompleted_tok = self.cur_toks[0]
            for tok in self.cur_toks:
                if best_uncompleted_tok.cost > tok.cost:
                    best_uncompleted_tok = tok
            best_uncompleted_tok.cur_label.pop(0)
            return best_uncompleted_tok.cur_label
        else:
            best_completed_tok = self.completed_token_pool[0]
            for tok in self.completed_token_pool:
                if best_completed_tok.rescaled_cost > tok.rescaled_cost:
                    best_completed_tok = tok
            best_completed_tok.cur_label.pop(0)
            return best_completed_tok.cur_label

