# Decoding-graph creation

Here we explain how to build wfst-based decoding graph, along with some data-preparation steps related to it.

The foundation of graph is [OpenFst](http://www.openfst.org/twiki/bin/view/FST/WebHome) and its python version [openfst-python](https://pypi.org/project/openfst-python/)
which uses [Openfst Python extension](http://www.openfst.org/twiki/bin/view/FST/PythonExtension).

The building steps and data-preparation scripts are reference to [Kaldi](http://kaldi-asr.org/).

Three data file are necessary for decoding graph creation:
- character table file which contains all characters and corresponding index
- speller file which spell word to its character sequence
- language model in ARPA format

There will be three output file:
- new character table file contains disambiguation symbols and epsilon 
- word table file which contains all words and corresponding index the decoding graph

# Preparing character table file

We need to prepare the OpenFst input symbol table characters.txt. The table assign integer index to all the characters in system.
Actually, the character file is the same as the vocab file used in Seq2Seq model. An example of how the character table file look like is:

```
## head characters.txt
<unk> 0
一 1
丁 2
七 3
万 4
丈 5
三 6
上 7
下 8
不 9

## tail characters.txt
r 3640
s 3641
t 3642
u 3643
v 3644
w 3645
x 3646
y 3647
z 3648
<space> 3649
```

A new character table file which contains disambiguation symbols and epsilon will be produced later.

# Preparing the speller S

If you are familiar with WFST or ASR system, the speller file plays the role of lexicon. In Seq2Seq ASR system such as LAS and Transformer, the modeling unit is
always character. All words are mapped to its character sequence just like spell it. That is why we call it speller file. A small part of the speller file is:
```
## head speller.txt
<unk> <unk>
一丝不挂 一 丝 不 挂
一个人的旅行 一 个 人 的 旅 行
一个孩子 一 个 孩 子
一个窗口 一 个 窗 口
一二星座 一 二 星 座
一传 一 传
一凡 一 凡
一前一后 一 前 一 后
一千五百多 一 千 五 百 多

## tail speller.txt
鼻尖 鼻 尖
齐姐 齐 姐
龌 龌
龙南 龙 南
龙宝 龙 宝
龙桥 龙 桥
龙玉 龙 玉
龙舟 龙 舟
龙阳 龙 阳

```

According to the character table file and speller file, the word table file and new character table file will be produced.

```
speller_builder = SpellerBuilder()
S = self.speller_builder('speller.txt', 'characters.txt')
speller_builder.write_words_table('words.txt')
speller_builder.write_disambig_chars_table('characters_disambig.txt')
S.write('S.fst')

```

# Preparing the grammar G

The most part of grammar G is an acceptor with words as its symbols. The exception is the disambiguation symbol #0 which only apperas on the input side. Note
that we only support 3gram language model for now. 
We use the word table file generated when creating speller file and language model to build grammar G. 

```
grammar_builder = GrammarBuilder()
G = grammar_builder('lm.arpa', 'words.txt')
G.write('G.fst')
```

During buiding process, these operation have be done:
- skip the gram which contains the out-of-vocabulary words
- replace epsilons on the input side with the special disambiguation symbol #0
- sort the arc for every state in G according the input label index


# Preparing decoding graph SG

After Creating the speller graph S and grammar graph G, we could use WFST compose algorithm to build decoding graph SG. Then the decoding graph SG can be 
optimized by determinization and minimization. Additional steps we have to do are as follows:
- remove the arc whose output label is unk
- replace all the disambiguation symbols with epsilon

```
SG = fst.compose(S, G)
SG = fst.determinize(SG)
SG.minimize()
SG.arcsort(sort_type='ilabel')
remove_unk_arc()
remove_disambig_symbol()
SG.arcsort(sort_type='ilabel')
SG.write('SG.fst')

```
After all these steps, we get the decoding graph SG









