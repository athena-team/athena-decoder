#ifndef TFMODEL_H
#define TFMODEL_H

#include "type.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/util/stat_summarizer.h"
#include "tensorflow/cc/saved_model/tag_constants.h"

using namespace tensorflow;

namespace inference {

struct TFModel {
    std::unique_ptr<Session> session;
    char config[1024];
};

struct TFIO {
    std::vector<std::pair<std::string, tensorflow::Tensor> > input_tensors;
    std::vector<std::string > output_names;
    std::vector<tensorflow::Tensor> output_tensors;         //shape  [batch_size]

    void clear(){
        input_tensors.clear();
        output_names.clear();
        output_tensors.clear();
    }

    template<typename T>
    int set_input(DATA_TYPE<T>& in); // (tensor_name, tensor)

    int set_output(std::string tname); // (tensor_name)

    int get_index(std::vector<std::string>& output_names, std::string name);

};

TFModel* init_model(std::string model_path, int num_threads);

int model_run(const TFModel* tfmodel, TFIO* tfio);
}
#endif //end TFMODEL_H
