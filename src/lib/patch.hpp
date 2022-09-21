#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

#include "signature.hpp"
#include "delta.hpp"

namespace rd
{
/// <summary>
/// Applies delta to the original file to create updated file. 
/// </summary>
/// <typeparam name="OutIterator">Forward iterator that implement increment(++) and dereference(*) operators</typeparam>
/// <param name="original">Pointer to original data array</param>
/// <param name="del">Delta structure used for patching</param>
/// <param name="output">Iterator to the output data</param>
template <typename OutIterator>
void patch(const char* original, const delta& del, OutIterator output)
{
	for (const auto& instruction : del.instructions)
	{
		if (instruction.command == "COPY_DATA")
		{
			output = std::copy(instruction.data.cbegin(), instruction.data.cend(), output);
		}
		else if (instruction.command == "COPY_CHUNK")
		{
			output = std::copy_n(original + instruction.start_index, instruction.data_length, output);
		}
		else
		{
			throw std::invalid_argument("Unknown command in delta file: " + instruction.command);
		}
	}
};

/// <summary>
/// Applies delta to the original file to create updated file.
/// </summary>
/// <typeparam name="OutIterator">Forward iterator that implement increment(++) and dereference(*) operators</typeparam>
/// <param name="original">Input file stream of the original data</param>
/// <param name="del">Delta structure used for patching</param>
/// <param name="output">Iterator to the output data</param>
template <typename OutIterator>
void patch(std::ifstream& original, const delta& del, OutIterator output)
{
	for (const auto& instruction : del.instructions)
	{
		if (instruction.command == "COPY_DATA")
		{
			output = std::copy(instruction.data.cbegin(), instruction.data.cend(), output);
		}
		else if (instruction.command == "COPY_CHUNK")
		{
			original.seekg(instruction.start_index, original.beg);
			output = std::copy_n(std::istreambuf_iterator<char>(original), instruction.data_length, output);
		}
		else
		{
			throw std::invalid_argument("Unknown command in delta file: " + instruction.command);
		}
	}
};

}; // namespace rd
