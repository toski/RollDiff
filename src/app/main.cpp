#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <iterator>
#include <fstream>

#include "signature.hpp"
#include "delta.hpp"
#include "patch.hpp"


struct command_line_arguments
{
	std::string command{"help"};
	std::string first_file;
	std::string second_file;
	std::string third_file;
	size_t chunk_size{ 100 };
	bool print_progress{ false };
};

command_line_arguments show_usage(char* program_name)
{
	std::cerr << "Usage: " << program_name << " \n"
		<< "\tsignature old-file signature-file \n"
		<< "\tdelta signature-file new-file delta-file \n"
		<< "\tpatch old-file delta-file gen-file \n"
		<< "\t-h,--help\t\tShow this help message.\n"
		<< "\n"
		<< "Options:\n"
		<< "\t-c,--chunk\t\tSize of chunks in bytes. Default is 100.\n"
		<< "\t-v,--verbose\t\tShow progress."
		<< std::endl;

	return command_line_arguments{};
}

command_line_arguments process_arguments(int argc, char** argv)
{
	if (argc < 3)
	{
		return show_usage(argv[0]);
	}

	command_line_arguments result{};

	try
	{
		for (int i = 1; i < argc; ++i)
		{
			std::string arg = argv[i];

			if ((arg == "-h") || (arg == "--help"))
			{
				return show_usage(argv[0]);
			}
			else if ((arg == "-c") || (arg == "--chunk"))
			{
				if (i + 1 < argc)
				{
					result.chunk_size = std::stoul(argv[++i]);
				}
				else
				{
					return show_usage(argv[0]);
				}
			}
			else if (arg == "signature")
			{
				if (i + 2 < argc)
				{
					result.first_file = argv[++i];
					result.second_file = argv[++i];
					result.command = arg;
				}
				else
				{
					return show_usage(argv[0]);
				}
			}
			else if (arg == "delta")
			{
				if (i + 3 < argc)
				{
					result.first_file = argv[++i];
					result.second_file = argv[++i];
					result.third_file = argv[++i];
					result.command = arg;
				}
				else
				{
					return show_usage(argv[0]);
				}
			}
			else if (arg == "patch")
			{
				if (i + 3 < argc)
				{
					result.first_file = argv[++i];
					result.second_file = argv[++i];
					result.third_file = argv[++i];
					result.command = arg;
				}
				else
				{
					return show_usage(argv[0]);
				}
			}
			else if ((arg == "-v") || (arg == "--verbose"))
			{
				result.print_progress = true;
			}
			else
			{
				return show_usage(argv[0]);
			}
		}
	}
	catch (const std::exception&)
	{
		return show_usage(argv[0]);
	}

	return result;
}


void create_signature(const command_line_arguments& cla)
{
	try
	{
		// create signature
		std::ifstream old_file(cla.first_file, std::ios_base::binary);
		if (!old_file.is_open())
		{
			throw std::runtime_error("Unable to open old file!");
		}
		old_file.seekg(0, old_file.end);
		size_t old_file_size = old_file.tellg();
		old_file.seekg(0, old_file.beg);
		auto old_file_signature = rd::calculate_signature<std::istreambuf_iterator<char>>(
			std::istreambuf_iterator<char>(old_file), old_file_size, cla.chunk_size);

		// save signature to file
		std::ofstream signature_file(cla.second_file, std::ios_base::binary);
		rd::signature::write_to_binary_file(signature_file, old_file_signature);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error while creating signature for file '" << cla.first_file << "': " << e.what() << std::endl;
	}
}

void create_delta(const command_line_arguments& cla)
{
	try
	{
		// load signature
		std::ifstream signature_file(cla.first_file, std::ios_base::binary);
		if (!signature_file.is_open())
		{
			throw std::runtime_error("Unable to open signature file!");
		}
		rd::signature signature_;
		rd::signature::read_from_binary_file(signature_file, signature_);

		// load new file
		std::ifstream new_file(cla.second_file, std::ios_base::binary);
		if (!new_file.is_open())
		{
			throw std::runtime_error("Unable to open new file!");
		}
		new_file.seekg(0, new_file.end);
		size_t new_file_size = new_file.tellg();
		new_file.seekg(0, new_file.beg);

		// create delta and save it to file
		rd::delta delta_ = rd::calculate_delta<std::istreambuf_iterator<char>>(signature_, std::istreambuf_iterator<char>(new_file), new_file_size);
		std::ofstream delta_file(cla.third_file, std::ios_base::binary);
		rd::delta::write_to_binary_file(delta_file, delta_);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error while creating delta from signature file '" << cla.first_file 
			<< "' and new file '" << cla.second_file << "': " << e.what() << std::endl;
	}
}

void create_patch(const command_line_arguments& cla)
{
	try
	{
		// load old file
		std::ifstream old_file(cla.first_file, std::ios_base::binary);
		if (!old_file.is_open())
		{
			throw std::runtime_error("Unable to open old file!");
		}

		// load delta
		std::ifstream delta_file(cla.second_file, std::ios_base::binary);
		if (!old_file.is_open())
		{
			throw std::runtime_error("Unable to open delta file!");
		}
		rd::delta delta_;
		rd::delta::read_from_binary_file(delta_file, delta_);

		// patch old file and save it
		std::ofstream patch_file(cla.third_file, std::ios_base::binary);
		rd::patch<std::ostreambuf_iterator<char>>(
			old_file, delta_, std::ostreambuf_iterator<char>(patch_file));
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error while creating patch from old file '" << cla.first_file
			<< "' and delta '" << cla.second_file << "': " << e.what() << std::endl;
	}
}


int main(int argc, char** argv)
{
	auto cla = process_arguments(argc, argv);
	if (cla.command == "help")
	{
		return EXIT_SUCCESS;
	}

	if (cla.command == "signature")
	{
		assert(cla.first_file.length() > 0);
		assert(cla.second_file.length() > 0);

		create_signature(cla);
	}

	if (cla.command == "delta")
	{
		assert(cla.first_file.length() > 0);
		assert(cla.second_file.length() > 0);
		assert(cla.third_file.length() > 0);

		create_delta(cla);
	}

	if (cla.command == "patch")
	{
		assert(cla.first_file.length() > 0);
		assert(cla.second_file.length() > 0);
		assert(cla.third_file.length() > 0);

		create_patch(cla);
	}

	return EXIT_SUCCESS;
}
