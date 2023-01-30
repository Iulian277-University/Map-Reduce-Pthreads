#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <numeric>
#include "document.h"

/**
 * @brief Parse the input file and create a list of documents of type `Document`
 * @param input_file The path of the file to be parsed
 * @return 1 if the file was parsed successfully, 0 otherwise
*/
int parse_input_file(std::string input_file_path, std::vector<Document> &documents) {
    // Open the file
    std::string line;
    std::ifstream file;
    file.open(input_file_path);

    if (!file.is_open()) {
        std::cout << "Couldn't open the file `" << input_file_path << "`\n";
        return -1;
    }

    // Number of documents
    std::getline(file, line);
    int num_documents = std::stoi(line);

    for (int i = 0; i < num_documents; ++i) {
        std::getline(file, line);
        documents.push_back(Document(line));
    }

    // Close the file
    file.close();

    return 0;
}


/**
 * @brief Given a file, return the size of the file in bytes
*/
int get_file_size(std::string filename) {
    std::ifstream in_file(filename.c_str(), std::ifstream::ate | std::ifstream::binary);
    in_file.seekg(0, std::ifstream::end);
    return int(in_file.tellg()); 
}


/**
 * @brief Given a list of documents, and the number of mappers, split the documents in a greedy manner.
 * The goal is to have some equal size partitions for each mapper. At each step, an `ideal_split_value`
 * is computed and try to extend the current partition until the size of the partition is greater than
 * the `ideal_split_value`. In this way, the partitions will be as equal as possible.
 * @param documents The list of documents to be split between mappers
 * @param num_mappers The number of mappers
 * @return A map with the mapper id as key and a list of documents as value
*/
std::map<int, std::vector<Document>> docs_greedy_split(std::vector<Document> documents, int num_mappers) {
    // Sort the documents descending by their size
    sort(documents.begin(), documents.end(), [](Document& doc1, Document& doc2) {
        return doc2.getSize() < doc1.getSize();
    });

    std::map<int, std::vector<Document>> mappers_docs;

    int mapper_idx = 0;
    int start = 0;
    int stop  = documents.size();
    for (int i = start; i < stop; ) {
        // Idea of improvement: calculate some partial sums from right to left
        // before entering this `for(start ... stop`). This tip will reduce the complexity
        // because we will no longer need to compute the sum from the current index to the right
        // Obs: The number of files and reducers is not very large, therefore this approach is still performing well 
        int ideal_split_value =
                    accumulate(
                        documents.begin() + i,
                        documents.end(),
                        0,
                        [](int acc, Document& doc) {return acc + doc.getSize();}) / (num_mappers - mapper_idx);
        
        // Last mapper
        if (num_mappers - mapper_idx == 1) {
            for (int j = i; j < stop; ++j)
                mappers_docs[mapper_idx].push_back(documents[j]);
            break;
        }

        // Try to extend the number of documents for this current mapper to the right,
        // until the `split_value` will be closest to the `ideal_split_value`
        if (documents[i].getSize() >= ideal_split_value) {
            mappers_docs[mapper_idx].push_back(documents[i]);
            ++i;
            ++mapper_idx;
        } else {
            int j = i;
            int partial_sum = 0;
            for (j = i; j < stop - 1; ++j) {
                partial_sum += abs(documents[j].getSize() + documents[j + 1].getSize());
                mappers_docs[mapper_idx].push_back(documents[j]);
                if (abs(documents[j].getSize() - ideal_split_value) < abs(partial_sum - ideal_split_value))
                    break;
            }
            i = j + 1;
            ++mapper_idx;
        }
    }

    return mappers_docs;
}
