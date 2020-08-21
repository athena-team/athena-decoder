#!/usr/bin/env bash

# This script prepare data for aishell dataset

mkdir -p ./data ./graph
wget -P ./data http://www.openslr.org/resources/33/resource_aishell.tgz
(cd data && tar xvf resource_aishell.tgz)
awk '{print $1}' data/resource_aishell/lexicon.txt | grep -v "SPOKEN_NOISE\|SIL" | \
    awk -F "" '{
                    for(i=1;i<=NF;i++){
                        printf("%s",$i);
                    }
                    printf("\t");
                    for(i=1;i<=NF;i++){
                        printf("%s ",$i);
                    } 
                    printf("\n");
                }' > graph/lexicon.txt

sed -i '1i <unk> <unk>' graph/lexicon.txt
(cd graph && ln -sf ../data/vocab graphemes.txt)
python local/filter_lexicon.py graph/lexicon.txt graph/graphemes.txt > graph/lexicon_filter.txt
mv graph/lexicon_filter.txt graph/lexicon.txt



