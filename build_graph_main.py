from absl import logging
from graph import GraphBuilder

logging.set_verbosity(logging.INFO)
graph_builder = GraphBuilder()
speller_file = ''
chars_file = ''
arpa_file = ''
graph_builder.make_graph(speller_file, chars_file, arpa_file)
