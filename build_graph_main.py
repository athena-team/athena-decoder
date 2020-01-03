import sys
from absl import logging
from graph import GraphBuilder

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    if len(sys.argv) != 4 and len(sys.argv) != 7:
        logging.warning(('Usage: python {} input_speller_file input_chars_file input_arpa_file\n'
            ' output_disambig_chars_file output_words_file output_fst_file'.format(sys.argv[0])))
        logging.warning('output_disambig_chars_file="characters_disambig.txt"\n'
                'output_words_file="words.txt"\n'
                'output_fst_file="SG.fst"')
    graph_builder = GraphBuilder()
    if len(sys.argv) == 4:
        graph_builder.make_graph(sys.argv[1], sys.argv[2], sys.argv[3])
    if len(sys.argv) == 7:
        graph_builder.make_graph(sys.argv[1], sys.argv[2], sys.argv[3],
                sys.argv[4], sys.argv[5], sys.argv[6])
