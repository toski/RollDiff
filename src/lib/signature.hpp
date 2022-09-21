#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

#include "hash.hpp"

namespace rd
{

/// <summary>
/// Basic building block of a data sequence
/// </summary>
struct chunk
{
	size_t start_position{0};
	size_t length{0};
	uint32_t hash{0};
};

/// <summary>
/// Structure describing data sequence.	
/// Consists of a sequence of chunks.
/// </summary>
struct signature
{
	std::vector<chunk> chunks{};

	/// <summary>
	/// Writes given signature object to a binary file
	/// </summary>
	/// <param name="os">File to which to save the given signature</param>
	/// <param name="sig">signature to save to the given binary file</param>
	/// <returns>given stream object</returns>
	static std::ostream& write_to_binary_file(std::ostream& os, const signature& sig);

	/// <summary>
	/// Reads signature structure from a binary file and saves it to the given signature object
	/// </summary>
	/// <param name="is">File from which to load the signature data</param>
	/// <param name="sig">signature that will be loaded from the given file</param>
	/// <returns>given stream object</returns>
	static std::istream& read_from_binary_file(std::istream& is, signature& sig);
};


bool operator==(const chunk& left, const chunk& right);
bool operator!=(const chunk& left, const chunk& right);
std::ostream& operator<<(std::ostream& os, const chunk& sig);

bool operator==(const signature& left, const signature& right);
bool operator!=(const signature& left, const signature& right);

/// <summary>
/// Creates a signature of the given data
/// </summary>
/// <typeparam name="InputIter">Forward iterator that implement increment(++) and dereference(*) operators</typeparam>
/// <param name="data">Forward iterator to the beginning of the input data.</param>
/// <param name="data_length">Length of the input</param>
/// <param name="chunk_length">How big should each chunk be</param>
/// <returns></returns>
template <typename InputIter>
signature calculate_signature(InputIter data, size_t data_length, size_t chunk_length)
{
	if (!*data)
	{
		throw std::invalid_argument("data parameter is nullptr!");
	}

	signature result;

	size_t data_index = 0;
	while (data_index < data_length)
	{
		chunk new_chunk;

		new_chunk.start_position = data_index;
		if ((data_index + chunk_length) < data_length)
		{
			new_chunk.length = chunk_length;
		}
		else
		{
			new_chunk.length = data_length - data_index;
		}

		new_chunk.hash = compute_hash(data, new_chunk.length);
		data_index += new_chunk.length;

		result.chunks.push_back(new_chunk);
	}

	return result;
};
	
}; // namespace rd
