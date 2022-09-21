#include <gtest/gtest.h>

#include "hash.hpp"
#include "signature.hpp"
#include "delta.hpp"
#include "patch.hpp"
#include "test_data.h"

#include <string>
#include <ostream>
#include <iterator>
#include <fstream>

TEST(test_hash_roll, chunk_modify)
{
	test_data_small data_before;
	test_data_small data_after;

	char* data1 = data_before.data;
	char* data2 = data_after.data;
	auto hash_before = rd::compute_hash(data1, data_before.chunk_length);
	auto hash_after = rd::compute_hash(data2, data_after.chunk_length);
	EXPECT_EQ(hash_before, hash_after);

	auto signature_before = rd::calculate_signature(data_before.data, data_before.data_length, data_before.chunk_length);
	auto signature_after  = rd::calculate_signature(data_after.data, data_after.data_length, data_after.chunk_length);
	EXPECT_EQ(signature_before, signature_after);

	data_after.data[0] = 'a';
	data2 = data_after.data;
	hash_after = rd::compute_hash(data2, data_after.chunk_length);
	EXPECT_NE(hash_before, hash_after);
	signature_after = rd::calculate_signature(data_after.data, data_after.data_length, data_after.chunk_length);
	EXPECT_NE(signature_before, signature_after);
	data_after.data[0] = '1';
	signature_after = rd::calculate_signature(data_before.data, data_before.data_length - 100, data_before.chunk_length);
	EXPECT_NE(signature_before, signature_after);

	signature_after = rd::calculate_signature(data_before.data, data_before.data_length, data_before.chunk_length - 10);
	EXPECT_NE(signature_before, signature_after);
	EXPECT_EQ(signature_after.chunks[0].length, data_before.chunk_length - 10);
	EXPECT_EQ(signature_after.chunks[7].length, 70);
}


TEST(test_hash_roll, multi_change_test)
{
	test_data_small original_data;
	auto original_signature = rd::calculate_signature<char*>(original_data.data, original_data.data_length - 50, original_data.chunk_length);

	EXPECT_EQ(original_signature.chunks.size(), 7);
	EXPECT_EQ(original_signature.chunks[0].length, original_data.chunk_length);
	EXPECT_EQ(original_signature.chunks[6].length, 50);
	rd::signature expected_signature{ { {0, 100, 0x31477121}, {100, 100, 0x6c03a0c3},  {200, 100, 0x1C42219B}, {300, 100, 0xB408C42C},
		{400, 100, 0x4b699624}, {500, 100, 0x203bff41}, {600, 50, 0xe81b3115} } };
	EXPECT_EQ(original_signature, expected_signature);

	test_data_small_multichange multi_change_data;
	rd::delta delta_multi = rd::calculate_delta<char*>(original_signature, multi_change_data.data, multi_change_data.data_length);
	EXPECT_TRUE(delta_multi.instructions.size() == 10);
	EXPECT_EQ(delta_multi.data_length, 665);

	// we cant match first 100 bytes to anything so we copy them to delta 
	EXPECT_EQ(delta_multi.instructions[0].command, "COPY_DATA");
	EXPECT_EQ(delta_multi.instructions[0].start_index, 0);
	EXPECT_EQ(delta_multi.instructions[0].data_length, 100);
	EXPECT_EQ(strncmp(multi_change_data.data, delta_multi.instructions[0].data.data(), 100), 0);

	// then we match a chunk from index 105 to 205 so we need to copy next 5 bytes of input to the delta
	EXPECT_EQ(delta_multi.instructions[1].command, "COPY_DATA");
	EXPECT_EQ(delta_multi.instructions[1].start_index, 100);
	EXPECT_EQ(delta_multi.instructions[1].data_length, 5);
	EXPECT_EQ(strncmp(multi_change_data.data + 100, delta_multi.instructions[1].data.data(), 5), 0);

	// then we copy matched chunk which is original chunk 1
	EXPECT_EQ(delta_multi.instructions[2].command, "COPY_CHUNK");
	EXPECT_EQ(delta_multi.instructions[2].start_index, 100);
	EXPECT_EQ(delta_multi.instructions[2].data_length, original_data.chunk_length);
	EXPECT_EQ(delta_multi.instructions[2].chunk_id, 1);

	// next we match original chunk 3 
	EXPECT_EQ(delta_multi.instructions[3].command, "COPY_CHUNK");
	EXPECT_EQ(delta_multi.instructions[3].start_index, 300);
	EXPECT_EQ(delta_multi.instructions[3].data_length, original_data.chunk_length);
	EXPECT_EQ(delta_multi.instructions[3].chunk_id, 3);

	// then original chunk 4
	EXPECT_EQ(delta_multi.instructions[4].command, "COPY_CHUNK");
	EXPECT_EQ(delta_multi.instructions[4].start_index, 400);
	EXPECT_EQ(delta_multi.instructions[4].data_length, original_data.chunk_length);
	EXPECT_EQ(delta_multi.instructions[4].chunk_id, 4);

	// we match original chunk 6 after 5 bytes so we need to copy those 5 bytes to the delta
	EXPECT_EQ(delta_multi.instructions[5].command, "COPY_DATA");
	EXPECT_EQ(delta_multi.instructions[5].start_index, 405);
	EXPECT_EQ(delta_multi.instructions[5].data_length, 5);
	EXPECT_EQ(strncmp(multi_change_data.data + 405, delta_multi.instructions[5].data.data(), 5), 0);

	// then copy original chunk 6 which is only 50 bytes long
	EXPECT_EQ(delta_multi.instructions[6].command, "COPY_CHUNK");
	EXPECT_EQ(delta_multi.instructions[6].start_index, 600);
	EXPECT_EQ(delta_multi.instructions[6].data_length, original_data.chunk_length / 2);
	EXPECT_EQ(delta_multi.instructions[6].chunk_id, 6);

	// we then match original chunk 2
	EXPECT_EQ(delta_multi.instructions[7].command, "COPY_CHUNK");
	EXPECT_EQ(delta_multi.instructions[7].start_index, 200);
	EXPECT_EQ(delta_multi.instructions[7].data_length, original_data.chunk_length);
	EXPECT_EQ(delta_multi.instructions[7].chunk_id, 2);

	// we match original chunk 5
	EXPECT_EQ(delta_multi.instructions[8].command, "COPY_CHUNK");
	EXPECT_EQ(delta_multi.instructions[8].start_index, 500);
	EXPECT_EQ(delta_multi.instructions[8].data_length, original_data.chunk_length);
	EXPECT_EQ(delta_multi.instructions[8].chunk_id, 5);

	// 5 bytes are unmatched so we copy them
	EXPECT_EQ(delta_multi.instructions[9].command, "COPY_DATA");
	EXPECT_EQ(delta_multi.instructions[9].start_index, 660);
	EXPECT_EQ(delta_multi.instructions[9].data_length, 5);
	EXPECT_EQ(strncmp(multi_change_data.data + 660, delta_multi.instructions[9].data.data(), 5), 0);
}


