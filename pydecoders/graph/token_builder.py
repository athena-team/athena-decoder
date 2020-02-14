
from collections import OrderedDict
import openfst_python as fst
from absl import logging

class TokenBuilder:
    def __init__(self):
        self.token_fst = fst.Fst()
        self.graphemes_table = OrderedDict()

    def read_disambig_graphemes_table(self, disambig_graphemes_file):
        with open(disambig_graphemes_file, 'r') as f:
            for line in f:
                items = line.strip().split()
                assert len(items) == 2
                self.graphemes_table[items[0]] = int(items[1])

    def make_token_fst(self, blank):
        start_state = self.token_fst.add_state()
        blank_start = self.token_fst.add_state()
        blank_end = self.token_fst.add_state()
        self.token_fst.set_start(start_state)
        self.token_fst.set_final(start_state, 0.0)
        blank_id = self.graphemes_table[blank]
        eps_id = self.graphemes_table['<eps>']
        assert eps_id == 0
        self.token_fst.add_arc(start_state, fst.Arc(0, 0, 0.0, blank_start))
        self.token_fst.add_arc(blank_start, fst.Arc(blank_id, 0, 0.0, blank_start))
        self.token_fst.add_arc(blank_end, fst.Arc(blank_id, 0, 0.0, blank_end))
        self.token_fst.add_arc(blank_end, fst.Arc(0, 0, 0.0, start_state))
        for token, idx in self.graphemes_table.items():
            if token == blank or token == '<eps>':
                continue
            # disambig symbols starts with '#'
            if token[0] == '#':
                self.token_fst.add_arc(start_state, fst.Arc(0, idx, 0.0, start_state))
            else:
                node = self.token_fst.add_state()
                self.token_fst.add_arc(blank_start, fst.Arc(idx, idx, 0.0, node))
                self.token_fst.add_arc(node, fst.Arc(idx, 0, 0.0, node))
                self.token_fst.add_arc(node, fst.Arc(0, 0, 0.0, blank_end))

    def __call__(self, disambig_graphemes_file, blank='<blk>'):
        self.read_disambig_graphemes_table(disambig_graphemes_file)
        logging.info('default blank symbol is <blk>, it must exist in graphemes.txt(usually the last symbol)')
        if (blank not in self.graphemes_table or
                '<eps>' not in self.graphemes_table):
            logging.error('blank symbol or <eps> not exists')
            raise IndexError
        self.make_token_fst(blank)
        self.token_fst.arcsort(sort_type='olabel')
        return self.token_fst


