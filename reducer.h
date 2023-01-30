#ifndef REDUCER_H
#define REDUCER_H

class Reducer {
friend bool operator<  (const Reducer &r1, const Reducer &r2);
friend bool operator== (const Reducer &r1, const Reducer &r2);

private:
    int id;
    int exp;

public:
    Reducer();
    Reducer(int id);
    Reducer(int id, int exp);

    int getId();
    int getExp();

    void setId(int id);
    void setExp(int exp);

    int process(std::vector<std::map<int, std::vector<long long>>> all_partial_lists);
};

#endif
