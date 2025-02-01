#pragma once
#include "remix_vars.hpp"

namespace components
{
	class map_settings : public component
	{
	public:
		map_settings();
		~map_settings() = default;

		static inline map_settings* p_this = nullptr;
		static map_settings* get() { return p_this; }

		enum TRANSITION_MODE : uint8_t
		{
			ONCE_ON_ENTER = 0,
			ONCE_ON_LEAVE = 1,
			ALWAYS_ON_ENTER = 2,
			ALWAYS_ON_LEAVE = 3,
		};

		enum TRANSITION_TRIGGER_TYPE : uint8_t
		{
			CHOREO = 0,
			SOUND = 1,
			LEAF = 2,
		};

		struct remix_transition_s
		{
			TRANSITION_TRIGGER_TYPE trigger_type;

			// choreo trigger
			std::string choreo_name;
			std::string choreo_actor;
			std::string choreo_event;
			std::string choreo_param1;

			// sound trigger
			std::uint32_t sound_hash;
			std::string sound_name;

			// leaf trigger
			std::unordered_set<std::uint32_t> leafs;

			std::string config_name;
			TRANSITION_MODE mode;
			remix_vars::EASE_TYPE interpolate_type;
			float delay_in = 0.0f;
			float delay_out = 0.0f;
			float duration = 0.0f;
			std::uint64_t hash;
			bool _state_enter = false;
		};

		struct marker_settings_s
		{
			std::uint32_t index = 0;
			Vector origin = {};
			bool no_cull = false;
			Vector rotation = { 0.0f, 0.0f, 0.0f };
			Vector scale = { 1.0f, 1.0f, 1.0f }; // no_cull only
			std::unordered_set<std::uint32_t> areas; // no_cull only
			std::unordered_set<std::uint32_t> when_not_in_leafs; // no_cull only
			void* handle = nullptr;
			bool is_hidden = false;
			bool imgui_is_selected = false;
		};

		struct api_config_var
		{
			std::string variable;
			std::string value;
		};

		struct remix_light_settings_s
		{
			struct point_s
			{
				Vector position;
				Vector radiance;
				float radiance_scalar = 1.0f;
				float radius = 1.0f;
				float timepoint = 0.0f;
				float smoothness = 0.5f;

				// shaping
				bool use_shaping = false;
				Vector direction = { 0.0f, 0.0f, 1.0f };
				float degrees = 90.0; // cone angle
				float softness = 0.0f; // cone
				float exponent = 0.0f; // focus
			};

			std::vector<point_s> points;
			bool run_once = 0u;
			bool loop = false;
			bool loop_smoothing = false;
			bool trigger_always = false;

			std::string trigger_choreo_name;
			std::string trigger_choreo_actor;
			std::string trigger_choreo_event;
			std::string trigger_choreo_param1;
			std::uint32_t trigger_sound_hash;
			float trigger_delay = 0.0f;

			std::string kill_choreo_name;
			std::uint32_t kill_sound_hash;
			float kill_delay = 0.0f;
		};

		enum AREA_CULL_MODE : uint8_t
		{
			AREA_CULL_MODE_NO_FRUSTUM = 0,
			AREA_CULL_MODE_FRUSTUM = 1,
			AREA_CULL_MODE_FRUSTUM_FORCE_AREA = 2,
			AREA_CULL_COUNT = 3,
			// -------------------
			AREA_CULL_MODE_DEFAULT = AREA_CULL_MODE_FRUSTUM_FORCE_AREA,
		};

		struct leaf_tweak_s
		{
			std::unordered_set<std::uint32_t> in_leafs;
			std::unordered_set<std::uint32_t> areas;
		};

		struct hide_area_s
		{
			std::unordered_set<std::uint32_t> areas;
			std::unordered_set<std::uint32_t> when_not_in_leafs;
		};

		struct area_overrides_s
		{
			std::unordered_set<std::uint32_t> leafs;
			std::unordered_set<std::uint32_t> areas;
			std::unordered_set<std::uint32_t> hide_leafs;
			std::vector<hide_area_s> hide_areas;
			std::vector<leaf_tweak_s> leaf_tweaks;
			AREA_CULL_MODE cull_mode;
			std::uint32_t area_index;
		};

