#include <chrono>
#include <iostream>
#include <rtl-sdr.h>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono_literals;

constexpr unsigned char null_range[] = {0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d,
										0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
										0x84, 0x85, 0x86, 0x87, 0x88};
constexpr unsigned long null_range_size =
	sizeof(null_range) / sizeof(*null_range);

constexpr unsigned char pattern[] = {
	0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1,
	0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0,
	0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1,
	0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0,
	0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1,
	0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0,
	0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1,
	0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0,
	0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0};
constexpr unsigned long pattern_size = sizeof(pattern) / sizeof(*pattern);

bool is_null(unsigned char c)
{
	// noise
	for(unsigned long i = 0; i < null_range_size; ++i)
	{
		if(c == null_range[i]) return true;
	}
	return false;
}

bool expect_data(unsigned char* ptr, int start_index, int count_right)
{
	bool expect_data = false;
	for(auto i = start_index; i < count_right; ++i)
	{
		if(!is_null(ptr[i]))
			expect_data = true;
		else
			expect_data = false;
	}
	return expect_data;
}

bool test_pattern(const std::vector<unsigned char>& test)
{
	if(test.size() < pattern_size) return false;
	bool result = false;
	for(std::size_t i = 0; i < pattern_size; ++i)
	{
		if(test[i] == pattern[i])
			result = true;
		else
			result = false;
	}
	return result;
}

int main()
{
	const uint32_t dev_index = 0;
	if(rtlsdr_get_device_count() == 1)
		std::cout << "Connected device name: "
				  << rtlsdr_get_device_name(dev_index) << endl;
	else
	{
		std::cout << "No device connected or too many. Exiting." << std::endl;
		return -1;
	}

	rtlsdr_dev_t* dev = nullptr;
	rtlsdr_open(&dev, dev_index);
	rtlsdr_set_center_freq(dev, 434000000);
	rtlsdr_set_sample_rate(dev, 2000000);

	const int len = 512;
	unsigned char buf[len];
	int nread;
	int space_counter = 0, data_counter = 0;
	std::vector<unsigned char> gen_pattern;
	bool reset_space_counter = false, reset_data_counter = false;
	std::this_thread::sleep_for(5000us);

	bool need_echo = false;
	while(true)
	{
		int average = 0;
		rtlsdr_reset_buffer(dev);
		rtlsdr_read_sync(dev, &buf, len, &nread);
		for(int i = 0; i < len; ++i)
		{
			average += int(buf[i]);

			// process data
			if(!expect_data(buf, i, 3))
			{
				if(reset_space_counter)
				{
					reset_space_counter = false;
					//					std::cout << "Space: "
					//<< space_counter
					//<< std::endl;
					space_counter = 0;
					if((gen_pattern.size() <= pattern_size) &&
					   (!gen_pattern.empty()))
						gen_pattern.push_back(0x0);
				}

				space_counter++;
				reset_data_counter = true;
			}
			else
			{
				if(reset_data_counter)
				{
					reset_data_counter = false;
					//					std::cout << "Data: " <<
					// data_counter
					//<< std::endl;
					data_counter = 0;
					if(gen_pattern.size() <= pattern_size)
						gen_pattern.push_back(0x1);
				}

				data_counter++;
				reset_space_counter = true;
			}
		}

		if(test_pattern(gen_pattern))
		{
			std::cout << pattern_size << std::endl;
			for(std::size_t i = 0; i < pattern_size; ++i)
				std::cout << int(gen_pattern[i]) << " ";
			break;
		}
		if(nread == len && average > 66000) { need_echo = true; }

		if(need_echo) {}

		std::cout.flush();
	}

	rtlsdr_close(dev);
}
