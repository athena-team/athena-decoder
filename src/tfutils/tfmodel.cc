#include "tfmodel.h"

namespace inference {

int TFIO::get_index(std::vector<std::string>& output_names, std::string name) {
    std::vector<std::string>::iterator it = find(output_names.begin(), output_names.end(), name);
    if (it == output_names.end()){
        std::cerr << "Can not find output!" << std::endl;
        return -1;
    }

    return it - output_names.begin();
}

TFModel* init_model(std::string model_path, int num_threads) {
    TFModel* tfmodel = new TFModel();
    std::unique_ptr<GraphDef> graph_def;
    graph_def.reset(new GraphDef());
    // read graph from disk
    Status s = ReadBinaryProto(Env::Default(), model_path, graph_def.get());
    if (!s.ok()){
        std::cerr << "Could not create TensorFlow Graph: " << s << std::endl;
        return NULL;
    }

    // create session options
    SessionOptions options;
    ConfigProto& config = options.config;
    if (num_threads > 0) {
        config.set_intra_op_parallelism_threads(1);
        config.set_inter_op_parallelism_threads(num_threads);
    }
    tfmodel->session.reset(tensorflow::NewSession(options)); 
    s = tfmodel->session->Create(*(graph_def.get()));
    if (!s.ok()){
        std::cerr << "Could not create TensorFlow Session: " << s;
        return NULL;
    }
    return tfmodel;
}

template<typename T>
int TFIO::set_input(DATA_TYPE<T>& in) {
    std::vector<std::pair<std::string, Tensor> > &input_tensors = this->input_tensors;
    Tensor* tensor = nullptr;
    
    const std::unique_ptr<Buffer<T> > &buffer = get_buffer<T>(in);
    std::vector<std::size_t> &shape = get_shape<T>(in);
    std::vector<long long> tmp_shape;
    for (int i=0; i<shape.size(); i++) {
        tmp_shape.push_back(shape[i]);
    }
    
    TensorShape ts;
    Status s = TensorShapeUtils::MakeShape(tmp_shape, &ts);
    if (!s.ok()){
        std::cerr << "Error when make shape from vector: " << s;
        return -1;
    }

    std::int64_t num_elements = ts.num_elements();

    // Create Tensor
    input_tensors.emplace_back(std::pair<std::string, Tensor>(get_name(in),
            std::move(Tensor(tensorflow::DataTypeToEnum<T>::v(), ts))));
    T* tensor_ptr = (input_tensors[input_tensors.size()-1].second).flat<T>().data();
    std::copy_n(buffer->ptr(), num_elements, tensor_ptr);
    return 0;
}

int TFIO::set_output(const std::string name) {
    //set output name
    this->output_names.push_back(name);
    return 0;
}


template int TFIO::set_input(DATA_TYPE<float>&);
template int TFIO::set_input(DATA_TYPE<int>&);
}
