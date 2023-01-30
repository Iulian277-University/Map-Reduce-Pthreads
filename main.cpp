#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include "document.h"
#include "mapper.h"
#include "reducer.h"
#include "utils.h"


typedef struct ThreadArguments {
    int id;

    std::vector<std::set<long long>> *power_table;

    int used_mappers;
    int used_reducers;

    std::vector<Mapper>  *mappers;
    std::vector<Reducer> *reducers;

    std::vector<std::map<int, std::vector<long long>>> *all_partial_lists;

    pthread_barrier_t *barrier;
} thread_args;


/**
 * @brief Function that is shared by all threads and it is used to run all the mappers and reducers
 * @param arg The arguments of the current thread given as a structure 
 * @return NULL
 */
void *parallel_map_reduce(void *arg) {
    // Grab the args
    thread_args *targs = (thread_args *) arg;
    int thread_id = targs->id;

    std::vector<std::set<long long>> *power_table = targs->power_table;

    int used_mappers  = targs->used_mappers;
    int used_reducers = targs->used_reducers;

    std::vector<Mapper>  mappers  = *(targs->mappers);
    std::vector<Reducer> reducers = *(targs->reducers);

    std::vector<std::map<int, std::vector<long long>>> *all_partial_lists = targs->all_partial_lists;

    pthread_barrier_t *barrier = targs->barrier;

    // Mappers
    if (thread_id < used_mappers)
        (*all_partial_lists)[thread_id] = mappers[thread_id].process(power_table, used_reducers);
    
    // Barrier
    pthread_barrier_wait(barrier);

    // Reducers
    if (thread_id >= used_mappers)
        reducers[thread_id - used_mappers].process(*all_partial_lists);

    pthread_exit(NULL);
}


/**
 * @brief Entry point of the program
 * @param argc Number of arguments
 * @param argv Number of mapper threads, number of reducer threads, input file
 * @return 0 if everything went well, 1 otherwise
 */
int main(int argc, char *argv[]) {
    // Sanity check
    if (argc != 4) {
        std::cout << "[USAGE]: ./tema1 <num_mappers> <num_reducers> <input_file>\n";
        return -1;
    }

    // Parse the input
    int num_mappers, num_reducers;
    sscanf(argv[1], "%d", &num_mappers);
    sscanf(argv[2], "%d", &num_reducers);
    std::string input_file_path(argv[3]);

    // Precompute `power_table`
    std::vector<std::set<long long>> power_table = std::vector<std::set<long long>>(num_reducers + 1 + 2);
    for (int exp = 2; exp <= num_reducers + 1; ++exp) {
        for (long long x = 1; pow(x, exp) <= (1LL << 32) - 1; ++x) {
            power_table[exp].insert(pow(x, exp));
        }
    }   

    // Create the `Documents` vector
    std::vector<Document> documents;
    int res_parse = parse_input_file(input_file_path, documents);
    if (res_parse)
        return -1;

    for (auto &document : documents)
        document.setSize(get_file_size(document.getPath()));

    // Generate "partitions" based on documents size (map<mapper_id, docs>)
    std::map<int, std::vector<Document>> mappers_ids_docs = docs_greedy_split(documents, num_mappers);

    // Initialize the `mappers` vector (0 ... num_used_mappers)
    std::vector<Mapper> mappers;
    for (size_t id = 0; id < mappers_ids_docs.size(); ++id)
        mappers.push_back(Mapper(id));

    // Add documents to each mapper
    for (auto const& x : mappers_ids_docs) {
        int mapper_id              = x.first;
        std::vector<Document> docs = x.second;
        for (Document doc: docs)
            mappers[mapper_id].addDoc(doc);
    }

    // Initialize the `reducers` vector (num_used_mappers ... <num_used_mappers + num_reducers)
    std::vector<Reducer> reducers;
    for (size_t exp = 2, id = mappers_ids_docs.size(); id < mappers_ids_docs.size() + num_reducers; ++id, ++exp)
        reducers.push_back(Reducer(id, exp));

    // Initialize `all_partial_lists` with `mappers.size()` elements
    std::vector<std::map<int, std::vector<long long>>> all_partial_lists =
                                                std::vector<std::map<int, std::vector<long long>>>(mappers.size());

    // Number of mappers and reducers used for the task
    int used_mappers  = mappers.size();
    int used_reducers = reducers.size();
    int num_threads   = used_mappers + used_reducers;

    // Vector of threads
	pthread_t threads[num_threads];

    // Barrier waiting for `num_threads` threads
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, num_threads);

    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_args *targs = (thread_args *) calloc(1, sizeof(thread_args));
        targs->id                = i,
        targs->power_table       = &power_table;
        targs->used_mappers      = used_mappers;
        targs->used_reducers     = used_reducers;
        targs->mappers           = &mappers;
        targs->reducers          = &reducers;
        targs->all_partial_lists = &all_partial_lists;
        targs->barrier           = &barrier;

		int res = pthread_create(&threads[i], NULL, parallel_map_reduce, (void *) targs);

		if (res) {
            std::cout << "[ERROR]: Couldn't create thread with ID `" << i << "`\n";
            return -1;
		}
	}

    // Join threads
    void *status;
    for (int i = 0; i < num_threads; i++) {
		int res = pthread_join(threads[i], &status);

		if (res) {
            std::cout << "[ERROR]: Couldn't wait thread with ID `" << i << "`\n";
            return -1;
		}
	}
    
    // Destroy the barrier
    pthread_barrier_destroy(&barrier);

    return 0;
}
