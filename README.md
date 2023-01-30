# MapReduce Paradigm

The purpose of this application is to count how many `perfect powers` are present in some given files.
This program is implemented in C++11 and uses the `Pthreads` library. 

The input consist of some text files, where each file contains one number per line. I want to show the fact that I can increase the speedup when performing the task in a parallel manner.

I implemented a parallel processing technique called [MapReduce](https://en.wikipedia.org/wiki/MapReduce) which has 3 major steps: `Map`, `Shuffle`, `Reduce`. The second step is not implemented in this project, because it was not required.

# Map

Starting from a list of `N` documents, each Mapper will have to process some documents. The way I decided to split the documents between Mappers was by implementing a simple heuristic which obtains good results, taking into consideration its complexity and the fact that this computation is done only at the beginning of the execution. Each mapper process its assigned documents and return some `partial_lists` containing the perfect powers (for each exponent) for all documents that were assigned for that specific mapper.

## Documents split heuristic

- `Idea`
    
    The first step is to sort the files descending by their size. 

    At each step I am calculating the `ideal_split_value` as being the sum of values from the current index to the last element (to the right) divided by the remaining free mappers.

    Then, I try to extend the number of documents (to the right), until the total size for this mapper it's closest to the ideal value calculated above.

    For a better understanding, I will illustrate the process with an example: Let's say I have 4 documents with the following sizes (in bytes): 16, 15, 21, 19 and I want to split this "array" in 3 partitions (3 mappers) as balanced as possible.

    - sort_desc() => 21 | 19 16 15
    
        ideal_split_value = (21 + 19 + 16 + 15) / 3 = 23. I try to extend the list which contains the element 21 with the next element 19 and see which case is better: abs(21 - 23) vs. abs(21 + 19 - 23). In this case, the heuristic says that I will assign only the file with the size 21 to this first mapper.

    - ... 19 | 16 15

        ideal_split_value = (19 + 16 + 15) / 2 = 25. I try to extend the file with size 19 to the right (but again, the preferred option is to stop here).

    - the process continues and the last mapper will get the last files (which will have the smallest sizes, because I did that non-ascending sorting at the beginning).


    This approach gives reasonable results, knowing that this is a NPH problem ([Partition problem](https://en.wikipedia.org/wiki/Partition_problem)).


- `Complexity`
  
    In the worst-case scenario, the complexity is $O(N^2)$, but in general it's roughly $O(N)$ if the files have similar sizes.


## Perfect power table

- `Idea`

    For checking if a number is a perfect power, I precomputed a table for all perfect powers, for all integers, starting with exponent `2`, until `num_reducers + 1`. This ensures me that the computation is done only once at the beginning of the program and I don't need to perform the same check for every number (which leads to a computationally expensive task)

    To give you an idea, the program ran in ~5 minutes only for 1 test, and now it runs ~20 seconds (15 times faster) and the problem can get worse if the documents are getting bigger.

    Another interesting thing is that the perfect powers tend to be more sparsed when I increase the base $a$ or the exponent $b$ in the expression $a^b$. This allows me to compute and store those values very fast and cheap.

- `Complexity`

    $O(reducers * log_2(n))$, where $n = x^{1/exp}$.


# Reduce

As I said, the mappers return some `partial_lists` containing the perfect powers (for each exponent) for all documents that were assigned for that specific mapper. When **all** the mappers ended up their jobs, reducers comes into play.

Each reducer:

- Aggregate the `partial_lists` for exponent `E` (for which it is responsible for).
- Count the unique values from the `aggregated_list` and write the value in a file.


# Pthreads

I created `num_mappers` + `num_reducers` threads and used a structure for sharing the `thread_id` and all the variables and pointers the threads needed. This allows me to avoid using global variables and it's a more elegant way of doing the things.


This is a snippet from the program. The barrier waits for all theads (`num_mappers` + `num_reducers`) because they can't differentiate between mappers and reducers when giving the number of waiters to the barrier. However, reducers will be the first that will hit the barrier, because they have nothing to compute before the barrier.

```C++
// Mappers
if (thread_id < used_mappers)
    (*all_partial_lists)[thread_id] = mappers[thread_id].process(power_table, used_reducers);

// Barrier
pthread_barrier_wait(barrier);

// Reducers
if (thread_id >= used_mappers)
    reducers[thread_id - used_mappers].process(*all_partial_lists);
```
