#pragma once

namespace utils
{
	inline bool g_external_console_created = false;
	inline HANDLE g_console_handle = nullptr;

	inline void console_write(std::string_view text)
	{
		if (!g_console_handle) {
			return;
		}
			

		DWORD written = 0;
		WriteConsoleA(g_console_handle, text.data(), text.size(), &written, nullptr);
	}

	class ConsoleStreamBuf : public std::streambuf
	{
	protected:
		std::string buffer;

		int overflow(int ch) override
		{
			if (ch != EOF) {
				buffer.push_back(static_cast<char>(ch));
			}

			flush();
			return ch;
		}

		int sync() override
		{
			flush();
			return 0;
		}

		void flush()
		{
			if (!buffer.empty())
			{
				console_write(buffer);
				buffer.clear();
			}
		}

	public:
		~ConsoleStreamBuf() override {
			flush();
		}
	};

	inline ConsoleStreamBuf console_buf;
	inline std::ostream console_out(&console_buf);

    inline void console()
    {
        if (!g_external_console_created)
        {
			g_external_console_created = true;
			AllocConsole();

			// create our own screen buffer because we dont want to use STD_OUTPUT_HANDLE
			g_console_handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
			if (g_console_handle == INVALID_HANDLE_VALUE) {
				return;
			}

			SetConsoleActiveScreenBuffer(g_console_handle);

			CONSOLE_SCREEN_BUFFER_INFO info;
			GetConsoleScreenBufferInfo(g_console_handle, &info);

			const SHORT new_width = 500;
			const SHORT new_height = std::max((SHORT)(info.srWindow.Bottom + 1), (SHORT)300);

			// shrink window temporarily to avoid SetConsoleScreenBufferSize failure
			SMALL_RECT rect = { 0, 0, 1, 1 };
			SetConsoleWindowInfo(g_console_handle, TRUE, &rect);

			// apply buffer size
			COORD new_size = { new_width, new_height };
			SetConsoleScreenBufferSize(g_console_handle, new_size);

			// resize visible window
			rect = { 0, 0, (SHORT)(120 - 1), (SHORT)(40 - 1) };
			SetConsoleWindowInfo(g_console_handle, TRUE, &rect);
        }
    }

	inline void set_console_color_red(bool highlight = false)
	{
		if (g_external_console_created) 
		{
			WORD color = FOREGROUND_RED;
			if (highlight) {
				color |= FOREGROUND_INTENSITY;
			}
			SetConsoleTextAttribute(g_console_handle, color);
		}
	}

	inline void set_console_color_green(bool highlight = false)
	{
		if (g_external_console_created) 
		{
			WORD color = FOREGROUND_GREEN;
			if (highlight) {
				color |= FOREGROUND_INTENSITY;
			}
			SetConsoleTextAttribute(g_console_handle, color);
		}
	}

	inline void set_console_color_blue(bool highlight = false)
	{
		if (g_external_console_created) 
		{
			WORD color = FOREGROUND_BLUE;
			if (highlight) {
				color |= FOREGROUND_INTENSITY;
			}
			SetConsoleTextAttribute(g_console_handle, color);
		}
	}

	inline void set_console_color_yellow(bool highlight = false)
	{
		if (g_external_console_created)
		{
			WORD color = FOREGROUND_RED | FOREGROUND_GREEN;
			if (highlight) {
				color |= FOREGROUND_INTENSITY;
			}
			SetConsoleTextAttribute(g_console_handle, color);
		}
	}

	inline void set_console_color_default(bool highlight = false)
	{
		if (g_external_console_created)
		{
			WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
			if (highlight) {
				color |= FOREGROUND_INTENSITY;
			}
			SetConsoleTextAttribute(g_console_handle, color);
		}
	}

	enum class LOG_TYPE
	{
		LOG_TYPE_DEFAULT,
		LOG_TYPE_STATUS,
		LOG_TYPE_GREEN,
		LOG_TYPE_WARN,
		LOG_TYPE_ERROR,
    };

	inline const char* log_type_to_string(LOG_TYPE type)
	{
		switch (type)
		{
		case LOG_TYPE::LOG_TYPE_DEFAULT: return "INFO";
		case LOG_TYPE::LOG_TYPE_STATUS:  return "STATUS";
		case LOG_TYPE::LOG_TYPE_GREEN:   return "OK";
		case LOG_TYPE::LOG_TYPE_WARN:    return "WARN";
		case LOG_TYPE::LOG_TYPE_ERROR:   return "ERROR";
		default:                         return "UNKNOWN";
		}
	}
	
	inline std::mutex log_mutex;
	inline std::ofstream log_file;
	inline std::once_flag log_file_init_flag;

	inline void init_log_file()
	{
		std::call_once(log_file_init_flag, []()
			{
				const std::string file_path = glob::root_path + "\\rtx_comp\\logfile.txt";
				log_file.open(file_path, std::ios::out | std::ios::trunc);
			});
	}

	inline void log(const std::string_view& module_str, const std::string_view& msg, LOG_TYPE type = LOG_TYPE::LOG_TYPE_DEFAULT, bool highlight = false, bool newline_infront = false, bool no_newline_at_end = false)
	{
		std::lock_guard<std::mutex> lock(log_mutex);

		// width of the inner module field
		constexpr int inner_width = 14;

		auto colorize = [](const LOG_TYPE& t, const bool h)
			{
				switch (t)
				{
				case LOG_TYPE::LOG_TYPE_DEFAULT:
					set_console_color_default(h);
					break;
				case LOG_TYPE::LOG_TYPE_STATUS:
					set_console_color_blue(h);
					break;
				case LOG_TYPE::LOG_TYPE_GREEN:
					set_console_color_green(h);
					break;
				case LOG_TYPE::LOG_TYPE_WARN:
					set_console_color_yellow(h);
					break;
				case LOG_TYPE::LOG_TYPE_ERROR:
					set_console_color_red(h);
					break;
				default:
					break;
				}
			};

		auto print_prefix = [&] 
			{
				console_out << std::setw(2) << (type == LOG_TYPE::LOG_TYPE_ERROR ? "!" : " ") << "[ ";

				colorize(type, true);
				console_out << std::format("{:>{}}", module_str, inner_width);
				set_console_color_default();

				console_out << " ]  ";
				colorize(type, highlight);
			};

		if (newline_infront) {
			console_out << '\n';
		}

		std::string_view remaining = msg;

		while (!remaining.empty() && remaining != "\n")
		{
			print_prefix();

			const size_t newline = remaining.find('\n');
			if (newline == std::string_view::npos)
			{
				console_out << remaining;
				break;
			}

			console_out << remaining.substr(0, newline) << '\n';
			remaining.remove_prefix(newline + 1);
		}

		if (!no_newline_at_end && (msg.empty() || msg.back() != '\n')) {
			console_out << '\n';
		}

		set_console_color_default();

		init_log_file();
		if (log_file.is_open())
		{
			log_file
				<< "[" << log_type_to_string(type) << "] "
				<< "[" << module_str << "] "
				<< msg << std::endl; // auto flush
		}
	}
}
