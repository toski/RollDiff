#include "delta.hpp"
#include "hash.hpp"



namespace rd
{

std::ostream& operator<<(std::ostream& os, const delta& del)
{
	os << del.data_length;
	os << del.instructions.size();

	for (const auto& i : del.instructions)
	{
		os << i.command.c_str() << i.start_index << i.chunk_id << i.data_length;
		std::copy(i.data.cbegin(), i.data.cend(), std::ostream_iterator<char>(os));
	}

	return os;
}

std::istream& operator>>(std::istream& is, delta& del)
{
	is >> del.data_length;

	size_t num_instructions{ 0 };
	is >> num_instructions;
	del.instructions.reserve(num_instructions);

	for (size_t i = 0; i < num_instructions; ++i)
	{
		delta::instruction new_instruction;
		is >> new_instruction.command;
		is >> new_instruction.start_index;
		is >> new_instruction.chunk_id;
		is >> new_instruction.data_length;

		new_instruction.data.reserve(new_instruction.data_length);
		std::copy_n(std::istream_iterator<char>(is), new_instruction.data_length, std::back_inserter(new_instruction.data));
	}

	return is;
}

std::ostream& delta::write_to_binary_file(std::ostream& os, const delta& del)
{
	os.write(reinterpret_cast<const char*>(&del.data_length), sizeof(del.data_length));
	size_t num_instructions = del.instructions.size();
	os.write(reinterpret_cast<const char*>(&num_instructions), sizeof(num_instructions));
	
	for (const auto& i : del.instructions)
	{
		char command = i.command == "COPY_DATA" ? 0 : 1;
		os.write(&command, sizeof(char));

		os.write(reinterpret_cast<const char*>(&i.start_index), sizeof(i.start_index));
		os.write(reinterpret_cast<const char*>(&i.chunk_id), sizeof(i.chunk_id));
		
		os.write(reinterpret_cast<const char*>(&i.data_length), sizeof(i.data_length));
		os.write(i.data.data(), sizeof(char) * (i.command == "COPY_DATA" ? i.data_length : 0));
	}

	return os;
}
std::istream& delta::read_from_binary_file(std::istream& is, delta& del)
{
	is.read(reinterpret_cast<char*>(&del.data_length), sizeof(del.data_length));
	size_t num_instructions = 0;
	is.read(reinterpret_cast<char*>(&num_instructions), sizeof(num_instructions));
	del.instructions.reserve(num_instructions);
	
	for (size_t i = 0; i < num_instructions; ++i)
	{
		delta::instruction new_instruction;

		char command = '\0';
		is.read(&command, sizeof(char));
		new_instruction.command = command == 0 ? "COPY_DATA" : "COPY_CHUNK";

		is.read(reinterpret_cast<char*>(&new_instruction.start_index), sizeof(new_instruction.start_index));
		is.read(reinterpret_cast<char*>(&new_instruction.chunk_id), sizeof(new_instruction.chunk_id));

		is.read(reinterpret_cast<char*>(&new_instruction.data_length), sizeof(new_instruction.data_length));
		new_instruction.data.resize(new_instruction.data_length);
		is.read(new_instruction.data.data(), sizeof(char) * (command == 0 ? new_instruction.data_length : 0));

		del.instructions.push_back(std::move(new_instruction));
	}

	return is;
}

}; // namespace rd
