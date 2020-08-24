"""Transfer Arpa tri-gram language model to WFST format

Python versioin arpa2fst scripts, support arbitrary 
arpa language model
"""

import math
import re
from absl import logging
import openfst_python as fst


class GrammarBuilder:
    """GrammarBuilder
    Builder class to transfer language model to WFST
    """
    def __init__(self, eps='<eps>', sb='<s>', se='</s>', ds='#0'):
        self.gram2state = {}
        self.grammar_fst = fst.Fst()
        self.order = 0
        self.eps = eps
        self.sb = sb
        self.se = se
        self.disambig_symbol = ds
        
        self.grammar_fst.add_state()
        self.gram2state[self.eps] = 0
        self.grammar_fst.set_start(0)

        self.words_table = {}
        self.max_order = 0

    def to_tropical(self, prob):
        """Convert probility to weight in WFST"""
        weight = -1.0 * math.log(10.0) * float(prob)
        return weight

    def arpa2fst(self, arpa_file):
        """
        Parse arpa language model and build WFST

        Args:
            arpa_file: arpa file to be convert
        """
        arpa = open(arpa_file, 'r')
        for line in arpa:
            line = line.strip()
            if line == '':
                continue
            elif line.startswith('#'):
                continue
            elif line.startswith('\\data'):
                continue
            elif line.startswith('ngram '):
                line = line.replace("ngram ", "")
                line = re.sub(r"=.*", "", line)
                self.max_order = int(line)
                logging.info("max order:{}".format(self.max_order))
                continue
            elif line.startswith('\\end'):
                continue
            elif line.startswith('\\'):
                self.order = int(line.replace('\\', '').replace('-grams:', ''))
                logging.info("reading {}-grams".format(self.order))
                continue
            assert self.order != 0
            if self.order == 1:
                self.process_unigram(line)
            elif self.order < self.max_order:
                self.process_middle_gram(line)
            elif self.order == self.max_order:
                self.process_highest_gram(line)
            else:
                pass
        arpa.close()

    def __call__(self, arpa_file, words_table_file):
        """functional callback

        Args:
            arpa_file: arpa file to be convert
            words_table_file: words table file from word to id
        Return:
            grammar_fst: result wfst
        """
        with open(words_table_file, 'r') as f:
            for line in f:
                word, idx = line.strip().split()
                self.words_table[word] = int(idx)
        self.arpa2fst(arpa_file)
        self.grammar_fst.arcsort(sort_type='ilabel')
        return self.grammar_fst

    def sid(self, symbol):
        """Wrapper for words table

        """
        if symbol not in self.words_table:
            raise IndexError
        else:
            return self.words_table[symbol]

    def find_state_of(self, gram):
        """ The map from gram to state """
        if gram not in self.gram2state:
            self.gram2state[gram] = self.grammar_fst.add_state()
        return self.gram2state[gram]

    def make_arc(self, isym, osym, weight, nextstate):
        """return the arc in WFST"""
        return fst.Arc(self.sid(isym), self.sid(osym), self.to_tropical(weight), nextstate)

    def process_unigram(self, gram):
        """Process unigram in arpa file"""
        parts = re.split(r'\s+', gram)
        boff = '0.0'
        if len(parts) == 3:
            prob, word, boff = parts
        elif len(parts) == 2:
            prob, word = parts
        else:
            raise NotImplementedError
        if word not in self.words_table:
            logging.debug('[{} {} {}] skipped: not in word table'.format(prob, word, boff))
            return
        if self.max_order == 1:
            src = self.find_state_of(self.sb)
            des = self.find_state_of(self.sb)
            self.grammar_fst.add_arc(src, self.make_arc(word, word, prob, des))
        elif word == self.se:
            src = self.find_state_of(self.eps)
            self.grammar_fst.set_final(src, self.to_tropical(prob))

        elif word == self.sb:
            pass
        else:
            src = self.find_state_of(word)
            des = self.find_state_of(self.eps)
            self.grammar_fst.add_arc(src, self.make_arc(self.disambig_symbol, self.eps, boff, des))
            self.grammar_fst.add_arc(des, self.make_arc(word, word, prob, src))

    def process_middle_gram(self, gram):
        """Process middle gram in arpa file"""
        parts = re.split(r'\s+', gram)
        for word in parts[1:self.order+1]:
            if word not in self.words_table:
                logging.debug('{} skipped: not in word table'.format(gram))
                return
        if parts[self.order] == self.se:
            src = self.find_state_of('/'.join(parts[1:self.order]))
            self.grammar_fst.set_final(src, self.to_tropical(parts[0]))
        else:
            boff = "0.0"
            if len(parts) == self.order+2:
                boff = parts[-1]
            src = self.find_state_of('/'.join(parts[1:self.order+1]))
            des = self.find_state_of('/'.join(parts[2:self.order+1]))
            self.grammar_fst.add_arc(src, self.make_arc(self.disambig_symbol, self.eps, boff, des))

            prob = parts[0]
            src = self.find_state_of('/'.join(parts[1:self.order]))
            des = self.find_state_of('/'.join(parts[1:self.order+1]))
            self.grammar_fst.add_arc(src,
                    self.make_arc(parts[self.order], parts[self.order], prob, des))

    def process_highest_gram(self, gram):
        """Process highest gram in arpa file"""
        parts = re.split(r'\s+', gram)
        for word in parts[1:self.order+1]:
            if word not in self.words_table:
                logging.debug('{} skipped: not in word table'.format(gram))
                return
        prob = parts[0]
        if parts[self.order] == self.se:
            src = self.find_state_of('/'.join(parts[1:self.order]))
            self.grammar_fst.set_final(src, self.to_tropical(prob))

        else:
            src = self.find_state_of('/'.join(parts[1:self.order]))
            des = self.find_state_of('/'.join(parts[2:self.order+1]))
            self.grammar_fst.add_arc(src,
                    self.make_arc(parts[self.order], parts[self.order], prob, des))
