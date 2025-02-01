#pragma once
#include "map_settings.hpp"

namespace components
{
	class light_interpolator
	{
	public:
		light_interpolator() = default;

		bool init(const std::vector<map_settings::remix_light_settings_s::point_s>& points,
			bool looping = false, bool loop_smoothing = false);

		bool is_initialized() const { return m_initialized; }

		bool advance_time(float frametime);
		void interpolate(
			remixapi_Float3D* position = nullptr, 
			remixapi_Float3D* radiance = nullptr, 
			float* radius = nullptr,
			remixapi_Float3D* direction = nullptr,
			float* degrees = nullptr,
			float* softness = nullptr, 
			float* exponent = nullptr
		);
	
	private:
		bool m_initialized = false;
		bool m_looping = false;
		bool m_loop_smoothing = false;
		std::vector<map_settings::remix_light_settings_s::point_s> m_points;
		std::vector<float> m_segment_durations;
		float m_elapsed_time = 0.0f;
		float m_total_duration = 0.0f;

		void calculate_segment_durations()
		{
			m_segment_durations.clear();
			float total_original_duration = 0.0f;

			// calculate the sum of all segment durations as defined by the points
			for (size_t i = 0; i < m_points.size() - 1; ++i)
			{
				float segment_duration = m_points[i + 1].timepoint - m_points[i].timepoint;
				total_original_duration += segment_duration;
				m_segment_durations.push_back(segment_duration);
			}

			if (m_looping && m_loop_smoothing && m_points.size() > 1)
			{
				float last_to_first_duration = m_points.back().timepoint - m_points[m_points.size() - 2].timepoint;
				total_original_duration += last_to_first_duration;
				m_segment_durations.push_back(last_to_first_duration);
			}

			if (m_looping && m_loop_smoothing && total_original_duration > 0.0f)
			{
				const float scale_factor = m_total_duration / total_original_duration;
				for (auto& duration : m_segment_durations) {
					duration *= scale_factor;
				}
			}
		}

		void interpolate_timepoints(const size_t start, const size_t end)
		{
			float prev_time = (start > 0) ? m_points[start - 1].timepoint : 0.0f;
			const float next_time = m_points[end].timepoint;

			const size_t segment_count = end - start;
			const float segment_duration = (next_time - prev_time) / static_cast<float>(segment_count + 1);

			for (size_t i = start; i < end; ++i)
			{
				if (m_points[i].timepoint == 0.0f) {
					m_points[i].timepoint = prev_time + segment_duration;
				}
				prev_time = m_points[i].timepoint; // update for next iteration
			}
		}

		template<typename T>
		T lerp(const T& start, const T& end, float t) const {
			return start + (end - start) * t;
		}
	};

	class remix_lights : public component
	{
	public:
		remix_lights();

		static inline remix_lights* p_this = nullptr;
		static remix_lights* get() { return p_this; }

		static void on_event_start(const std::string_view& name, const std::string_view& actor, const std::string_view& event, const std::string_view& param1);
		static void on_event_finish(const std::string_view& name);
		static void on_sound_start(std::uint32_t hash);
		static void on_client_frame();
		static void on_map_load();

		// -----

		struct remix_light_s
		{
			map_settings::remix_light_settings_s def;
			std::uint32_t light_num = 0u;
			float timer = 0.0f;
			bool is_marked_for_destruction = false;
			light_interpolator mover;
			remixapi_LightHandle handle = nullptr;
			remixapi_LightInfoSphereEXT ext = {};
			remixapi_LightInfo info = {};
		};

		bool update_remix_light(remix_light_s* light);
		bool spawn_remix_light(remix_light_s* light);

		void add_all_map_setting_lights_without_creation_trigger();
		void add_single_map_setting_light(map_settings::remix_light_settings_s* def);
		void destroy_map_light(remix_light_s* light);
		void destroy_all_map_lights();
		void destroy_and_clear_all_map_lights();
		void update_all_map_lights();
		void draw_all_map_lights();

	private:
		static inline std::uint32_t m_map_light_spawn_tracker = 0u;
		static inline std::vector<remix_light_s> m_map_lights = {};
	};
}
