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
    main('egs/hkust/graph/speller.txt',
        'egs/hkust/graph/characters.txt',
        'egs/hkust/graph/lm_hkust.arpa',
        'egs/hkust/graph/characters_disambig.txt',
        'egs/hkust/graph/words.txt',
        'egs/hkust/graph/SG.fst')