		struct hide_models_s
		{
			std::unordered_set<std::string> substrings;
			std::unordered_set<float> radii;
		};

		struct map_settings_s
		{
			std::string	mapname;
			float fog_dist = 0.0f;
			DWORD fog_color = 0xFFFFFFFF;
			float water_uv_scale = 1.0f;
			std::unordered_map<std::uint32_t, area_overrides_s> area_settings;
			hide_models_s hide_models;
			std::vector<remix_transition_s> remix_transitions;
			std::vector<marker_settings_s> map_markers;
			std::vector<std::string> api_var_configs;
			std::vector<remix_light_settings_s> remix_lights;
			bool using_any_light_sound_hash = false;
			bool using_any_transition_sound_hash = false;
			bool using_any_transition_sound_name = false;
		};

		static map_settings_s& get_map_settings() { return m_map_settings; }
		static const std::string& get_map_name() { return m_map_settings.mapname; }

		void set_settings_for_map(const std::string& map_name);
		static void spawn_markers_once();
		static void destroy_markers();
		static void on_map_load(const std::string& map_name);
		static void on_map_unload();
		static void clear_map_settings();
		static void reload();

		struct level_bool_s
		{
			bool c1m1_hotel = false, c1m2_streets = false, c1m3_mall = false, c1m4_atrium = false,
				 c2m1_highway = false, c2m2_fairgrounds = false, c2m3_coaster = false, c2m4_barns = false, c2m5_concert = false,
				 c3m1_plankcountry = false, c3m2_swamp = false, c3m3_shantytown = false, c3m4_plantation = false,
				 c4m1_milltown_a = false, c4m2_sugarmill_a = false, c4m3_sugarmill_b = false, c4m4_milltown_b = false, c4m5_milltown_escape = false,
				 c5m1_waterfront = false, c5m1_waterfront_sndscape = false, c5m2_park = false, c5m3_cemetery = false, c5m4_quarter = false, c5m5_bridge = false,
				 credits = false, curling_stadium = false, tutorial_standards = false, tutorial_standards_vs = false,
				 c6m1_riverbank = false, c6m2_bedlam = false, c6m3_port = false,
				 c7m1_docks = false, c7m2_barge = false, c7m3_port = false,
				 c8m1_apartment = false, c8m2_subway = false, c8m3_sewers = false, c8m4_interior = false, c8m5_rooftop = false,
				 c9m1_alleys = false, c9m2_lots = false,
				 c10m1_caves = false, c10m2_drainage = false, c10m3_ranchhouse = false, c10m4_mainstreet = false, c10m5_houseboat = false,
				 c11m1_greenhouse = false, c11m2_offices = false, c11m3_garage = false, c11m4_terminal = false, c11m5_runway = false,
				 c12m1_hilltop = false, c12m2_traintunnel = false, c12m3_bridge = false, c12m4_barn = false, c12m5_cornfield = false,
				 c13m1_alpinecreek = false, c13m2_southpinestream = false, c13m3_memorialbridge = false, c13m4_cutthroatcreek = false;

