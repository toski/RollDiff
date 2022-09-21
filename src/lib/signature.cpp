#include "signature.hpp"

namespace rd
{

bool operator==(const chunk& left, const chunk& right)
{
	return left.start_position == right.start_position && left.length == right.length && left.hash == right.hash;
}

bool operator!=(const chunk& left, const chunk& right)
{
	return !(left == right);
}

std::ostream& operator<<(std::ostream& os, const chunk& ch)
{
	os << "(" << ch.start_position << " " << ch.length << " " << ch.hash << ")";

	return os;
}

bool operator==(const signature& left, const signature& right) 
{ 
	if (left.chunks.size() != right.chunks.size())
	{
		return false;
	}

	for (size_t i = 0; i < left.chunks.size(); ++i)
	{
		if (left.chunks[i] != right.chunks[i])
		{
			return false;
		}
	}

	return true;
}

bool operator!=(const signature& left, const signature& right)
{
	return !(left == right);
}

std::ostream& signature::write_to_binary_file(std::ostream& os, const signature& sig)
{
	size_t num_chunks = sig.chunks.size();
	os.write(reinterpret_cast<const char*>(&num_chunks), sizeof(num_chunks));

	for (const auto& ch : sig.chunks)
	{
		os.write(reinterpret_cast<const char*>(&ch.start_position), sizeof(ch.start_position));
		os.write(reinterpret_cast<const char*>(&ch.length), sizeof(ch.length));
		os.write(reinterpret_cast<const char*>(&ch.hash), sizeof(ch.hash));
	}

	return os;
}
std::istream& signature::read_from_binary_file(std::istream& is, signature& sig)
{
	size_t num_chunks = 0;
	is.read(reinterpret_cast<char*>(&num_chunks), sizeof(num_chunks));
	sig.chunks.reserve(num_chunks);

	for (size_t i = 0; i < num_chunks; ++i)
	{
		chunk new_chunk;
		
		is.read(reinterpret_cast<char*>(&new_chunk.start_position), sizeof(new_chunk.start_position));
		is.read(reinterpret_cast<char*>(&new_chunk.length), sizeof(new_chunk.length));
		is.read(reinterpret_cast<char*>(&new_chunk.hash), sizeof(new_chunk.hash));

		sig.chunks.push_back(new_chunk);
	}

	return is;
}
	
}; // namespace rd
