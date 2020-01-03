import sys
import json
import time
import tensorflow as tf
from absl import logging
from athena.main import (
    build_model_from_jsonfile
)
import numpy as np
from athena.tools.wfst_decoder import WFSTDecoder

logging.set_verbosity(logging.INFO)
jsonfile = "egs/hkust/mtl_transformer.json"
p, model, _, checkpointer, dataset_builder = build_model_from_jsonfile(jsonfile, 0)
checkpointer.restore_from_best()
dataset_builder = dataset_builder.load_csv(p.test_csv).compute_cmvn_if_necessary(True)
dataset = dataset_builder.as_dataset(batch_size=1)
vocab = []
with open(p.dataset_config['text_config']['model'],'r') as f:
    for line in f:
        char = line.strip().split()[0]
        vocab.append(char)
words = []
with open('egs/hkust/build_graph/words.txt','r') as f:
    for line in f:
        word = line.strip().split()[0]
        words.append(word)

decoder = WFSTDecoder("egs/hkust/build_graph/SG.fst")
logging.info("load WFST graph successfully")
trans_file = open("egs/hkust/trans.txt",'w')
ground_truth_file = open("egs/hkust/ground_truth.txt",'w')

total_feats = 0
total_time = 0.0
for index, sample in enumerate(dataset):
    sample = model.prepare_samples(sample)
    total_feats += sample['input_length'].numpy()[0]
    ground_truth = sample['output'][0]
    ground_truth = ' '.join([vocab[int(idx)] for idx in ground_truth])
    enc_outputs = model.model.decode(sample, None, return_encoder=True)

    begin = time.time()
    decoder.decode(enc_outputs, (0,), model.model.inference_one_step)
    trans = decoder.get_best_path()
    total_time += time.time()-begin
    trans = ' '.join([words[int(idx)] for idx in trans])
    logging.info("predictions: {} labels: {}".format(trans, ground_truth))
    trans_file.write("{}\n".format(trans))
    trans_file.flush()
    ground_truth_file.write("{}\n".format(ground_truth))
    ground_truth_file.flush()
logging.info("decoding total time is {}\n".format(total_time))
logging.info("total feat num is {}\n".format(total_feats))
trans_file.close()   
ground_truth_file.close()
