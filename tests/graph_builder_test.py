
from unittest import TestCase, main
import os
from tempfile import TemporaryDirectory
from absl import logging
import openfst_python as fst
from pydecoders.graph.graph_builder import GraphBuilder

class GraphBuilderTestCase(TestCase):
    def setUp(self):
        self.unittest_dir = TemporaryDirectory()

    def test_graph_builder(self):
        graph_builder = GraphBuilder(graph_type='LG')
        LG_fst = graph_builder.make_graph(
                lexicon_file='examples/hkust/graph/lexicon.txt',
                graphemes_file='examples/hkust/graph/graphemes.txt',
                grammar_file='examples/hkust/graph/lm_hkust.arpa',
                disambig_graphemes_file=os.path.join(self.unittest_dir.name, 'unittest_disambig_graphemes.txt'),
                words_file=os.path.join(self.unittest_dir.name, 'unittest_words.txt'),
                graph_file=os.path.join(self.unittest_dir.name, 'unittest_LG.fst')
                )
        self.assertNotEqual(-1, LG_fst.start())
        self.assertNotEqual(0, LG_fst.num_states())
        self.assertEqual('tropical', LG_fst.weight_type())
        self.assertEqual('standard', LG_fst.arc_type())

    def tearDown(self):
        self.unittest_dir.cleanup()

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    main()
