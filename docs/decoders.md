# Decoders
Currently, we supply two types of decoder: WFST-based decoder and beam-search decoder.
Both of these decoders could be used for frame-synchronous(CTC#todo) and output-label-synchronous(LAS, Transformer) models.
To make use of our decoders, you just need to implement the interfaces. Our decoders will use these the interfaces to
get scores and related states from your models. We will discuss these interfaces in detail lately.



# WFST-based decoder

To use the WFST-based decoder, you must build WFST decoding graph firstly. About the steps that how to build decoding
graph, please reference to document [build-graph.md]. The advantage of WFST-based decoder is that we could introduce
word level information to help finding better ASR results. It is a common need that incorporating word level information
when use Seq2Seq models.

# Beam-search decoder

Beam-search decoder execute beam search algorithm for your model. Many parameters can be set to control the 
decoding process. Actually if setting max_active parameter to 1, you get Arg-Max decoder.


# The Decodable interface

In order to minimize the interaction between the decoder and model, we design three operation. The signature
of these operation is as flowing:
    - encoder_outputs = get_encoder_outputs(input_feature)
    - (initial_packed_states,...) = get_initial_packed_states()
    - (batch_log_scores,batch_packed_states) = inference_one_step(encoder_outputs, current_input, current_packed_states)

The structure of most Seq2Seq model is encoder-decoder. The output of encoder will be used many times while decoding.
The function get_encoder_outputs receive input_feature and get encoder outputs. The encoder outputs should be an array
or list and each element maps to one timestep high level feature. 

The decoder(decoder of encoder-decoder structure) of Seq2Seq model is just like an recurrent neural network that generate output label step by step. 
To produce one output label this step, we must use current input label and hidden states last step. For different
model, hidden states are different(hidden states, alpha weights or just input label) and we just pack all necessary states into tuple.
Then we use them to produce labels and get new packed states for future steps. You should determine what states you want to use and return.
get_initial_packed_states function return the initial states for iteration

The third interface is inference_one_step function. The function takes outputs of encoder, input label of current step and corresponding
states to generate scores and states for next step. Notice that the current input and states must be orginazed in batch. The returned scores
and states are also orginazed in batch. The decoder will collect all the inputs and states and calculate them once for efficiency.
That is why we orginazed inputs, states and scores in batch.

We design a ToyE2EModel class to show how to implement these three interfaces. If the user are still confused about the interfaces, the class
may be helpful.








