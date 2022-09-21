#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

#include <exception>
#include <unordered_map>
#include <set>

#include "hash.hpp"
#include "signature.hpp"

namespace rd
{

/// <summary>
/// Delta keeps information and data needed to patch original data in order to get the modified data.
/// </summary>
struct delta
{
	/// <summary>
	/// Instruction what to do whit data in order to patch the original file
	/// command: Wan be 'COPY_DATA' or 'COPY_CHUNK'
	/// start_index: Where does the data starts in the original file. Used for 'COPY_CHUNK' instruction.
	/// data_length: Length of the data to copy. Used for 'COPY_CHUNK' instruction.
	/// data: Data to copy to the new file. Used for 'COPY_DATA' instruction.
	/// chunk_id: id of the chunk in the original file. Mostly used for debugging purposes.
	/// </summary>
	struct instruction
	{
		std::string command{ "ERROR" };
		size_t start_index{ 0 };
		size_t data_length{ 0 };
		std::vector<char> data;
		size_t chunk_id{ 0 };
	};

	std::vector<instruction> instructions;
	size_t data_length{ 0 };

	/// <summary>
	/// Writes given delta object to a binary file
	/// </summary>
	/// <param name="os">File to which to save the given delta</param>
	/// <param name="del">delta to save to the given binary file</param>
	/// <returns>given stream object</returns>
	static std::ostream& write_to_binary_file(std::ostream& os, const delta& del);

