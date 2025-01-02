#include "std_include.hpp"

namespace components
{
	/**
	 * Initializes the light interpolator
	 * @param points			Reference to point-list
	 * @param looping			Light is looping
	 * @param loop_smoothing	Add additional segment between last and first point
	 * @return 
	 */
	bool light_interpolator::init(const std::vector<map_settings::remix_light_settings_s::point_s>& points, const bool looping, const bool loop_smoothing)
	{
		if (points.size() == 1) {
			return false;
		}

		m_initialized = true;
		m_points = points;
		m_looping = looping;
		m_loop_smoothing = loop_smoothing;

		// ensure first point has timepoint 0
		m_points[0].timepoint = 0.0f;

		// total duration defined by the last point
		m_total_duration = m_points.back().timepoint;

		if (points.size() > 1 && m_total_duration == 0.0f)
		{
			game::console();
			std::cout << "[RemixLights][light_interpolator::init] Encountered a light were the last point has no defined timepoint! Placeholder in-use, please fix!" << std::endl;

			// use timepoint of prev. point + 1.0
			m_total_duration = m_points[m_points.size() - 2].timepoint + 1.0f;

			// write the placeholder value into the last point
			m_points.back().timepoint = m_total_duration; 
		}

		// calculate time for points with no defined timepoint
		bool needs_timepoint_calc = false;
		size_t calc_index_start = 0;

		for (size_t i = 1; i < m_points.size(); ++i)
		{
			// point has no defined timepoint
			if (m_points[i].timepoint == 0.0f)
			{
				needs_timepoint_calc = true;
				if (calc_index_start == 0) {
					calc_index_start = i;
				}
			}
			else // point with timepoint
			{
				if (needs_timepoint_calc) // check if previous points had no timepoint
				{
					interpolate_timepoints(calc_index_start, i); // evenly distribute time
					needs_timepoint_calc = false;
					calc_index_start = 0;
				}
			}
		}

		calculate_segment_durations();
		return true;
	}

	/**
	 * Advances time
	 * @param frametime		Time of the last frame
	 * @return				True if move is done (also true for looping lights)
	 */
	bool light_interpolator::advance_time(const float frametime)
	{
		m_elapsed_time += frametime;

		if (m_elapsed_time >= m_total_duration) 
		{
			if (m_looping)  {
				m_elapsed_time -= m_total_duration;
			}
			else  {
				m_elapsed_time = m_total_duration;
			}

			return true;
		}

		return false;
	}

