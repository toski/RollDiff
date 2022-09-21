#include <gtest/gtest.h>
#include "checksum.hpp"
#include "hash.hpp"
#include <string>


TEST(test_hash, checksum_adler32)
{
	std::string data{ "The quick brown fox jumps over the lazy dog" };
	EXPECT_EQ(rd::compute_checksum(data.begin(), data.length()), 0x5bdc0fda);

	data = "Wikipedia";
	EXPECT_EQ(rd::compute_checksum(data.begin(), data.length()), 0x11E60398);

	data = "Adler-32 is a checksum algorithm written by Mark Adler in 1995, modifying Fletcher's checksum. Compared to a cyclic redundancy check of the same length, it trades reliability for speed (preferring the latter). Adler-32 is more reliable than Fletcher-16, and slightly less reliable than Fletcher-32.";
	EXPECT_EQ(rd::compute_checksum(data.begin(), data.length()), 0x396f68d6);
}

TEST(test_hash, jenkins_hash_function)
{
	std::string data{ "The quick brown fox jumps over the lazy dog" };
	EXPECT_EQ(rd::compute_hash(data.begin(), data.length()), 0x519e91f5);

	data = "Wikipedia";
	EXPECT_EQ(rd::compute_hash(data.begin(), data.length()), 0x2eb8e7cd);

	data = "Jenkins's one_at_a_time hash was originally created to fulfill certain requirements described by Colin Plumb, a cryptographer, but was ultimately not put to use.";
	EXPECT_EQ(rd::compute_hash(data.begin(), data.length()), 0xd20c13be);
}
