#ifndef DOCUMENT_H
#define DOCUMENT_H

class Document {
private:
    std::string path;
    int size;

public:
    Document();
    Document(std::string path);
    
    std::string getPath();
    int getSize();

    void setPath(std::string path);
    void setSize(int size);
};

#endif
