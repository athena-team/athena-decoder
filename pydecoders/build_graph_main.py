from absl import logging
from graph import GraphBuilder
import json
import sys

def build_graph(graph_type, input_lexicon_file, input_graphemes_file, input_grammar_file,
        output_disambig_graphemes_file='graphemes_disambig.txt',
        output_words_file='words.txt',
        output_graph_file='LG.fst'):
    if graph_type == 'LG':
        logging.info('start to build  LG graph')
    elif graph_type == 'TLG':
        logging.info('start to build  TLG graph')
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
    if len(sys.argv) < 2:
        logging.warning('Usage: python {} config_json_file'.format(sys.argv[0]))
        sys.exit()
    json_file = sys.argv[1]
    config = None
    with open(json_file) as f:
        config = json.load(f)
    build_graph(config['graph']['graph_type'],
            config['graph']['input_lexicon_file'],
            config['graph']['input_graphemes_file'],
            config['graph']['input_grammar_file'],
            config['graph']['output_disambig_graphemes_file'],
            config['graph']['output_words_file'],
            config['graph']['output_graph_file'])
