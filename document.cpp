#include <iostream>
#include "document.h"


// Constructors
Document::Document() {
}

Document::Document(std::string path) {
    this->setPath(path);
}


// Getters
std::string Document::getPath() {
    return this->path;
}

int Document::getSize() {
    return this->size;
}


// Setters
void Document::setPath(std::string path) {
    this->path = path;
}

void Document::setSize(int size) {
    this->size = size;
}
