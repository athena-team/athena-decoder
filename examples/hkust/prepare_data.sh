#!/usr/bin/env bash

# This script prepare data for hkust dataset

mkdir -p ./data ./graph
(cd graph && ln -sf ../data/vocab graphemes.txt)
(cd graph && ln -sf ../data/lm_hkust.arpa lm.arpa)