	/**
	 * Calculate light properties for the current tick \n
	 * All arguments are optional - use nullptr to not update a specific property
	 * @param position		(LightInfoEXT)
	 * @param radiance		(LightInfo)
	 * @param radius		(LightInfo)
	 * @param direction		(LightInfoEXT)
	 * @param degrees		(LightInfoEXT)
	 * @param softness		(LightInfoEXT)
	 * @param exponent		(LightInfoEXT)
	 */
	void light_interpolator::interpolate(remixapi_Float3D* position, remixapi_Float3D* radiance, float* radius, 
										 remixapi_Float3D* direction, float* degrees, float* softness, float* exponent)
	{
		{
			map_settings::remix_light_settings_s::point_s* temp_pt = nullptr;
			if (m_elapsed_time <= 0.0f) {
				temp_pt = &m_points[0];
			} else if (m_elapsed_time >= m_total_duration) {
				temp_pt = &m_points.back();
			}

			if (temp_pt)
			{
				if (position) { *position = temp_pt->position.ToRemixFloat3D(); }
				if (radiance) { *radiance = (temp_pt->radiance * temp_pt->radiance_scalar).ToRemixFloat3D(); }
				if (radius) { *radius = temp_pt->radius; }
				if (direction) { *direction = temp_pt->direction.ToRemixFloat3D(); }
				if (degrees) { *degrees = temp_pt->degrees; }
				if (softness) { *softness = temp_pt->softness; }
				if (exponent) { *exponent = temp_pt->exponent; }
			}
		}

		float time = m_elapsed_time;
		for (size_t i = 0; i < m_segment_durations.size(); ++i)
		{
			if (time <= m_segment_durations[i]) 
			{
				const auto t = time / m_segment_durations[i];
				const auto& p0 = m_points[((i - 1) + m_points.size()) % m_points.size()];
				const auto& p1 = m_points[i % m_points.size()];
				const auto& p2 = m_points[(m_loop_smoothing && i == m_points.size() - 1) ? 0 : (i + 1) % m_points.size()];
				const auto& p3 = m_points[(i + 2) % m_points.size()];

				const float t2 = t * t;
				const float t3 = t2 * t;

				// hermite interpolation
				if (position)
				{
					*position = (
						  p1.position * (2.0f * t3 - 3.0f * t2 + 1.0f) 
						+ p2.position * (-2.0f * t3 + 3.0f * t2) 
						+ (p2.position - p0.position) * p1.smoothness * (t3 - 2.0f * t2 + t) 
						+ (p3.position - p1.position) * p2.smoothness * (t3 - t2)
						).ToRemixFloat3D();
				}

				if (direction)
				{
					Vector dir = (
						  p1.direction * (2.0f * t3 - 3.0f * t2 + 1.0f)
						+ p2.direction * (-2.0f * t3 + 3.0f * t2)
						+ (p2.direction - p0.direction) * p1.smoothness * (t3 - 2.0f * t2 + t)
						+ (p3.direction - p1.direction) * p2.smoothness * (t3 - t2)
						);

					dir.Normalize();
					*direction = dir.ToRemixFloat3D();
				}

				// linear interpolations

				if (radiance) {
					*radiance = lerp((p1.radiance * p1.radiance_scalar), (p2.radiance * p2.radiance_scalar), t).ToRemixFloat3D();
				}

				if (radius) {
					*radius = lerp(p1.radius, p2.radius, t);
				}

				if (degrees) {
					*degrees = lerp(p1.degrees, p2.degrees, t);
				}

				if (softness) {
					*softness = lerp(p1.softness, p2.softness, t);
				}

				if (exponent) {
					*exponent = lerp(p1.exponent, p2.exponent, t);
				}

				return;
			}

			time -= m_segment_durations[i];
		}

		// should not happen if durations are correct
		if (position) { *position = m_points.back().position.ToRemixFloat3D(); }
		if (radiance) { *radiance = (m_points.back().radiance * m_points.back().radiance_scalar).ToRemixFloat3D(); }
		if (radius) { *radius = m_points.back().radius; }
		if (direction) { *direction = m_points.back().direction.ToRemixFloat3D(); }
		if (degrees) { *degrees = m_points.back().degrees; }
		if (softness) { *softness = m_points.back().softness; }
		if (exponent) { *exponent = m_points.back().exponent; }
	}

	// ----

	/**
	 * Calculate and update remixApi light for the current tick 
	 * @param light		The light
	 * @return			True if successfull
	 */
	bool remix_lights::update_remix_light(remix_light_s* light)
	{
		if (!light || (light && !light->mover.is_initialized())) {
			return false;
		}

		if (light->handle) {
			destroy_map_light(light);
		}

		if (light)
		{
			light->mover.interpolate(
				&light->ext.position, 
				&light->info.radiance, 
				&light->ext.radius,
				&light->ext.shaping_value.direction,
				&light->ext.shaping_value.coneAngleDegrees,
				&light->ext.shaping_value.coneSoftness,
				&light->ext.shaping_value.focusExponent);

			light->ext.shaping_hasvalue = light->ext.shaping_value.coneAngleDegrees != 180.0f;

			return remix_api::get()->m_bridge.CreateLight(&light->info, &light->handle) == REMIXAPI_ERROR_CODE_SUCCESS;
		}

		return false;
	}

	/**
	 * Spawns a remixApi light
	 * @param light		The light
	 * @return			True if successfull
	 */
	bool remix_lights::spawn_remix_light(remix_light_s* light)
	{
		if (!light) {
			return false;
		}

		if (light->handle) {
			destroy_map_light(light);
		}

		if (light)
		{
			const auto& pt = light->def.points[0];
			light->ext.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
			light->ext.pNext = nullptr;
			light->ext.position = remixapi_Float3D{ pt.position.x, pt.position.y, pt.position.z };
			light->ext.radius = pt.radius;
			light->ext.shaping_hasvalue = pt.use_shaping;
			light->ext.shaping_value = {};
			light->ext.shaping_value.direction = remixapi_Float3D{ pt.direction.x, pt.direction.y, pt.direction.z };
			light->ext.shaping_value.coneAngleDegrees = pt.degrees;
			light->ext.shaping_value.coneSoftness = pt.softness;
			light->ext.shaping_value.focusExponent = pt.exponent;

			light->info.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
			light->info.pNext = &light->ext;
			light->info.hash = utils::string_hash64(utils::va("api-light%d", light->light_num));
			light->info.radiance = remixapi_Float3D{ pt.radiance.x * pt.radiance_scalar, pt.radiance.y * pt.radiance_scalar, pt.radiance.z * pt.radiance_scalar };

			return remix_api::get()->m_bridge.CreateLight(&light->info, &light->handle) == REMIXAPI_ERROR_CODE_SUCCESS;
		}

		return false;
	}

