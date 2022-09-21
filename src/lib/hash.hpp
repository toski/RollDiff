#pragma once

#include <cstdint>

namespace rd
{

/// <summary>
/// Jenkins hash function
/// Used for computing hash of chunks and files.
/// Details: https://en.wikipedia.org/wiki/Jenkins_hash_function
/// </summary>
/// <typeparam name="InIterator">Forward iterator that implement increment(++) and dereference(*) operators</typeparam>
/// <param name="input">Forward iterator to the beginning of the input data. Keep in mind that it will be modified by this function</param>
/// <param name="data_length">Length of the input</param>
/// <returns>uint32_t representing hash of the given data</returns>
template <typename InIterator>
uint32_t compute_hash(InIterator& input, size_t data_length)
{
    size_t i = 0;
    uint32_t hash = 0;
    for (size_t i = 0; i < data_length; ++i)
    {
        hash += *input++;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}


}; // namespace rd