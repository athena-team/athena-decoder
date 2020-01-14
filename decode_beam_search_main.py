from absl import logging
from decoders import BeamSearchDecoder

class ToyE2EModel:
    """Toy E2E Model with necessary interfaces
    
    Interfaces:
        get_encoder_outputs: outputs of encoder for encoder-decoder structure model
            or just outputs for CTC model;the function should be execute only once
            for every input utterance;Additionally, the return value could be a tuple
            wrapping outputs of encoder and other necessary information, for example
            output mask.
        get_initial_packed_states: initial information for model inference; the return
            value could be a tuple wrapping any information.
        inference_one_step:callback function of E2E model;the function take encoder output
            current label input and corresponding inner information to inference scores for
            all the output labels; Additionally, the function should return the new inner
            information for next inference step.
    """
    def __init__(self):
        """Read  scores from file obtained in advance for this toy model"""
        self.step = 0
        self.scores_per_step = []
        batch = []
        with open('egs/hkust/beam_search_scores.txt', 'r') as f:
            for line in f:
                items = line.strip().split()
                if len(items) == 4:
                    if batch != []:
                        self.scores_per_step.append(batch)
                    batch = []
                else:
                    items = [float(item) for item in items]
                    batch.append(items)
        self.scores_per_step.append(batch)

    def get_encoder_outputs(self, input_feats):
        """return fake encoder outputs"""
        return [0.0 for _ in range(100)]

    def get_initial_packed_states(self):
        """return fake initial states"""
        return (0,)

    def inference_one_step(self, encoder_outputs, label_input, packed_inner_states):
        """model inference one step in batch

        Args:
            encoder_outputs: encoder outputs, shape=...
            label_input: current input labels, shape=[batch, label]
            packed_inner_states: inner states for every input label, shape=[batch, states]
        """
        batch_scores = self.scores_per_step[self.step]
        batch = len(batch_scores)
        self.step += 1
        return batch_scores, [(self.step,) for _ in range(batch)]


if __name__ == '__main__':

    logging.set_verbosity(logging.INFO)
    e2e_model = ToyE2EModel()
    input_feats = [0.0 for _ in range(100)]
    initial_packed_states = e2e_model.get_initial_packed_states()
    enc_outputs = e2e_model.get_encoder_outputs(input_feats)
    label_input = 3650
    vocab = {}
    with open('egs/hkust/data/vocab', 'r') as f:
        for line in f:
            word, idx = line.strip().split()
            vocab[int(idx)] = word
    decoder = BeamSearchDecoder()
    decoder.decode(enc_outputs, initial_packed_states, e2e_model.inference_one_step)
    trans_idx = decoder.get_best_path()
    trans = ' '.join([vocab[int(idx)] for idx in trans_idx])
    logging.info("predictions: {}".format(trans))




