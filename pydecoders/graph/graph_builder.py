"""Build decode graph

build decoder graph based on grammar WFST and
speller WFST using Compose algorithm
"""

import openfst_python as fst
from .grammar_builder import GrammarBuilder
from .lexicon_builder import LexiconBuilder
from .token_builder import TokenBuilder
from absl import logging

def remove_unk_arc(graph, unk_ids):
    """Replace unk symbol with eps """
    dead_state = graph.add_state()
    for state in graph.states():
        aiter = graph.mutable_arcs(state)
        while not aiter.done():
            arc = aiter.value()
            if arc.olabel in unk_ids:
                arc.nextstate = dead_state
                aiter.set_value(arc)
            aiter.next()
    graph.connect()

def remove_disambig_symbol(graph, disambig_ids):
    """Replace disambig symbols with eps"""
    for state in graph.states():
        aiter = graph.mutable_arcs(state)
        while not aiter.done():
            arc = aiter.value()
            if arc.ilabel in disambig_ids:
                arc.ilabel = 0
                aiter.set_value(arc)
            aiter.next()

class GraphBuilder:
    """Graph builder which use speller builder and grammar builder
    to produce decode graph"""
    def __init__(self, graph_type='LG'):
        """init graph builder

        Args:
            graph_type: 
                TLG for CTC model
                LG for Seq2Seq model like LAS, Transformer
        """
        self.lexicon_builder = LexiconBuilder()
        self.grammar_builder = GrammarBuilder()
        self.token_builder = TokenBuilder()
        self.graph = None
        self.graph_type = graph_type
        if self.graph_type != 'LG' and self.graph_type != 'TLG':
            logging.error('only support LG and TLG graph')
            raise NotImplementedError

    def make_graph(self, lexicon_file, graphemes_file, grammar_file, sil_symbol,
            disambig_graphemes_file='graphemes_disambig.txt',
            words_file='words.txt', graph_file='LG.fst'):
        """build decode graph and write to disk

        Args:
            lexicon_file: lexicon file from word to graphemes sequence
            graphemes_file: graphemes id file
            grammar_file: arpa language model file
            disambig_graphemes_file: ouput graphemes table from grapheme to id
                isymbols_table for result WFST
            words_file: output words table from word to id 
                osymbols_table for result WFST
            graph_file: write result graph to graph_file, default: LG.fst
        """

        if self.graph_type == 'TLG':
            L = self.lexicon_builder(lexicon_file, graphemes_file, sil_symbol)
            logging.info('grapheme should be  phones or syllables')
            logging.info('please confirm the sil symbol is "SIL" or some other you defined')
        elif self.graph_type == 'LG':
            L = self.lexicon_builder(lexicon_file, graphemes_file, sil_symbol)
            logging.info('grapheme should be characters')
            logging.info('please confirm the sil symbol is <space> or some other you defined')
        self.lexicon_builder.write_words_table(words_file)
        self.lexicon_builder.write_disambig_graphemes_table(disambig_graphemes_file)
        logging.info('make lexicon fst successfully')

        G = self.grammar_builder(grammar_file, words_file)
        logging.info('make grammar fst successfully')

        LG = fst.compose(L, G)
        logging.info('LG compose successfully')
        LG = fst.determinize(LG)
        logging.info('LG determinize successfully')
        LG.minimize()
        logging.info('LG minimize successfully')
        LG.arcsort(sort_type='ilabel')

        if self.graph_type == 'TLG':
            T = self.token_builder(disambig_graphemes_file)
            self.graph = fst.compose(T, LG)
            logging.info('TLG compose successfully')
            remove_unk_arc(self.graph, self.lexicon_builder.unk_ids)
            logging.info('TLG fst remove unk successfully')

        elif self.graph_type == 'LG':
            self.graph = LG
            remove_unk_arc(self.graph, self.lexicon_builder.unk_ids)
            logging.info('LG fst remove unk successfully')
            remove_disambig_symbol(self.graph, self.lexicon_builder.disambig_ids)
            logging.info('LG fst remove disambig  successfully')

        self.graph.arcsort(sort_type='ilabel')
        self.graph.write(graph_file)
        logging.info('write graph successfully')
        return self.graph