TEST(test_hash_roll, binary_file_test)
{
	constexpr size_t chunk_size = 5000;
	const std::string old_file_name = "data/old.bmp";
	const std::string signature_file_name = "data/signature.bin";
	const std::string new_file_name = "data/new.bmp";
	const std::string delta_file_name = "data/delta.bin";
	const std::string patched_file_name = "data/patched.bmp";
	
	// create signature file
	{
		std::ifstream old_file(old_file_name, std::ios_base::binary);
		EXPECT_TRUE(old_file.is_open());
		old_file.seekg(0, old_file.end);
		size_t old_file_size = old_file.tellg();
		old_file.seekg(0, old_file.beg);
		auto old_file_signature = rd::calculate_signature<std::istreambuf_iterator<char>>(
			std::istreambuf_iterator<char>(old_file), old_file_size, chunk_size);

		// save signature to file
		std::ofstream signature_file(signature_file_name, std::ios_base::binary);
		EXPECT_TRUE(signature_file.is_open());
		rd::signature::write_to_binary_file(signature_file, old_file_signature);
	}

	// create delta file
	{
		// load signature
		std::ifstream signature_file(signature_file_name, std::ios_base::binary);
		EXPECT_TRUE(signature_file.is_open());
		rd::signature signature_;
		rd::signature::read_from_binary_file(signature_file, signature_);

		// load new file
		std::ifstream new_file(new_file_name, std::ios_base::binary);
		EXPECT_TRUE(new_file.is_open());
		new_file.seekg(0, new_file.end);
		size_t new_file_size = new_file.tellg();
		new_file.seekg(0, new_file.beg);

		// create delta and save it to file
		rd::delta delta_ = rd::calculate_delta<std::istreambuf_iterator<char>>(
			signature_, std::istreambuf_iterator<char>(new_file), new_file_size);
		std::ofstream delta_file(delta_file_name, std::ios_base::binary);
		EXPECT_TRUE(delta_file.is_open());
		rd::delta::write_to_binary_file(delta_file, delta_);
	}

	// create the patch file
	{
		// load old file
		std::ifstream old_file(old_file_name, std::ios_base::binary);
		EXPECT_TRUE(old_file.is_open());

		// load delta
		std::ifstream delta_file(delta_file_name, std::ios_base::binary);
		EXPECT_TRUE(delta_file.is_open());
		rd::delta delta_;
		rd::delta::read_from_binary_file(delta_file, delta_);

		// patch old file and save it
		std::ofstream patched_file(patched_file_name, std::ios_base::binary);
		EXPECT_TRUE(patched_file.is_open());
		rd::patch<std::ostreambuf_iterator<char>>(
			old_file, delta_, std::ostreambuf_iterator<char>(patched_file));
	}

	// compare old and patched files
	{
		// load old file
		std::ifstream new_file(new_file_name, std::ios_base::binary);
		EXPECT_TRUE(new_file.is_open());
		new_file.seekg(0, new_file.end);
		size_t new_file_size = new_file.tellg();
		new_file.seekg(0, new_file.beg);
		auto new_file_signature = rd::calculate_signature<std::istreambuf_iterator<char>>(
			std::istreambuf_iterator<char>(new_file), new_file_size, chunk_size);

		// load patched file
		std::ifstream patched_file(patched_file_name, std::ios_base::binary);
		EXPECT_TRUE(patched_file.is_open());
		patched_file.seekg(0, patched_file.end);
		size_t patched_file_size = patched_file.tellg();
		patched_file.seekg(0, patched_file.beg);
		auto patched_file_signature = rd::calculate_signature<std::istreambuf_iterator<char>>(
			std::istreambuf_iterator<char>(patched_file), patched_file_size, chunk_size);

		// sizes and signatures should be the same
		EXPECT_EQ(new_file_size, patched_file_size);
		for (size_t i = 0; i < new_file_signature.chunks.size(); ++i)
		{
			EXPECT_EQ(new_file_signature.chunks[i], patched_file_signature.chunks[i]);
		}
	}
}