			void update(const std::string& n)
			{
					 if (n == "c1m1_hotel") c1m1_hotel = true;
				else if (n == "c1m2_streets") c1m2_streets = true;
				else if (n == "c1m3_mall") c1m3_mall = true;
				else if (n == "c1m4_atrium") c1m4_atrium = true;
				else if (n == "c2m1_highway") c2m1_highway = true;
				else if (n == "c2m2_fairgrounds") c2m2_fairgrounds = true;
				else if (n == "c2m3_coaster") c2m3_coaster = true;
				else if (n == "c2m4_barns") c2m4_barns = true;
				else if (n == "c2m5_concert") c2m5_concert = true;
				else if (n == "c3m1_plankcountry") c3m1_plankcountry = true;
				else if (n == "c3m2_swamp") c3m2_swamp = true;
				else if (n == "c3m3_shantytown") c3m3_shantytown = true;
				else if (n == "c3m4_plantation") c3m4_plantation = true;
				else if (n == "c4m1_milltown_a") c4m1_milltown_a = true;
				else if (n == "c4m2_sugarmill_a") c4m2_sugarmill_a = true;
				else if (n == "c4m3_sugarmill_b") c4m3_sugarmill_b = true;
				else if (n == "c4m4_milltown_b") c4m4_milltown_b = true;
				else if (n == "c4m5_milltown_escape") c4m5_milltown_escape = true;
				else if (n == "c5m1_waterfront") c5m1_waterfront = true;
				else if (n == "c5m1_waterfront_sndscape") c5m1_waterfront_sndscape = true;
				else if (n == "c5m2_park") c5m2_park = true;
				else if (n == "c5m3_cemetery") c5m3_cemetery = true;
				else if (n == "c5m4_quarter") c5m4_quarter = true;
				else if (n == "c5m5_bridge") c5m5_bridge = true;
				else if (n == "credits") credits = true;
				else if (n == "curling_stadium") curling_stadium = true;
				else if (n == "tutorial_standards") tutorial_standards = true;
				else if (n == "tutorial_standards_vs") tutorial_standards_vs = true;
				else if (n == "c6m1_riverbank") c6m1_riverbank = true;
				else if (n == "c6m2_bedlam") c6m2_bedlam = true;
				else if (n == "c6m3_port") c6m3_port = true;
				else if (n == "c7m1_docks") c7m1_docks = true;
				else if (n == "c7m2_barge") c7m2_barge = true;
				else if (n == "c7m3_port") c7m3_port = true;
				else if (n == "c8m1_apartment") c8m1_apartment = true;
				else if (n == "c8m2_subway") c8m2_subway = true;
				else if (n == "c8m3_sewers") c8m3_sewers = true;
				else if (n == "c8m4_interior") c8m4_interior = true;
				else if (n == "c8m5_rooftop") c8m5_rooftop = true;
				else if (n == "c9m1_alleys") c9m1_alleys = true;
				else if (n == "c9m2_lots") c9m2_lots = true;
				else if (n == "c10m1_caves") c10m1_caves = true;
				else if (n == "c10m2_drainage") c10m2_drainage = true;
				else if (n == "c10m3_ranchhouse") c10m3_ranchhouse = true;
				else if (n == "c10m4_mainstreet") c10m4_mainstreet = true;
				else if (n == "c10m5_houseboat") c10m5_houseboat = true;
				else if (n == "c11m1_greenhouse") c11m1_greenhouse = true;
				else if (n == "c11m2_offices") c11m2_offices = true;
				else if (n == "c11m3_garage") c11m3_garage = true;
				else if (n == "c11m4_terminal") c11m4_terminal = true;
				else if (n == "c11m5_runway") c11m5_runway = true;
				else if (n == "c12m1_hilltop") c12m1_hilltop = true;
				else if (n == "c12m2_traintunnel") c12m2_traintunnel = true;
				else if (n == "c12m3_bridge") c12m3_bridge = true;
				else if (n == "c12m4_barn") c12m4_barn = true;
				else if (n == "c12m5_cornfield") c12m5_cornfield = true;
				else if (n == "c13m1_alpinecreek") c13m1_alpinecreek = true;
				else if (n == "c13m2_southpinestream") c13m2_southpinestream = true;
				else if (n == "c13m3_memorialbridge") c13m3_memorialbridge = true;
				else if (n == "c13m4_cutthroatcreek") c13m4_cutthroatcreek = true;
			}

			void reset()
			{
				memset(this, 0, sizeof(level_bool_s));
			}
		};

		static inline level_bool_s is_level = {};

	private:
		static inline map_settings_s m_map_settings = {};
		static inline std::vector<std::string> m_args;
		static inline bool m_spawned_markers = false;
		static inline bool m_loaded = false;

		bool parse_toml();
		bool matches_map_name();
		void open_and_set_var_config(const std::string& config, bool no_error = false, bool ignore_hashes = false, const char* custom_path = nullptr);
	};
}
