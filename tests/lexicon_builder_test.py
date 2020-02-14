
from unittest import TestCase, main
from absl import logging
import openfst_python as fst
from pydecoders.graph.lexicon_builder import LexiconBuilder 

class LexiconBuilderTestCase(TestCase):
    def test_lexicon_builder(self):
        lexicon_builder = LexiconBuilder()
        lexicon_fst = lexicon_builder('examples/hkust/graph/lexicon.txt',
                'examples/hkust/graph/graphemes.txt', sil_symbol='<space>')
        disambig_graphemes_table = lexicon_builder.disambig_graphemes_table
        words_table = lexicon_builder.words_table
        disambig_ids = lexicon_builder.disambig_ids
        self.assertNotEqual(-1, lexicon_fst.start())
        self.assertNotEqual(0, lexicon_fst.num_states())
        self.assertEqual('tropical', lexicon_fst.weight_type())
        self.assertEqual('standard', lexicon_fst.arc_type())
        self.assertEqual(disambig_graphemes_table['<eps>'], 0)
        self.assertTrue('#0' in words_table)
        self.assertFalse(disambig_ids == [])

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    main()


