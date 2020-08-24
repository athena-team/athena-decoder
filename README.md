
# Athena-Decoder

*Athena-Decoder* is an open-source Automatic Speech Recognition (ASR) decoder tool implemented in python. 

Our vision is to offer an universal, standard and user-friendly decoder tool for all kinds of model including but not limited to CTC, Attention-based model and Transformer

## Key Features

- Support frame-synchronous models
- Support output-label-synchronous models
- WFST-based decoder introducing word-level information for seq2seq model

## Installation

This project has only been tested on Python 3. We recommend creating a virtual environment and installing the python requirements there.

```bash
git clone https://github.com/athena-team/athena-decoder.git
pip install -r requirements.txt
python setup.py bdist_wheel sdist
python -m pip install --ignore-installed dist/pydecoders-0.1.0*.whl
source ./tools/env.sh
```
## Build Graph
1. Build graph for HKUST
```bash
python pydecoders/build_graph_main.py examples/hkust/conf/hkust.conf
```
2. Build graph for AISHELL
```bash
# prepare data
cd examples/aishell && ./prepare_data.sh
# build graph
python pydecoders/build_graph_main.py examples/aishell/conf/aishell.conf
```


## Decode
1. WFST-based Decoder
```bash
# first build WFST graph,optional
# or you can skip this build step and use graph in hkust/graph directly 
python pydecoders/build_graph_main.py
# run decode step
python pydecoders/decode_wfst_main.py examples/hkust/conf/hkust.conf
```
2. Beam-Search decoder
```bash
# run beam search decoder 
python pydecoders/decode_beam_search_main.py examples/hkust/conf/hkust.conf
```
3. ArgMax decoder
+ modify the max_active parameter to 1 in BeamSearch decoder
+ run BeamSearch decoder

Please reference to build-graph.md and decoders.md in docs directory in detail.



