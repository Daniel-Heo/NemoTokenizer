#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "nemo_tokenizer.h"

namespace py = pybind11;

PYBIND11_MODULE(nemo_tokenizer_core, m) {
    m.doc() = "C++ implementation of NemoTokenizer for Python";
    
    py::class_<NemoTokenizer>(m, "NemoTokenizerCore")
        .def(py::init<>())
        .def("loadTokenizer", &NemoTokenizer::loadTokenizer)
        .def("tokenize", &NemoTokenizer::tokenize, 
            py::arg("text"), py::arg("add_special_tokens") = true)
        .def("batch_tokenize", &NemoTokenizer::batch_tokenize, 
            py::arg("text"), py::arg("add_special_tokens") = true)
        .def("encode", &NemoTokenizer::encode, 
            py::arg("text"), py::arg("add_special_tokens") = true)
        .def("decode", &NemoTokenizer::decode, 
            py::arg("ids"), py::arg("skip_special_tokens") = true)
        .def("convert_tokens_to_ids", &NemoTokenizer::convert_tokens_to_ids,
            py::arg("tokens"), py::arg("add_special_tokens") = true)
        .def("convert_ids_to_tokens", &NemoTokenizer::convert_ids_to_tokens,
            py::arg("ids"), py::arg("skip_special_tokens") = true)
        .def("convert_tokens_to_text", &NemoTokenizer::convert_tokens_to_text,
            py::arg("tokens"), py::arg("skip_special_tokens") = true);
}