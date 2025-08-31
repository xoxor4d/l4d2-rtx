#pragma once
#include "map_settings.hpp"

namespace components
{
	namespace cmd
	{
		extern bool debug_pos_time;
		extern bool show_api_lights;
		extern bool show_mesh_bone_info_attached;
		extern bool show_mesh_bone_info;
	}

	class remix_lights : public component
	{
	public:
		remix_lights();

		static inline remix_lights* p_this = nullptr;
		static remix_lights* get() { return p_this; }

		static void on_draw_model_exec(const ModelRenderInfo_t& info);
		static void on_event_start(const std::string_view& name, const std::string_view& actor, const std::string_view& event, const std::string_view& param1);
		static void on_event_finish(const std::string_view& name);
		static void on_sound_start(std::uint32_t hash);
		static void on_client_frame();
		static void on_map_load();

		// -----

		class light
		{
		public:

			class interpolator
			{
			public:
				interpolator() = default;

				bool init(const std::vector<map_settings::remix_light_settings_s::point_s>& points,
					bool looping = false, bool loop_smoothing = false);

				bool is_initialized() const { return m_initialized; }

				bool advance_time(float frametime);
				void restart() { m_elapsed_time = 0.0f; }

				void interpolate(
					light* parent,
					remixapi_Float3D* position = nullptr,
					remixapi_Float3D* radiance = nullptr,
					float* radius = nullptr,
					remixapi_Float3D* direction = nullptr,
					float* degrees = nullptr,
					float* softness = nullptr,
					float* exponent = nullptr,
					float* volumetric_scale = nullptr,
					bool is_attached = false
				);

				map_settings::remix_light_settings_s::point_s* get_points() {
					return m_points.data();
				}

				std::vector<map_settings::remix_light_settings_s::point_s>& get_points_vec() {
					return m_points;
				}

				size_t get_points_count() const {
					return m_points.size();
				}

			private:
				bool m_initialized = false;
				bool m_looping = false;
				bool m_loop_smoothing = false;
				std::vector<map_settings::remix_light_settings_s::point_s> m_points;
				std::vector<float> m_segment_durations;
				float m_elapsed_time = 0.0f;
				float m_total_duration = 0.0f;

			public:
				void calculate_segment_durations()
				{
					// total duration defined by the last point
					m_total_duration = m_points.back().timepoint;

					m_segment_durations.clear();
					float total_original_duration = 0.0f;

					// calculate the sum of all segment durations as defined by the points
					for (size_t i = 0; i < m_points.size() - 1; ++i)
					{
						float segment_duration = (m_points)[i + 1].timepoint - (m_points)[i].timepoint;
						total_original_duration += segment_duration;
						m_segment_durations.push_back(segment_duration);
					}

					if (m_looping && m_loop_smoothing && m_points.size() > 1)
					{
						float last_to_first_duration = m_points.back().timepoint - (m_points)[m_points.size() - 2].timepoint;
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

			private:
				void interpolate_timepoints(const size_t start, const size_t end)
				{
					float prev_time = (start > 0) ? (m_points)[start - 1].timepoint : 0.0f;
					const float next_time = (m_points)[end].timepoint;

					const size_t segment_count = end - start;
					const float segment_duration = (next_time - prev_time) / static_cast<float>(segment_count + 1);

					for (size_t i = start; i < end; ++i)
					{
						if ((m_points)[i].timepoint == 0.0f) {
							(m_points)[i].timepoint = prev_time + segment_duration;
						}
						prev_time = (m_points)[i].timepoint; // update for next iteration
					}
				}

				template<typename T>
				T lerp(const T& start, const T& end, float t) const {
					return start + (end - start) * t;
				}
			};

			// ----

			bool has_spawn_trigger() const { return m_def.trigger_sound_hash || !m_def.trigger_choreo_actor.empty(); }
			bool has_kill_trigger() const { return m_def.kill_sound_hash || !m_def.kill_choreo_name.empty(); }
			bool has_attach_parms() const { return m_def.attach_prop_radius != 0.0f || !m_def.attach_prop_name.empty(); }
			bool is_attached() const { return m_attachframe && m_attachframe == m_attachframe_counter; }

			Vector calculate_direction_for_point(const map_settings::remix_light_settings_s::point_s* point) const;
			Vector calculate_position_for_point(const map_settings::remix_light_settings_s::point_s* point) const;

			// ----
			
			map_settings::remix_light_settings_s m_def = {};
			std::uint32_t m_light_num = 0u;
			float m_timer = 0.0f;
			bool m_is_marked_for_destruction = false;
			interpolator m_mover;
			remixapi_LightHandle m_handle = nullptr;
			remixapi_LightInfoSphereEXT m_ext = {};
			remixapi_LightInfo m_info = {};
			Vector m_attached_position;
			Vector m_attached_angle;
			Vector m_attached_bone_forward = { 1.0f, 0.0f, 0.0f };
			Vector m_attached_bone_right = { 0.0f, 1.0f, 0.0f };
			Vector m_attached_bone_up = { 0.0f, 0.0f, 1.0f };
			std::uint32_t m_attachframe = 0u;
			int m_entity_index = -1;
		};

		bool update_static_remix_light(light* light, const map_settings::remix_light_settings_s::point_s* pt);
		bool update_remix_light(light* light);
		bool spawn_remix_light(light* light);

		void add_all_map_setting_lights_without_creation_trigger();

		void add_single_map_setting_light_for_editing(map_settings::remix_light_settings_s* def);
		void add_single_map_setting_light(map_settings::remix_light_settings_s* def);

		void destroy_map_light(light* light);
		void destroy_all_map_lights();
		void destroy_and_clear_all_active_lights();
		void update_all_active_lights();
		void draw_all_active_lights();

		size_t get_active_light_count() { return m_active_lights.size(); }
		light* get_first_active_light() { return !m_active_lights.empty() ? &m_active_lights.front() : nullptr; }

		void debug_print_player_pos_time();

	private:
		static inline std::uint32_t m_active_light_spawn_tracker = 0u;
		static inline std::vector<light> m_active_lights = {};
		static inline std::uint32_t m_attachframe_counter = 0u;
		bool m_is_paused = false;

		// -
		float m_dbgpos_print_timer = 0.0f;
		float m_dbgpos_timer_since_movement = 0.0f;
		float m_dbgpos_timepoint_on_movement = 0.0f;
		Vector m_dbgpos_last_pos = {};
		bool m_dbgpos_on_steady_once = false;
		float m_dbgpos_last_curtime = 0.0f;

	};
}
