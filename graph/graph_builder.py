"""Build decode graph

build decoder graph based on grammar WFST and
speller WFST using Compose algorithm
"""

import openfst_python as fst
from .grammar_builder import GrammarBuilder
from .speller_builder import SpellerBuilder
from absl import logging

class GraphBuilder:
    """Graph builder which use speller builder and grammar builder
    to produce decode graph
    """
    def __init__(self):
        self.speller_builder = SpellerBuilder()
        self.grammar_builder = GrammarBuilder()
        self.SG = None

    def make_graph(self, speller_file, chars_file, arpa_file,
            disambig_chars_file='characters_disambig.txt',
            words_file='words.txt', fst_file='SG.fst'):
        """build decode graph and write to disk

        Args:
            speller_file: speller file from word to character
            chars_file: character id file
            arpa_file: arpa language model file
            disambig_chars_file: ouput characters table from character to id
                isymbols_table for result WFST
            words_file: output words table from word to id 
                osymbols_table for result WFST
            fst_file: write result graph to fst_file, default: SG.fst
        """
        S = self.speller_builder(speller_file, chars_file)
        self.speller_builder.write_words_table(words_file)
        self.speller_builder.write_disambig_chars_table(disambig_chars_file)
        logging.info('make speller fst successfully')
        G = self.grammar_builder(arpa_file, words_file)
        logging.info('make grammar fst successfully')
        SG = fst.compose(S, G)
        logging.info('fst compose successfully')
        self.SG = fst.determinize(SG)
        logging.info('fst determinize successfully')
        self.SG.minimize()
        logging.info('fst minimize successfully')
        self.SG.arcsort(sort_type='ilabel')
        self.remove_unk_arc()
        logging.info('fst remove unk successfully')
        self.remove_disambig_symbol()
        logging.info('fst remove disambig  successfully')
        self.SG.arcsort(sort_type='ilabel')
        self.SG.write(fst_file)
        return self.SG

    def remove_unk_arc(self):
        """Replace unk symbol with eps """
        unk_ids = self.speller_builder.unk_ids
        dead_state = self.SG.add_state()
        for state in self.SG.states():
            aiter = self.SG.mutable_arcs(state)
            while not aiter.done():
                arc = aiter.value()
                if arc.olabel in unk_ids:
                    arc.next_state = dead_state
                    aiter.set_value(arc)
                aiter.next()
        self.SG.connect()

    def remove_disambig_symbol(self):
        """Replace disambig symbols with eps"""
        disambig_ids = self.speller_builder.disambig_ids
        for state in self.SG.states():
            aiter = self.SG.mutable_arcs(state)
            while not aiter.done():
                arc = aiter.value()
                if arc.ilabel in disambig_ids:
                    arc.ilabel = 0
                    aiter.set_value(arc)
                aiter.next()