	/**
	 * Adds all map_setting lights to 'm_map_lights' that have no defined trigger and removes them from the map_settings vector
	 */
	void remix_lights::add_all_map_setting_lights_without_creation_trigger()
	{
		// should have happend already, just to make sure
		get()->destroy_all_map_lights();

		auto& msettings = map_settings::get_map_settings();
		for (auto it = msettings.remix_lights.begin(); it != msettings.remix_lights.end();)
		{
			// add lights without a trigger
			if (it->trigger_choreo_name.empty() && !it->trigger_sound_hash)
			{
				m_map_lights.push_back(
					remix_light_s(
						std::move(*it),
						m_map_light_spawn_tracker++,
						it->kill_delay
					));

				// erase element from the mapsettings vector
				it = msettings.remix_lights.erase(it);

				auto* light = &m_map_lights.back();

				if (light->def.points.size() > 1) {
					light->mover.init(light->def.points, light->def.loop, light->def.loop_smoothing);
				}

				// spawn it
				get()->spawn_remix_light(light);
			}
			else { ++it; }
		}
	}

	/**
	 * Adds a single map setting light to 'm_map_lights' - Immediately spawns it if trigger is not defined
	 * @param def	The map_setting light definition
	 */
	void remix_lights::add_single_map_setting_light(map_settings::remix_light_settings_s* def)
	{
		m_map_lights.push_back(
			remix_light_s(
				def->trigger_always ? *def : std::move(*def), // do not move the light if it can be triggered multiple times
				m_map_light_spawn_tracker++
			));

		auto* light = &m_map_lights.back();

		// spawn light if it does not use a trigger - triggered spawning is handled elsewhere
		if (light->def.trigger_choreo_name.empty() && !light->def.trigger_sound_hash)
		{
			if (light->def.points.size() > 1) {
				light->mover.init(light->def.points, light->def.loop, light->def.loop_smoothing);
			}

			// spawn it
			get()->spawn_remix_light(light);
		}
	}

	/**
	 * Destroys a light (remixApi light)
	 * @param light		The light to destroy
	 */
	void remix_lights::destroy_map_light(remix_light_s* light)
	{
		if (light->handle)
		{
			remix_api::get()->m_bridge.DestroyLight(light->handle);
			light->handle = nullptr;
		}
	}

	/**
	 * Destroys all lights in 'm_map_lights' (remixApi lights)
	 */
	void remix_lights::destroy_all_map_lights()
	{
		for (auto& l : m_map_lights) {
			destroy_map_light(&l);
		}
	}

	/**
	 * Destroys all lights in 'm_map_lights' (remixApi lights) and clears 'm_map_lights' 
	 */
	void remix_lights::destroy_and_clear_all_map_lights()
	{
		destroy_all_map_lights();
		m_map_lights.clear();
	}

	/**
	 * Updates all lights in 'm_map_lights'
	 * Handles Destroying, choreo trigger spawning, tick advancing and updating of remixApi lights
	 */
	void remix_lights::update_all_map_lights()
	{
		// destroy lights that are marked for destruction
		for (auto it = m_map_lights.begin(); it != m_map_lights.end();)
		{
			if (it->is_marked_for_destruction) 
			{
				// kill delay timer
				if (it->timer > 0.0f) 
				{
					it->timer -= interfaces::get()->m_globals->frametime;
					++it;
				}
				else
				{
					destroy_map_light(&*it);
					it = m_map_lights.erase(it);
				}
			}
			else { ++it; }
		}

		// iterate all map lights
		for (auto& l : m_map_lights)
		{
			if (l.mover.is_initialized())
			{
				const auto finished = l.mover.advance_time(interfaces::get()->m_globals->frametime);
				update_remix_light(&l);

				if (finished && l.def.run_once) { // destroy light on next frame
					l.is_marked_for_destruction = true;
				}
			}

			// if light is not yet spawned
			if (!l.handle)
			{
				// handle delayed triggering
				if (l.timer < l.def.trigger_delay) {
					l.timer += interfaces::get()->m_globals->frametime;
				}
				else
				{
					if (l.def.points.size() > 1) {
						l.mover.init(l.def.points, l.def.loop, l.def.loop_smoothing);
					}

					// spawn it
					get()->spawn_remix_light(&l);

					// set timer to kill delay
					l.timer = l.def.kill_delay;
				}
			}
		}
	}

