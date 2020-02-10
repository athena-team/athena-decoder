
from unittest import TestCase, main
from absl import logging
import openfst_python as fst
from pydecoders.graph.grammar_builder import convert_weight, GrammarBuilder 

class GrammarBuilderTestCase(TestCase):
    def test_convert_weight(self):
        self.assertEqual(convert_weight(-2.3), 5.295945713886305)
        self.assertEqual(convert_weight('-2.3'), 5.295945713886305)

    def test_grammar_builder(self):
        grammar_builder = GrammarBuilder()
        grammar_fst = grammar_builder('examples/hkust/graph/lm_hkust.arpa',
                'examples/hkust/graph/words.txt')
        self.assertNotEqual(-1, grammar_fst.start())
        self.assertNotEqual(0, grammar_fst.num_states())
        self.assertEqual('tropical', grammar_fst.weight_type())
        self.assertEqual('standard', grammar_fst.arc_type())

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    main()


