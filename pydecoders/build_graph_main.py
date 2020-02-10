from absl import logging
from graph import SGGraphBuilder
from graph import TLGGraphBuilder

def main_sg(input_speller_file, input_chars_file, input_arpa_file,
        output_disambig_chars_file='characters_disambig.txt',
        output_words_file='words.txt',
        output_fst_file='SG.fst'):
    graph_builder = SGGraphBuilder()
    graph_builder.make_graph(
            input_speller_file,
            input_chars_file,
            input_arpa_file,
            output_disambig_chars_file,
            output_words_file,
            output_fst_file)

def main_tlg(input_lexicon_file, input_tokens_file, input_arpa_file,
        output_disambig_tokens_file='tokens_disambig.txt',
        output_words_file='words.txt',
        output_fst_file='TLG.fst'):
    graph_builder = TLGGraphBuilder()
    graph_builder.make_graph(
            input_lexicon_file,
            input_tokens_file,
            input_arpa_file,
            output_disambig_tokens_file,
            output_words_file,
            output_fst_file)

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    logging.info('start to build  SG graph')
    main_sg('examples/hkust/graph/speller.txt',
            'examples/hkust/graph/characters.txt',
            'examples/hkust/graph/lm_hkust.arpa',
            'examples/hkust/graph/characters_disambig.txt',
            'examples/hkust/graph/words.txt',
            'examples/hkust/graph/SG.fst')

    logging.info('start to build  TLG graph')
    main_tlg('examples/callcenter/graph/lexicon.txt',
            'examples/callcenter/graph/tokens.txt',
            'examples/callcenter/graph/lm.arpa',
            'examples/callcenter/graph/tokens_disambig.txt',
            'examples/callcenter/graph/words.txt',
            'examples/callcenter/graph/TLG.fst')