	/**
	 * Draws all lights in 'm_map_lights'
	 */
	void remix_lights::draw_all_map_lights()
	{
		for (auto& l : m_map_lights)
		{
			if (l.handle) {
				remix_api::get()->m_bridge.DrawLightInstance(l.handle);
			}
		}
	}

	// #
	// #

	// called from: choreo_events::scene_ent_on_start_event_hk
	void remix_lights::on_event_start(const std::string_view& name)
	{
		auto& msettings = map_settings::get_map_settings();
		for (auto it = msettings.remix_lights.begin(); it != msettings.remix_lights.end();)
		{
			if (!it->trigger_choreo_name.empty() && name.contains(it->trigger_choreo_name))
			{
				get()->add_single_map_setting_light(&*it);

				// only spawn on the very first play of the vcd
				if (!it->trigger_always) {
					it = msettings.remix_lights.erase(it); // erase element from the mapsettings vector
				}
				else { ++it; }
			}
			else { ++it; }
		}
	}

	// called from: choreo_events::scene_ent_on_finish_event_hk
	void remix_lights::on_event_finish(const std::string_view& name)
	{
		for (auto& l : m_map_lights)
		{
			// only check active lights with a kill trigger not yet marked to be destroyed
			if (l.handle && !l.def.kill_choreo_name.empty() && !l.is_marked_for_destruction)
			{
				if (name.contains(l.def.kill_choreo_name)) {
					l.is_marked_for_destruction = true;
				}
			}
		}
	}

	/**
	 * Check if we have to calculate a hash in on_sound_start
	 * @return	returns true if we require hashing
	 */
	bool remix_lights::on_sound_start_require_hash()
	{
		// check map_setting lights (spawn)
		auto& msettings = map_settings::get_map_settings();
		if (!msettings.remix_lights.empty())
		{
			for (const auto& l : msettings.remix_lights)
			{
				if (l.trigger_sound_hash || l.kill_sound_hash) {
					return true;
				}
			}
		}

		// check active lights (kill)
		for (auto& l : m_map_lights)
		{
			// only check active lights with a kill trigger not yet marked to be destroyed
			if (l.handle && l.def.kill_sound_hash && !l.is_marked_for_destruction) {
				return true;
			}
		}

		return false;
	}

	void remix_lights::on_sound_start(const std::uint32_t hash)
	{
		// check for kill trigger
		for (auto& l : m_map_lights)
		{
			// only check active lights with a kill trigger not yet marked to be destroyed
			if (l.handle && l.def.kill_sound_hash && !l.is_marked_for_destruction)
			{
				if (l.def.kill_sound_hash == hash) {
					l.is_marked_for_destruction = true;
				}
			}
		}

		// check for spawn trigger
		auto& msettings = map_settings::get_map_settings();
		for (auto it = msettings.remix_lights.begin(); it != msettings.remix_lights.end();)
		{
			if (it->trigger_sound_hash == hash)
			{
				get()->add_single_map_setting_light(&*it);

				// only spawn on the very first play of sound
				if (!it->trigger_always) {
					it = msettings.remix_lights.erase(it); // erase element from the mapsettings vector
				}
				else { ++it; }
			}
			else { ++it; }
		}
	}

	void remix_lights::on_client_frame()
	{
		get()->update_all_map_lights();
		get()->draw_all_map_lights();
	}

	// called before map_settings
	void remix_lights::on_map_load()
	{
		// reset spawn tracker
		m_map_light_spawn_tracker = 0u;
	}

	remix_lights::remix_lights()
	{
		p_this = this;
	}
}
