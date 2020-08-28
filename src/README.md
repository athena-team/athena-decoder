
# Athena-Decoder-CPP

*Athena-Decoder-CPP* is the C++ version decoder for Automatic Speech Recognition (ASR).

Our goal is to offer an automated process including freezing acoustic model, building
decoding graph in WFST, speech recognition decoding and deploying ASR service (by HTTP or WebSocket)

In addition, some decoding techniqs will be proposed in this project 
including decoding strategies for label-synchronized model(Transformer), batched-decoder in GPU ,......

All these decoders are for industrial scenes.


## Installation

*Athena-Decoder-CPP* does not rely on other projects(usually Kaldi or Openfst).
*Athena-Decoder-CPP* could be compiled and used directly and independently


## Build Graph

1. Use Athena-Decoder to build graph by python, please reference to it 

2. The graph builded with Kaldi is also supported.
Actually, we will provide Shell scripts to build graph using 
Kaldi/Openfst operations.(Compose, minimize,determinize)

## Decoder

1. CTC-based Decoder
```bash
# prepare tools
cd tools && sh ./install_tools.sh

# make CTC decoder 
make

```

2. Transformer-WFST Decoder
3. Batched Decoder
