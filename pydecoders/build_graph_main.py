from absl import logging
from graph import GraphBuilder

def main(input_speller_file, input_chars_file, input_arpa_file,
        output_disambig_chars_file='characters_disambig.txt',
        output_words_file='words.txt',
        output_fst_file='SG.fst'):
    graph_builder = GraphBuilder()
    graph_builder.make_graph(
            input_speller_file,
            input_chars_file,
            input_arpa_file,
            output_disambig_chars_file,
            output_words_file,
            output_fst_file)

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    main('examples/hkust/graph/speller.txt',
        'examples/hkust/graph/characters.txt',
        'examples/hkust/graph/lm_hkust.arpa',
        'examples/hkust/graph/characters_disambig.txt',
        'examples/hkust/graph/words.txt',
        'examples/hkust/graph/SG.fst')
