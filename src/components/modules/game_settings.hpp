#pragma once

namespace components
{
	class game_settings : public component
	{
	public:
		game_settings();
		~game_settings() = default;

		static inline game_settings* p_this = nullptr;
		static auto get() { return &vars; }

		static void write_toml();
		static bool parse_toml();

		static void xo_gamesettings_update_fn();

	private:
		union var_value
		{
			bool boolean;
			int integer;
			float value;
		};

		enum var_type : std::uint8_t
		{
			var_type_boolean = 0,
			var_type_integer = 1,
			var_type_value = 2,
		};

		class variable
		{
		public:
			variable(const char* name, const char* desc, const bool boolean) :
				m_name(name), m_desc(desc), m_type(var_type_boolean)
			{
				m_var.boolean = boolean;
				m_var_default.boolean = boolean;
			}

			variable(const char* name, const char* desc, const int integer) :
				m_name(name), m_desc(desc), m_type(var_type_integer)
			{
				m_var.integer = integer;
				m_var_default.integer = integer;
			}

			variable(const char* name, const char* desc, const float value) :
				m_name(name), m_desc(desc), m_type(var_type_value)
			{
				m_var.value = value;
				m_var_default.value = value;
			}

			const char* get_str_value(bool get_default = false) const
			{
				switch (m_type)
				{
				case var_type_boolean:
					return utils::va("%s", (!get_default ? m_var.boolean : m_var_default.boolean) ? "true" : "false");

				case var_type_integer:
					return utils::va("%d", !get_default ? m_var.integer : m_var_default.integer);

				case var_type_value:
					return utils::va("%.2f", !get_default ? m_var.value : m_var_default.value);
				}

				return nullptr;
			}

			const char* get_str_type() const
			{
				switch (m_type)
				{
				case var_type_boolean:
					return "BOOL";

				case var_type_integer:
					return "INT";

				case var_type_value:
					return "FLOAT";
				}

				return nullptr;
			}

			std::string get_tooltip_string() const
			{
				std::string out;
				out += "# " + std::string(this->m_desc) + "\n";
				out += "# Type: " + std::string(this->get_str_type()) + " || Default: " + std::string(this->get_str_value(true));
				return out;
			}

			template <typename T>
			T get_as(bool default_val = false)
			{
				if (m_type == var_type_boolean) {
					return static_cast<T>(!default_val ? m_var.boolean : m_var_default.boolean);
				}

				if (m_type == var_type_integer) {
					return static_cast<T>(!default_val ? m_var.integer : m_var_default.integer);
				}

				if (m_type == var_type_value) {
					return static_cast<T>(!default_val ? m_var.value : m_var_default.value);
				}

				throw std::runtime_error("Unknown var_type");
			}

			template <typename T>
			T* get_ptr_as()
			{
				if (m_type == var_type_boolean) {
					return reinterpret_cast<T*>(&m_var.boolean);
				}

				if (m_type == var_type_integer) {
					return reinterpret_cast<T*>(&m_var.integer);
				}

				if (m_type == var_type_value) {
					return reinterpret_cast<T*>(&m_var.value);
				}

				throw std::runtime_error("Unknown var_type");
			}

			var_type get_type() const {
				return m_type;
			}

			// sets var and writes toml (bool)
			void set_var(const bool boolean, bool no_toml_update = false)
			{
				m_var.boolean = boolean;
				if (!no_toml_update) {
					write_toml();
				}
			}

			// sets var and writes toml (integer)
			void set_var(const int integer, bool no_toml_update = false)
			{
				m_var.integer = integer;
				if (!no_toml_update) {
					write_toml();
				}
			}

			// sets var and writes toml (float)
			void set_var(const float value, bool no_toml_update = false)
			{
				m_var.value = value;
				if (!no_toml_update) {
					write_toml();
				}
			}

			const char* m_name;
			const char* m_desc;

		private:
			var_value m_var;
			var_value m_var_default;
			var_type m_type;
		};

		// note:
		// cba. to impl. automatic detection of newlines in comments -> add '# ' manually :>

		struct var_definitions
		{
			variable lod_forcing =
			{
				"lod_forcing",
				"The mod normally forces LOD0 for everything. Setting this to false disables that.",
				true
			};

			variable enable_3d_sky =
			{
				"enable_3d_sky",
				"Enable tweaks required for the 3D skybox. Requires proper 3D skybox remix-runtime settings (sky auto detect). Can/will crash the game when its getting unfocused.",
				false
			};

			variable default_nocull_distance =
			{
				"default_nocull_distance",
				("The default distance (radius around player) where nothing will get culled.\n"
				 "# Value is only used by certain anti-culling modes & if there isn't a manual area/leaf override via a MapSettings entry."),
				600.0f
			};
		};

		static inline var_definitions vars = {};
	};
}
