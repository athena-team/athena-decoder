# Decoding-graph creation

Here we explain how to build wfst-based decoding graph, along with some data-preparation steps related to it.

The foundation of graph is [OpenFst](http://www.openfst.org/twiki/bin/view/FST/WebHome) and its python version [openfst-python](https://pypi.org/project/openfst-python/)
which uses [Openfst Python extension](http://www.openfst.org/twiki/bin/view/FST/PythonExtension).

The building steps and data-preparation scripts are reference to [Kaldi](http://kaldi-asr.org/).

Three data file are necessary for decoding graph creation:
- grapheme table file which contains all grapheme and corresponding index (usually graphemes.txt)
- lexicon file which contains the map from word to its grapheme sequence (usually lexicon.txt)
- language model in ARPA format (usually lm.arpa)

There will be three output file:
- new grapheme table file contains disambiguation symbols and epsilon (usually graphemes_disambig.txt) 
- word table file which contains all words and corresponding index (usually words.txt)
- the decoding graph (LG.fst or TLG.fst)

# Preparing grapheme table file

We need to prepare the OpenFst input symbol table from "grapheme.txt". The table assigns integer index to all the graphemes in system. Actually, the "grapheme.txt" file is the same as the vocab file used in Seq2Seq model. 

If the grapheme is character in Chinese, the example of how the grapheme table file looks like:

```
## head graphemes.txt
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

## tail graphemes.txt
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

A new grapheme table file which contains disambiguation symbols and epsilon will be produced later. The
generated table file is the input table file for the decoding graph. 
An example of how the input table file looks like:

```
## head graphemes_disambig.txt
<eps> 0
<unk> 1
一 2
丁 3
七 4
万 5
丈 6
三 7
上 8
下 9

## tail graphemes_disambig.txt
u 3644
v 3645
w 3646
x 3647
y 3648
z 3649
<space> 3650
#0 3651
#1 3652
#2 3653
......
```
When building the "TLG" type graph for CTC model and the grapheme is syllables, the "graphemes.txt" and "graphemes_disambig.txt" files look like:

```
## head graphemes.txt

SIL 0
h_T0#ao_T4 1
ii_T0#iao_T1 2
ii_T0#iao_T2 3
ii_T0#iao_T3 4
ii_T0#iao_T4 5
h_T0#ai_T4 6
s_T0#an_T4 7
z_T0#ai_T4 8
ch_T0#eng_T4 9

## tail graphemes.txt

t_T0#a_T2 1304
x_T0#van_T4 1305
x_T0#van_T3 1306
x_T0#van_T2 1307
x_T0#van_T1 1308
g_T0#u_T1 1309
g_T0#u_T2 1310
g_T0#u_T3 1311
g_T0#u_T4 1312
<blk> 1313

```
What you have to pay attention is that the "graphemes.txt" should contain blank symbols "\<blk\>"(usually is the last symbol) for "TLG" type graph.
```
## head graphemes_disambig.txt
<eps> 0
SIL 1
h_T0#ao_T4 2
ii_T0#iao_T1 3
ii_T0#iao_T2 4
ii_T0#iao_T3 5
ii_T0#iao_T4 6
h_T0#ai_T4 7
s_T0#an_T4 8
z_T0#ai_T4 9

## tail graphemes_disambig.txt

g_T0#u_T1 1310
g_T0#u_T2 1311
g_T0#u_T3 1312
g_T0#u_T4 1313
<blk> 1314
#0 1315
#1 1316
......

```


# Preparing the lexicon L

In lexicon file "lexicon.txt", all words are mapped to its grapheme sequence. The lexicon file shows how to spell or pronounce
a word.
An example looks like:
```
## head lexicon.txt
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

## tail lexicon.txt
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
If the grapheme is syllable, the lexicon looks like:
```
## head lexicon.txt
!SIL SIL
<unk> SIL
0 l_T0#ing_T2
1 ii_T0#i_T1
1 ii_T0#iao_T1
2 ee_T0#er_T2
2 ee_T0#er_T4
3 s_T0#an_T1
4 s_T0#iy_T4
5 uu_T0#u_T3


## tail lexicon.txt
鼬樱 ii_T0#iu_T4 ii_T0#ing_T1
鼬佐 ii_T0#iu_T4 z_T0#uo_T3
鼯 uu_T0#u_T2
鼹 ii_T0#ian_T3
鼹鼠 ii_T0#ian_T3 sh_T0#u_T3
鼷 x_T0#i_T1
鼽 q_T0#iu_T2
鼾 h_T0#an_T1
鼾声 h_T0#an_T1 sh_T0#eng_T1
齄 zh_T0#a_T1
```

According to the grapheme table file and lexicon file, the word table file (words.txt) and disambiguation grapheme file (graphemes_disambig.txt) will be produced.

```
## head words.txt
<eps> 0
<unk> 1
一丝不挂 2
一个人的旅行 3
一个孩子 4
一个窗口 5
一二星座 6
一传 7
一凡 8
一前一后 9

## tail words.txt
龙南 3470
龙宝 3471
龙桥 3472
龙玉 3473
龙舟 3474
龙阳 3475
龟头炎 3476
#0 3477
<s> 3478
</s> 3479

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



# Preparing decoding graph LG

After Creating the lexicon graph L and grammar graph G, we could use WFST compose algorithm to build decoding graph LG. Then the decoding graph LG could be 
optimized by determinization and minimization argorithms. Additional steps we have to do are as follows:
- remove the arc whose output label is "\<unk\>"
- replace all the disambiguation symbols with epsilon

After all these steps, we get the decoding graph LG


# Preparing decoding graph TLG

To build decoding graph "TLG" for CTC model, we need to create another WFST called T. The usage of T is compressing CTC sequences,
deleting blank symbols and removing duplicate symbols.

### Preparing the T

The function of T is deleting blank symbols and removing duplicate symbols. That is necessary for CTC model.
For example, There is a symbol sequence:
```
a a a <blk> b b <blk> b <blk>
```
After the process of T, the output sequence would be:
```
a b b
```
When building the T, we will map disambiguation symbols and blank symbol to epsilon. So There is no need to 
remove disambiguation symbols for "TLG" graph.



### Creating graph TLG

Eventually, we could build the decoding graph for CTC model by the Openfst composition algorithm.
Then we could  optimize the graph using determinization and minimization algorithm.

In summary, the creation of decoding graph would be:

```
TLG = comp(T,min(deter(LG)))
```







