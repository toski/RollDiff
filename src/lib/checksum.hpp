#pragma once

#include <cstdint>

namespace rd
{

/// <summary>
/// Adler32 checksum algorithm function.
/// Should not be used directly for file hash since it can have collisions for similar chunks.
/// Details: https://en.wikipedia.org/wiki/Adler-32
/// </summary>
/// <typeparam name="InIterator">Forward iterator that implement increment(++) and dereference(*) operators</typeparam>
/// <param name="input">Forward iterator to the beginning of the input data. Keep in mind that it will be modified by this function</param>
/// <param name="data_length">Length of the input</param>
/// <returns>uint32_t representing checksum of the given data</returns>
template <typename InIterator>
uint32_t compute_checksum(InIterator& input, size_t data_length)
{
    size_t A = 1;
    size_t B = 0;

    constexpr size_t mod = 65521;
    for (size_t i = 0; i < data_length; ++i)
    {
        A += *input++;
        A = A % mod;

        B += A;
        B = B % mod;
    }

    return static_cast<uint32_t>((B << 16) + A);
}


}; // namespace rd