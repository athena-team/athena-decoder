
from unittest import TestCase, main
import os
from tempfile import TemporaryDirectory
from absl import logging
import openfst_python as fst
from graph_builder import GraphBuilder

class GraphBuilderTestCase(TestCase):
    def setUp(self):
        self.unittest_dir = TemporaryDirectory()

    def test_graph_builder(self):
        graph_builder = GraphBuilder()
        SG_fst = graph_builder.make_graph(
                speller_file='../egs/hkust/speller.txt',
                chars_file='../egs/hkust/characters.txt',
                arpa_file='../egs/hkust/lm_hkust.arpa',
                disambig_chars_file=os.path.join(self.unittest_dir.name, 'unittest_disambig_characters.txt'),
                words_file=os.path.join(self.unittest_dir.name, 'unittest_words.txt'),
                fst_file=os.path.join(self.unittest_dir.name, 'unittest_SG.fst')
                )
        self.assertNotEqual(-1, SG_fst.start())
        self.assertNotEqual(0, SG_fst.num_states())
        self.assertEqual('tropical', SG_fst.weight_type())
        self.assertEqual('standard', SG_fst.arc_type())

    def tearDown(self):
        self.unittest_dir.cleanup()

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    main()
