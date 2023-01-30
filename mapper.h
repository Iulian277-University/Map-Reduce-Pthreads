#ifndef MAPPER_H
#define MAPPER_H

#include "document.h"

class Mapper {
friend bool operator<  (const Mapper &m1, const Mapper &m2);
friend bool operator== (const Mapper &m1, const Mapper &m2);

private:
    int id;
    std::vector<Document> docs;

public:
    Mapper();
    Mapper(int id);
    Mapper(int id, std::vector<Document> docs);

    int getId();
    std::vector<Document> getDocs();

    void setId(int id);
    void addDoc(Document doc);

    // `fmap` processes *1* document and returns partial lists (x^2, x^3, ..., x^(num_reducers+1))
    std::map<int, std::vector<long long>> fmap(std::vector<std::set<long long>>* power_table, Document doc, int num_reducers);
    // This is the function that opens the files, uses `fmap` and closes the files
    std::map<int, std::vector<long long>> process(std::vector<std::set<long long>>* power_table, int num_reducers);
};

#endif
