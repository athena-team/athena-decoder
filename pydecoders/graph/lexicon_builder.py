"""Convert lexicon file to WFST format

Python version script
lexicon maps words to monophone or context-dependent phone
However, in seq2seq ASR we map words to graphemes just
like spell the words, we call it lexicon file
"""

import math
from collections import OrderedDict
import openfst_python as fst

class LexiconBuilder:
    """LexiconBuilder
    Builder class to convert lexicon file to WFST format
    """
    def __init__(self):
        self.lexicons = []
        self.disambig_graphemes = OrderedDict()
        self.words = OrderedDict()
        self.max_disambig = 0
        self.lexicon_fst = fst.Fst()
        self.start = 0
        self.last_s = 2

    def add_disambig(self):
        """add disambig symbols to lexicon items when necessary"""
        count = {}
        is_sub_seq = set()
        self.max_disambig = 0
        reserved = set()
        disambig_of = {}
        for _, grapheme_seq in self.lexicons:
            grapheme_seq_str = ' '.join(grapheme_seq)
            if grapheme_seq_str in count:
                count[grapheme_seq_str] += 1
            else:
                count[grapheme_seq_str] = 1
            tmp_grapheme_seq = grapheme_seq[:]
            tmp_grapheme_seq.pop()
            while tmp_grapheme_seq:
                is_sub_seq.add(' '.join(tmp_grapheme_seq))
                tmp_grapheme_seq.pop()
        for _, grapheme_seq in self.lexicons:
            grapheme_seq_str = ' '.join(grapheme_seq)
            if grapheme_seq_str not in is_sub_seq and count[grapheme_seq_str] == 1:
                pass
            else:
                if grapheme_seq_str == '':
                    self.max_disambig += 1
                    reserved.add(self.max_disambig)
                    grapheme_seq = ['#' + str(self.max_disambig)]
                else:
                    if grapheme_seq_str in disambig_of:
                        cur_number = disambig_of[grapheme_seq_str]
                    else:
                        cur_number = 0
                    cur_number += 1
                    while cur_number in reserved:
                        cur_number += 1
                    if cur_number > self.max_disambig:
                        self.max_disambig = cur_number
                    disambig_of[grapheme_seq_str] = cur_number
                    grapheme_seq.append('#' + str(cur_number))

    def create_words_table(self):
        """create words table according to lexicon file
        eps map to 0
        """
        words_list = []
        for word, _ in self.lexicons:
            words_list.append(word)
        words_list = list(set(words_list))
        words_list.sort()
        words_list.insert(0, '<eps>')
        words_list.append('#0')
        words_list.append('<s>')
        words_list.append('</s>')
        self.words = {word:idx for idx, word in enumerate(words_list)}

    def write_words_table(self, words_file='words.txt'):
        """write words table to file

        Args:
            words_file: write to words_file, default:words.txt
        """
        words_file = open(words_file, 'w')
        for word, idx in self.words.items():
            words_file.write('{} {}\n'.format(word, idx))
        words_file.close()

    def create_disambig_graphemes_table(self, graphemes_file):
        """ create disambig graphemes table
        this graphemes table is different from input graphemes table
        because of eps and disambig symbols
        """
        graphemes_list = []
        with open(graphemes_file, 'r') as f:
            for line in f:
                grapheme = line.strip().split()[0]
                graphemes_list.append(grapheme)
        graphemes_list.insert(0, '<eps>')
        disambig = 0
        self.max_disambig += 1 # for sil disambig
        while disambig <= self.max_disambig:
            graphemes_list.append('#' + str(disambig))
            disambig += 1
        self.disambig_graphemes = {grapheme:idx for idx, grapheme in enumerate(graphemes_list)}

    def write_disambig_graphemes_table(self, disambig_graphemes_file='graphemes_disambig.txt'):
        """write disambig graphemes table to file

        Args:
            disambig_graphemes_file: write disambig graphemes file to
                default:graphemes_disambig.txt
        """
        graphemes_file = open(disambig_graphemes_file, 'w')
        for grapheme, idx in self.disambig_graphemes.items():
            graphemes_file.write('{} {}\n'.format(grapheme, idx))
        graphemes_file.close()

    @property
    def disambig_ids(self):
        disambig_ids = [self.disambig_graphemes['#'+str(s)] for s in range(self.max_disambig+1)]
        return disambig_ids

    @property
    def unk_ids(self):
        unk_ids = []
        if '<unk>' in self.words:
            unk_ids.append(self.words['<unk>'])
        if '<UNK>' in self.words:
            unk_ids.append(self.words['<UNK>'])
        return unk_ids

    @property
    def words_table(self):
        return self.words

    @property
    def disambig_graphemes_table(self):
        return self.disambig_graphemes

    def make_lexicon_fst(self, sil_symbol, sil_prob):
        """Convert lexicon to WFST format
        There is always a disambig symbols after sil_symbol
        the special disambig symbols have been added
        in self.create_disambig_graphemes_table function

        Args:
            sil_prob: probability from end of a word to sil symbol
            sil_symbol: 'SIL' for phone-based ASR;'<space>' for
            graphemeacter-based ASR
        """
        sil_cost = -1.0 * math.log(sil_prob)
        no_sil_cost = -1.0 * math.log(1.0 - sil_prob)
        sil_disambig_id = self.disambig_graphemes['#' + str(self.max_disambig)]
        
        start_state = self.lexicon_fst.add_state()
        loop_state = self.lexicon_fst.add_state()
        sil_state = self.lexicon_fst.add_state()
        disambig_state = self.lexicon_fst.add_state()
        self.lexicon_fst.set_start(start_state)
        
        self.lexicon_fst.add_arc(start_state, fst.Arc(self.disambig_graphemes['<eps>'],
            self.words['<eps>'], no_sil_cost, loop_state))
        self.lexicon_fst.add_arc(start_state, fst.Arc(self.disambig_graphemes['<eps>'],
            self.words['<eps>'], sil_cost, sil_state))
        self.lexicon_fst.add_arc(sil_state, fst.Arc(self.disambig_graphemes[sil_symbol],
            self.words['<eps>'], 0.0, disambig_state))
        self.lexicon_fst.add_arc(disambig_state, fst.Arc(sil_disambig_id,
            self.words['<eps>'], 0.0, loop_state))
        
        for word, grapheme_seq in self.lexicons:
            word_id = self.words[word]
            grapheme_id_seq = [self.disambig_graphemes[grapheme] for grapheme in grapheme_seq]
            eps_id = self.words['<eps>']
            src = loop_state
            for pos, grapheme_id in enumerate(grapheme_id_seq[:-1]):
                des = self.lexicon_fst.add_state()
                if pos == 0:
                    self.lexicon_fst.add_arc(src, fst.Arc(grapheme_id, word_id, 0.0, des))
                else:
                    self.lexicon_fst.add_arc(src, fst.Arc(grapheme_id, eps_id, 0.0, des))
                src = des
            last_grapheme_id = grapheme_id_seq[-1]
            if len(grapheme_id_seq) == 1:
                self.lexicon_fst.add_arc(src, fst.Arc(last_grapheme_id, word_id, no_sil_cost, loop_state))
                self.lexicon_fst.add_arc(src, fst.Arc(last_grapheme_id, word_id, sil_cost, sil_state))
            else:
                self.lexicon_fst.add_arc(src, fst.Arc(last_grapheme_id, eps_id, no_sil_cost, loop_state))
                self.lexicon_fst.add_arc(src, fst.Arc(last_grapheme_id, eps_id, sil_cost, sil_state))
        self.lexicon_fst.set_final(loop_state, 0.0)
        self.lexicon_fst.add_arc(loop_state, fst.Arc(self.disambig_graphemes['#0'],
            self.words['#0'], 0.0, loop_state))
        self.lexicon_fst.arcsort(sort_type='olabel')

    def __call__(self, lexicon_file, graphemes_file, sil_symbol, sil_prob=0.5):
        """ caller interface for build lexicon WFST

        Args:
            lexicon_file: input lexicon file
            graphemes_file: input graphemes file
        Returns:
            lexicon_fst: output lexicon WFST
        """
        with open(lexicon_file, 'r') as f:
            for line in f:
                items = line.strip().split()
                word = items[0]
                grapheme_seq = items[1:]
                self.lexicons.append((word, grapheme_seq))
        self.add_disambig()
        self.create_words_table()
        self.create_disambig_graphemes_table(graphemes_file)
        self.make_lexicon_fst(sil_symbol, sil_prob)
        return self.lexicon_fst

