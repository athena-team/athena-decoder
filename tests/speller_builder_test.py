
from unittest import TestCase, main
from absl import logging
import openfst_python as fst
from pydecoders.graph.speller_builder import SpellerBuilder 

class SpellerBuilderTestCase(TestCase):
    def test_speller_builder(self):
        speller_builder = SpellerBuilder()
        speller_fst = speller_builder('examples/hkust/graph/speller.txt',
                'examples/hkust/graph/characters.txt', sil_symbol='<space>')
        disambig_chars_table = speller_builder.disambig_chars_table
        words_table = speller_builder.words_table
        disambig_ids = speller_builder.disambig_ids
        self.assertNotEqual(-1, speller_fst.start())
        self.assertNotEqual(0, speller_fst.num_states())
        self.assertEqual('tropical', speller_fst.weight_type())
        self.assertEqual('standard', speller_fst.arc_type())
        self.assertEqual(disambig_chars_table['<eps>'], 0)
        self.assertTrue('#0' in words_table)
        self.assertFalse(disambig_ids == [])

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    main()


