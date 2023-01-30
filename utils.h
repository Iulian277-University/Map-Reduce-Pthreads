#ifndef UTILS_H
#define UTILS_H

int parse_input_file(std::string input_file_path, std::vector<Document> &documents);
int get_file_size(std::string filename);
std::map<int, std::vector<Document>> docs_greedy_split(std::vector<Document> documents, int num_mappers);

#endif
