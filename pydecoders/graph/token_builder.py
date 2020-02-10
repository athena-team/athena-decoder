
from collections import OrderedDict
import openfst_python as fst
from absl import logging

class TokenBuilder:
    def __init__(self):
        self.token_fst = fst.Fst()
        self.token_table = OrderedDict()

    def read_disambig_tokens_table(self, disambig_token_file):
        with open(disambig_token_file, 'r') as f:
            for line in f:
                items = line.strip().split()
                assert len(items) == 2
                self.token_table[items[0]] = int(items[1])

    def make_token_fst(self, blank):
        start_state = self.token_fst.add_state()
        blank_start = self.token_fst.add_state()
        blank_end = self.token_fst.add_state()
        self.token_fst.set_start(start_state)
        self.token_fst.set_final(start_state, 0.0)
        blank_id = self.token_table[blank]
        eps_id = self.token_table['<eps>']
        assert eps_id == 0
        self.token_fst.add_arc(start_state, fst.Arc(0, 0, 0.0, blank_start))
        self.token_fst.add_arc(blank_start, fst.Arc(blank_id, 0, 0.0, blank_start))
        self.token_fst.add_arc(blank_end, fst.Arc(blank_id, 0, 0.0, blank_end))
        self.token_fst.add_arc(blank_end, fst.Arc(0, 0, 0.0, start_state))
        for token, idx in self.token_table.items():
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

    def __call__(self, disambig_token_file, blank='<blk>'):
        self.read_disambig_tokens_table(disambig_token_file)
        if (blank not in self.token_table or
                '<eps>' not in self.token_table):
            logging.error('blank symbol or <eps> not exists')
            raise IndexError
        self.make_token_fst(blank)
        self.token_fst.arcsort(sort_type='olabel')
        return self.token_fst