TEST(test_hash_roll, binary_file_char_arrays)
{
	constexpr size_t chunk_size = 100;
	const std::string old_file_name = "data/old.bmp";
	const std::string signature_file_name = "data/signature.bin";
	const std::string new_file_name = "data/new.bmp";
	const std::string delta_file_name = "data/delta.bin";
	const std::string patched_file_name = "data/patched.bmp";

	// create signature file
	std::ifstream old_file(old_file_name, std::ios_base::binary);
	EXPECT_TRUE(old_file.is_open());
	old_file.seekg(0, old_file.end);
	size_t old_file_size = old_file.tellg();
	old_file.seekg(0, old_file.beg);
	std::vector<char> old_array;
	old_array.resize(old_file_size);
	EXPECT_TRUE(old_file.good());
	old_file.read(old_array.data(), old_file_size);
	EXPECT_TRUE(old_file.good());
	old_file.seekg(0, old_file.beg);

	auto old_file_signature = rd::calculate_signature<std::istreambuf_iterator<char>>(
		std::istreambuf_iterator<char>(old_file), old_file_size, chunk_size);
	auto old_array_signature = rd::calculate_signature<char*>(
		old_array.data(), old_file_size, chunk_size);
	EXPECT_EQ(old_file_signature, old_array_signature);

	// load new file
	std::ifstream new_file(new_file_name, std::ios_base::binary);
	EXPECT_TRUE(new_file.is_open());
	new_file.seekg(0, new_file.end);
	size_t new_file_size = new_file.tellg();
	new_file.seekg(0, new_file.beg);

	std::vector<char> new_array;
	new_array.resize(new_file_size);
	new_file.read(new_array.data(), new_file_size);
	new_file.seekg(0, new_file.beg);
	auto new_array_signature = rd::calculate_signature<char*>(
		new_array.data(), new_file_size, chunk_size);

	rd::delta delta_array = rd::calculate_delta<char*>(old_array_signature,	new_array.data(), new_file_size);
	EXPECT_EQ(delta_array.data_length, new_file_size);

	// memory patch
	std::vector<char> patch_array;
	patch_array.resize(delta_array.data_length);
	rd::patch<char*>(old_array.data(), delta_array, patch_array.data());
	auto patch_array_signature = rd::calculate_signature<char*>(
		patch_array.data(), delta_array.data_length, chunk_size);

	for (size_t i = 0; i < patch_array_signature.chunks.size(); ++i)
	{
		EXPECT_EQ(patch_array_signature.chunks[i], new_array_signature.chunks[i]);
	}

	std::ofstream patched_file(patched_file_name, std::ios_base::binary);
	patched_file.write(patch_array.data(), delta_array.data_length);
}