	/// <summary>
	/// Reads delta structure from a binary file and saves it to the given delta object
	/// </summary>
	/// <param name="is">File from which to load the delta data</param>
	/// <param name="del">delta that will be loaded from the given file</param>
	/// <returns>given stream object</returns>
	static std::istream& read_from_binary_file(std::istream& is, delta& del);
};

std::ostream& operator<<(std::ostream& os, const delta& dek);
std::istream& operator>>(std::istream& is, delta& del);



/// <summary>
/// Creates delta object from signature of the original data and the modified data
/// </summary>
/// <typeparam name="InputIter">Forward iterator that implement increment(++) and dereference(*) operators</typeparam>
/// <param name="sig">signature of the original data</param>
/// <param name="input">Iterator to the beginning of the modified data</param>
/// <param name="input_length">Length of the modified data</param>
/// <returns>delta structure describing changes in the modified file</returns>
template <typename InputIter>
delta calculate_delta(const signature& sig, InputIter input, size_t input_length)
{
	if (!*input)
	{
		throw std::invalid_argument("input parameter is nullptr!");
	}

	if (sig.chunks.size() == 0)
	{
		throw std::invalid_argument("Signature is empty! ");
	}

	delta result;

	// helper structures
	std::unordered_map<uint32_t, chunk> hash_chunk_map;
	std::unordered_map<uint32_t, size_t> hash_chunk_id_map;
	std::set<size_t> chunk_lengths;
	for (size_t i = 0; i < sig.chunks.size(); ++i)
	{
		hash_chunk_map.insert({ sig.chunks[i].hash, sig.chunks[i] });
		hash_chunk_id_map.insert({ sig.chunks[i].hash, i });
		chunk_lengths.insert(sig.chunks[i].length);
	}
	size_t data_index = 0;  // points to part of the input data that is not yet added to the delta structure
	size_t chunk_index = 0; // points to start of potential chunk that we are looking for in the input data


	// We need this helper buffer in case that we are dealing with stream iterators.
	// In that case we can use 'input' to iterate through input array only once.
	std::vector<char> input_buffer;
	const auto max_chunk_length = *chunk_lengths.crbegin();
	input_buffer.reserve(max_chunk_length * 2);
	size_t bytes_read_into_input_buffer = std::min(max_chunk_length * 2, input_length);
	for (size_t i = 0; i < bytes_read_into_input_buffer; ++i)
	{
		input_buffer.push_back(*input++);
	}
	size_t input_buffer_index = 0;

	while (data_index < input_length)
	{
		const bool we_cant_match_any_chunk_any_more = chunk_index + *chunk_lengths.cbegin() > input_length;
		if (we_cant_match_any_chunk_any_more)
		{
			result.instructions.push_back(impl::create_copy_data_instruction(data_index, input_length - data_index, input_buffer.cbegin()));
			result.data_length += result.instructions.back().data_length;

			return result;
		}

		// for every chunk length see if current chunk is original chunk
		// we are trying to match longer chunks first
		bool chunk_was_matched = false;
		auto length_iter = chunk_lengths.crbegin();
		while (length_iter != chunk_lengths.crend())
		{
			if ((chunk_index + *length_iter) > input_length)
			{
				// we cant match this chunk because it is too long
				++length_iter;
				continue;
			}

			auto chunk_hash = compute_hash(input_buffer.cbegin() + input_buffer_index, *length_iter);
			auto chunk_iter = hash_chunk_map.find(chunk_hash);
			const bool original_chunk_found = chunk_iter != hash_chunk_map.cend();
			if (original_chunk_found)
			{
				const bool we_have_some_data_to_copy_before_this_chunk = chunk_index > data_index;
				if (we_have_some_data_to_copy_before_this_chunk)
				{
					const auto data_index_in_buffer = data_index - (bytes_read_into_input_buffer - input_buffer.size());
					const auto chunk_index_in_buffer = chunk_index - (bytes_read_into_input_buffer - input_buffer.size());
					const auto length_of_data_to_write = chunk_index_in_buffer - data_index_in_buffer;
					assert(chunk_index_in_buffer == input_buffer_index);

					result.instructions.push_back(impl::create_copy_data_instruction(data_index, length_of_data_to_write, input_buffer.cbegin() + data_index_in_buffer));
					result.data_length += result.instructions.back().data_length;
				}

				delta::instruction new_instruction;
				new_instruction.command = "COPY_CHUNK";
				new_instruction.start_index = chunk_iter->second.start_position;
				new_instruction.data_length = chunk_iter->second.length;
				new_instruction.chunk_id = hash_chunk_id_map[chunk_hash];
				result.instructions.push_back(new_instruction);
				result.data_length += new_instruction.data_length;

				chunk_index += chunk_iter->second.length;
				input_buffer_index += chunk_iter->second.length;
				data_index = chunk_index;

				impl::refill_input_buffer(input, input_length, input_buffer, input_buffer_index, bytes_read_into_input_buffer);
				chunk_was_matched = true;
				break;
			}

			++length_iter;
		}

		if (chunk_was_matched)
		{
			chunk_was_matched = false;
			continue;
		}

		// Let's move up the data stream
		++chunk_index;
		++input_buffer_index;

		const bool add_data_to_input_buffer = input_buffer_index >= max_chunk_length;
		if (add_data_to_input_buffer)
		{
			const bool put_unmached_data_into_delta = data_index < chunk_index;
			if (put_unmached_data_into_delta)
			{
				const auto data_index_in_buffer = data_index - (bytes_read_into_input_buffer - input_buffer.size());
				const auto chunk_index_in_buffer = chunk_index - (bytes_read_into_input_buffer - input_buffer.size());
				const auto length_of_data_to_write = chunk_index_in_buffer - data_index_in_buffer;
				assert(chunk_index_in_buffer == input_buffer_index);

				// copy first half of the input_buffer to the delta
				result.instructions.push_back(impl::create_copy_data_instruction(data_index, length_of_data_to_write, input_buffer.cbegin() + data_index_in_buffer));
				result.data_length += result.instructions.back().data_length;

				data_index += length_of_data_to_write;
				assert(chunk_index == data_index);
			}

			impl::refill_input_buffer(input, input_length, input_buffer, input_buffer_index, bytes_read_into_input_buffer);
		}
	}

	return result;
};


/// <summary>
/// Helper functions used internally by delta functions 
/// </summary>
namespace impl
{
	inline delta::instruction create_copy_data_instruction(size_t start_index, size_t data_length, std::vector<char>::const_iterator data)
	{
		delta::instruction result;
		result.command = "COPY_DATA";
		result.start_index = start_index;
		result.data_length = data_length;

		result.data.resize(result.data_length);
		std::copy_n(data, result.data_length, result.data.begin());

		return std::move(result);
	}

	template <typename InputIter>
	void refill_input_buffer(
		InputIter& input,
		const size_t& input_length,
		std::vector<char>& input_buffer,
		size_t& input_buffer_index,
		size_t& bytes_read_into_input_buffer)
	{
		// move second half of the input_buffer up front
		std::copy(input_buffer.begin() + input_buffer_index, input_buffer.end(), input_buffer.begin());
		input_buffer.resize(input_buffer.size() - input_buffer_index);

		// fill up rest of the input_buffer
		const size_t length = std::min<size_t>(input_buffer_index, std::max<int>(0, static_cast<int>(input_length) - static_cast<int>(bytes_read_into_input_buffer)));
		for (size_t i = 0; i < length; ++i)
		{
			input_buffer.push_back(*input++);
		}
		bytes_read_into_input_buffer += length;
		input_buffer_index = 0;
	}
} // namespace impl
	
}; // namespace rd
