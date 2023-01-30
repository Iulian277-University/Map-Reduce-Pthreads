#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <cstdlib>
#include "mapper.h"


// Constructors
Mapper::Mapper() {
}

Mapper::Mapper(int id) {
    this->setId(id);
}

Mapper::Mapper(int id, std::vector<Document> docs) {
    this->setId(id);
    for (const auto& doc : docs)
        this->addDoc(doc);
}


// `<` overload
bool operator< (const Mapper &m1, const Mapper &m2) {
    return m1.id < m2.id;
}

// `==` overload
bool operator== (const Mapper &m1, const Mapper &m2) {
    return m1.id == m2.id;
}


// Getters
int Mapper::getId() {
    return this->id;
}

std::vector<Document> Mapper::getDocs() {
    return this->docs;
}


// Setters
void Mapper::setId(int id) {
    this->id = id;
}

void Mapper::addDoc(Document doc) {
    this->docs.push_back(doc);
}


/**
 * @brief Process a given document with the current mapper and return the partial list
 * @param power_table A precalculated table used for checking if a number is a perfect power
 * @param doc The document to be processed
 * @param used_reducers The number of reducers
 * @return The partial list of the current document
*/
std::map<int, std::vector<long long>> Mapper::fmap(std::vector<std::set<long long>>* power_table,
                                                   Document doc, int num_reducers) {
    // Open doc
    std::string line;
    std::ifstream file;
    file.open(doc.getPath());

    // Initialize `partial_lists` with empty vectors
    std::map<int, std::vector<long long>> partial_lists;
    for (int exp = 2; exp <= num_reducers + 1; ++exp)
        partial_lists[exp] = std::vector<long long>();

    if (!file.is_open()) {
        std::cout << "Couldn't open the file `" << doc.getPath() << "`\n";
        return partial_lists;
    }

    // Number of lines (1 number / line)
    std::getline(file, line);
    int num_numbers = std::stoi(line);

    // Check conditions and add to the the specific partial list *or lists* (e.g.: 81 = 3^4 or 9^2)
    for (int i = 0; i < num_numbers; ++i) {
        std::getline(file, line);
        long long n = std::stoll(line);

        if (n <= 0)
            continue;

        for (int exp = 2; exp <= num_reducers + 1; ++exp) {
            // Check if `n` is perfect power of a x^`exp`
            if ((*power_table)[exp].count(n))
                partial_lists[exp].push_back(n);
        }
    }

    // Close doc
    file.close();

    return partial_lists;
}


/**
 * @brief Process all documents with the current mapper and return the final lists
 * @param power_table A precalculated table used for checking if a number is a perfect power
 * @param used_reducers The number of reducers used in the program
 * @return The final lists of the current mapper
*/
std::map<int, std::vector<long long>> Mapper::process(std::vector<std::set<long long>>* power_table, int num_reducers) {
    // Initialize `partial_lists` with empty vectors
    std::map<int, std::vector<long long>> final_lists;
    for (int exp = 2; exp <= num_reducers + 1; ++exp)
        final_lists[exp] = std::vector<long long>();

    // For each assigned document, call `fmap` and append to the `final_lists`
    for (Document doc : this->getDocs()) {
        std::map<int, std::vector<long long>> partial_lists = fmap(power_table, doc, num_reducers);
        for (auto const& pair : partial_lists) {
            int exp_id                     = pair.first;
            std::vector<long long> numbers = pair.second;
            for (long long number : numbers)
                final_lists[exp_id].push_back(number);
        }
    }

    return final_lists;
}
