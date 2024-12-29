#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace utils
{
	#define VECTOR_SUBTRACT(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
	#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
	#define fsel(c,x,y)				( (c) >= 0 ? (x) : (y) )

	float rad_to_deg(float radians);
	float deg_to_rad(float degrees);

	// Remap a value in the range [A,B] to [C,D].
	inline float remap_val(float val, float A, float B, float C, float D)
	{
		if (A == B)
			return fsel(val - B, D, C);
		return C + (D - C) * (val - A) / (B - A);
	}

	int try_stoi(const std::string& str, const int& default_return_val = 0);
	float try_stof(const std::string& str, const float& default_return_val = 0.0f);
	std::string split_string_between_delims(const std::string& str, const char delim_start, const char delim_end);

	static std::vector<std::string> split(const std::string& raw_input, const std::vector<char>& delims)
	{
		std::vector<std::string> strings;

		auto find_first_delim = [](const std::string_view& input, const std::vector<char>& delims) -> std::pair<char, std::size_t>
			{
				auto first_delim = 0;
				auto first_delim_index = static_cast<std::size_t>(-1);
				auto index = 0u;

				for (auto& delim : delims)
				{
					if ((index = input.find(delim)) != std::string::npos)
					{
						if (first_delim_index == -1 || index < first_delim_index)
						{
							first_delim = delim;
							first_delim_index = index;
						}
					}
				}

				return { (char)first_delim, first_delim_index };
			};

		std::string input = raw_input;

		while (!input.empty())
		{
			const auto split_delim = find_first_delim(input, delims);
			if (split_delim.first != 0)
			{
				strings.push_back(input.substr(0, split_delim.second));
				input = input.substr(split_delim.second + 1);
			}
			else
			{
				break;
			}
		}

		strings.push_back(input);
		return strings;
	}

	static std::vector<std::string> split(std::string& str, char delimiter)
	{
		return split(str, std::vector < char >({ delimiter }));
	}

	static std::vector<std::string> split(const std::string& str, char delimiter)
	{
		return split(str, std::vector < char >({ delimiter }));
	}

	bool starts_with(std::string_view haystack, std::string_view needle);
	bool string_contains(const std::string_view& s1, const std::string_view s2);
	void replace_all(std::string& source, const std::string_view& from, const std::string_view& to);
	bool erase_substring(std::string& base, const std::string& replace);

	inline void to_lower(std::string& str) {
		std::ranges::transform(str.begin(), str.end(), str.begin(), [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
	}

	std::string str_to_lower(std::string input);
	std::string convert_wstring(const std::wstring& wstr);
	std::string& ltrim(std::string& s);
	std::string& rtrim(std::string& s);
	std::string& trim(std::string& s);
	bool has_matching_symbols(const std::string& str, char opening_symbol = '(', char closing_symbol = ')', bool single_only = false);
	const char* va(const char* fmt, ...);
	void extract_integer_words(const std::string_view& str, std::vector<int>& integers, bool check_for_duplicates);

	void row_major_to_column_major(const float* row_major, float* column_major);
	bool float_equal(float a, float b, float eps = 1.e-6f);
	float finterp_to(const float current, const float target, const float delta_time, const float interpolation_speed);

	bool open_file_homepath(const std::string& sub_dir, const std::string& file_name, std::ifstream& file);
	std::uint64_t string_hash64(const std::string_view& str);
	std::uint32_t string_hash32(const std::string_view& str);

	uint32_t hash32_combine(uint32_t seed, const char* str);
	uint32_t hash32_combine(uint32_t seed, int val);
	uint32_t hash32_combine(uint32_t seed, float val);

	bool shader_dump_vs(IDirect3DVertexShader9* vs, const std::string& file_path, const std::string& file_name);
	bool shader_dump_ps(IDirect3DPixelShader9* ps, const std::string& file_path, const std::string& file_name);

	uint32_t pack_2f_in_dword(float f1, float f2);

	class benchmark
	{
	public:
		benchmark() { start(); }
		~benchmark() {
#if defined(BENCHMARK)
			now();
#endif
		}

		// returns true if measured ms > arg 1
		bool now([[maybe_unused]] float* in_out_largest_ms = nullptr)
		{
#if defined(BENCHMARK)
			bool result = false;

			const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
			const auto end_time = std::chrono::high_resolution_clock::now();

			const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();
			const auto ms = static_cast<float>((end - start)) * 0.001f;

			if (in_out_largest_ms)
			{
				if (ms > *in_out_largest_ms) 
				{
					*in_out_largest_ms = ms;
					result = true;
				}
			}

			/*
			const auto last_end = std::chrono::time_point_cast<std::chrono::microseconds>(m_last).time_since_epoch().count();
			const auto ms_diff = static_cast<float>(end - last_end) * 0.001f;

			const char* op_str = sub_operation_str ? sub_operation_str : m_operation_str ? m_operation_str : nullptr;
			if (op_str)
			{
				game::printf_to_console(">> [ %.3f ms ]\t[ ~ %.3f ms ]\tfor operation [ %s ]\n", ms, ms_diff, op_str);
			}
			else
			{
				game::printf_to_console(">> [ %.3f ms ]\t[ ~ %.3f ms ]\tbenchmark end\n", ms, ms_diff);
			}*/

			m_last_ms = ms;
			m_last = end_time;
			return result;
#else
			return false;
#endif
		}

		// returns the last measurement of now()
		float get_ms() { return m_last_ms; }

	private:

		void start()
		{
#if defined(BENCHMARK)
			m_start = std::chrono::high_resolution_clock::now();
			m_last = m_start;
#endif
		}

		//const char* m_operation_str = nullptr;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_last;
		float m_last_ms;
	};
}
