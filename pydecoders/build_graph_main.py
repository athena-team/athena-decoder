from absl import logging
from graph import GraphBuilder

def main(graph_type, input_lexicon_file, input_graphemes_file, input_grammar_file,
        output_disambig_graphemes_file='graphemes_disambig.txt',
        output_words_file='words.txt',
        output_graph_file='LG.fst'):
    graph_builder = GraphBuilder(graph_type)
    graph_builder.make_graph(
            input_lexicon_file,
            input_graphemes_file,
            input_grammar_file,
            output_disambig_graphemes_file,
            output_words_file,
            output_graph_file)

if __name__ == '__main__':
    logging.set_verbosity(logging.INFO)
    graph_type = 'LG'
    #graph_type = 'TLG'
    if graph_type == 'LG':
        logging.info('start to build  LG graph')
        main(graph_type,
            'examples/hkust/graph/lexicon.txt',
            'examples/hkust/graph/graphemes.txt',
            'examples/hkust/graph/lm_hkust.arpa',
            'examples/hkust/graph/graphemes_disambig.txt',
            'examples/hkust/graph/words.txt',
            'examples/hkust/graph/LG.fst')
    elif graph_type == 'TLG':
        logging.info('start to build  TLG graph')
        main(graph_type,
            'examples/callcenter/graph/lexicon.txt',
            'examples/callcenter/graph/graphemes.txt',
            'examples/callcenter/graph/lm.arpa',
            'examples/callcenter/graph/graphemes_disambig.txt',
            'examples/callcenter/graph/words.txt',
            'examples/callcenter/graph/TLG.fst')




