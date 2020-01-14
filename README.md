
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
git clone https://github.com/didichuxing/athena-decoder.git
pip install -r requirements.txt
python setup.py bdist_wheel sdist
python -m pip install --ignore-installed dist/pydecoders-0.1.0*.whl
source ./tools/env.sh
```

