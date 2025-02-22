#include "std_include.hpp"

namespace components
{
	namespace cmd
	{
		bool debug_pos_time = false;
		bool show_api_lights = false;
	}

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
		(m_points)[0].timepoint = 0.0f;

		// total duration defined by the last point
		m_total_duration = m_points.back().timepoint;

		if (points.size() > 1 && m_total_duration == 0.0f)
		{
			game::console();
			std::cout << "[RemixLights][light_interpolator::init] Encountered a light were the last point has no defined timepoint! Placeholder in-use, please fix!" << std::endl;

			// use timepoint of prev. point + 1.0
			m_total_duration = (m_points)[m_points.size() - 2].timepoint + 1.0f;

			// write the placeholder value into the last point
			m_points.back().timepoint = m_total_duration; 
		}

		// calculate time for points with no defined timepoint
		bool needs_timepoint_calc = false;
		size_t calc_index_start = 0;

		for (size_t i = 1; i < m_points.size(); ++i)
		{
			// point has no defined timepoint
			if ((m_points)[i].timepoint == 0.0f)
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
	 * @param position			(LightInfoEXT)
	 * @param radiance			(LightInfo)
	 * @param radius			(LightInfo)
	 * @param direction			(LightInfoEXT)
	 * @param degrees			(LightInfoEXT)
	 * @param softness			(LightInfoEXT)
	 * @param exponent			(LightInfoEXT)
	 */
	void light_interpolator::interpolate(remixapi_Float3D* position, remixapi_Float3D* radiance, float* radius, 
										 remixapi_Float3D* direction, float* degrees, float* softness, float* exponent)
	{
		{
			map_settings::remix_light_settings_s::point_s* temp_pt = nullptr;
			if (m_elapsed_time <= 0.0f) {
				temp_pt = &m_points.front();
			} else if (m_elapsed_time >= m_total_duration) {
				temp_pt = &m_points.back();
			}

			if (temp_pt)
			{
				if (position) { *position = (temp_pt->position + m_position_offset).ToRemixFloat3D(); }
				if (radiance) { *radiance = (temp_pt->radiance * temp_pt->radiance_scalar).ToRemixFloat3D(); }
				if (radius) { *radius = temp_pt->radius; }
				if (direction) { *direction = temp_pt->direction.ToRemixFloat3D(); }
				if (degrees) { *degrees = temp_pt->degrees; }
				if (softness) { *softness = temp_pt->softness; }
				if (exponent) { *exponent = temp_pt->exponent; }

				return;
			}
		}

		float time = m_elapsed_time;
		for (size_t i = 0; i < m_segment_durations.size(); ++i)
		{
			if (time <= m_segment_durations[i]) 
			{
				const auto t = time / m_segment_durations[i];
				const auto& p0 = (m_points)[((i - 1) + m_points.size()) % m_points.size()];
				const auto& p1 = (m_points)[i % m_points.size()];
				const auto& p2 = (m_points)[(m_loop_smoothing && i == m_points.size() - 1) ? 0 : (i + 1) % m_points.size()];
				const auto& p3 = (m_points)[(i + 2) % m_points.size()];

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
						+ m_position_offset).ToRemixFloat3D();
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
		if (position) { *position = (m_points.back().position + m_position_offset).ToRemixFloat3D(); }
		if (radiance) { *radiance = (m_points.back().radiance * m_points.back().radiance_scalar).ToRemixFloat3D(); }
		if (radius) { *radius = m_points.back().radius; }
		if (direction) { *direction = m_points.back().direction.ToRemixFloat3D(); }
		if (degrees) { *degrees = m_points.back().degrees; }
		if (softness) { *softness = m_points.back().softness; }
		if (exponent) { *exponent = m_points.back().exponent; }
	}

	// ----

	/**
	 * Update a remixApi light using an "external" point
	 * @param light		Light handle
	 * @param pt		External point handle
	 * @return			True if successfull
	 */
	bool remix_lights::update_static_remix_light(remix_light_s* light, const map_settings::remix_light_settings_s::point_s* pt)
	{
		if (!light || !pt) {
			return false;
		}

		if (light->handle) {
			destroy_map_light(light);
		}

		if (light)
		{
			light->ext.position = (pt->position + light->attached_offset).ToRemixFloat3D();
			light->info.radiance = (pt->radiance * pt->radiance_scalar).ToRemixFloat3D();
			light->ext.radius = pt->radius;
			light->ext.shaping_hasvalue = pt->use_shaping;
			light->ext.shaping_value.direction = pt->direction.ToRemixFloat3D();
			light->ext.shaping_value.coneAngleDegrees = pt->degrees;
			light->ext.shaping_value.coneSoftness = pt->softness;
			light->ext.shaping_value.focusExponent = pt->exponent;

			// not updating these can result in a crash in bridge::remix_api?
			light->ext.pNext = nullptr;
			light->ext.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
			light->info.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
			light->info.pNext = &light->ext;

			return remix_api::get()->m_bridge.CreateLight(&light->info, &light->handle) == REMIXAPI_ERROR_CODE_SUCCESS;
		}

		return false;
	}

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
			light->mover.set_position_offset(light->attached_offset);

			light->mover.interpolate(
				&light->ext.position, 
				&light->info.radiance, 
				&light->ext.radius,
				&light->ext.shaping_value.direction,
				&light->ext.shaping_value.coneAngleDegrees,
				&light->ext.shaping_value.coneSoftness,
				&light->ext.shaping_value.focusExponent);

			light->ext.shaping_hasvalue = light->ext.shaping_value.coneAngleDegrees != 180.0f;

			// not updating these can result in a crash in bridge::remix_api?
			light->ext.pNext = nullptr;
			light->ext.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
			light->info.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
			light->info.pNext = &light->ext;

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
			light->ext.position = pt.position.ToRemixFloat3D();
			// disable single point lights with attach params until they get attached later down the line
			light->ext.radius = light->def.points.size() == 1u && light->has_attach_parms() ? 0.0f : pt.radius;
			light->ext.shaping_hasvalue = pt.use_shaping;
			light->ext.shaping_value = {};
			light->ext.shaping_value.direction = pt.direction.ToRemixFloat3D();
			light->ext.shaping_value.coneAngleDegrees = pt.degrees;
			light->ext.shaping_value.coneSoftness = pt.softness;
			light->ext.shaping_value.focusExponent = pt.exponent;

			light->info.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
			light->info.pNext = &light->ext;
			light->info.hash = utils::string_hash64(utils::va("api-light%d", light->light_num));
			light->info.radiance = (pt.radiance * pt.radiance_scalar).ToRemixFloat3D();

			const auto api = remix_api::get();
			return api->m_bridge.CreateLight(&light->info, &light->handle) == REMIXAPI_ERROR_CODE_SUCCESS;
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
			if (it->trigger_choreo_name.empty() && !it->trigger_sound_hash) // add lights without a trigger
			{
				m_active_lights.push_back(
					remix_light_s
					{
						*it, //std::move(*it),
						m_active_light_spawn_tracker++,
						it->kill_delay
					});

				// erase element from the mapsettings vector
				it = msettings.remix_lights.erase(it);

				auto* light = &m_active_lights.back();

				if (light->def.points.size() > 1) {
					light->mover.init(light->def.points, light->def.loop, light->def.loop_smoothing);
				}

				// spawn it
				get()->spawn_remix_light(light);
			}
			else { ++it; }
		}
	}


	void remix_lights::add_single_map_setting_light_for_editing(map_settings::remix_light_settings_s* def)
	{
		m_active_lights.push_back(
			remix_light_s(
				*def,
				m_active_light_spawn_tracker++
			));

		auto* light = &m_active_lights.back();

		if (light->def.points.size() > 1) {
			light->mover.init(light->def.points, true /* always loop*/, light->def.loop_smoothing);
		}

		// spawn it
		get()->spawn_remix_light(light);
	}

	/**
	 * Adds a single map setting light to 'm_map_lights' - Immediately spawns it if trigger is not defined
	 * @param def	The map_setting light definition
	 */
	void remix_lights::add_single_map_setting_light(map_settings::remix_light_settings_s* def)
	{
		m_active_lights.push_back(
			remix_light_s(
				*def, //def->trigger_always ? *def : std::move(*def), // do not move the light if it can be triggered multiple times
				m_active_light_spawn_tracker++
			));

		auto* light = &m_active_lights.back();

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
		for (auto& l : m_active_lights) {
			destroy_map_light(&l);
		}
	}

	/**
	 * Destroys all lights in 'm_map_lights' (remixApi lights) and clears 'm_map_lights' 
	 */
	void remix_lights::destroy_and_clear_all_active_lights()
	{
		destroy_all_map_lights();
		m_active_lights.clear();
	}

	/**
	 * Updates all lights in 'm_map_lights'
	 * Handles Destroying, choreo trigger spawning, tick advancing and updating of remixApi lights
	 */
	void remix_lights::update_all_active_lights()
	{
		const auto edit_mode = imgui::get()->m_light_edit_mode;

		// destroy lights that are marked for destruction
		for (auto it = m_active_lights.begin(); it != m_active_lights.end();)
		{
			if (it->is_marked_for_destruction)
			{
				// kill delay timer
				if (it->timer > 0.0f) 
				{
					it->timer -= interfaces::get()->m_globals->absoluteframetime;
					++it;
				}
				else
				{
					destroy_map_light(&*it);
					it = m_active_lights.erase(it);
				}
			}
			else { ++it; }
		}

		// iterate all map lights
		for (auto& l : m_active_lights)
		{
			if (l.mover.is_initialized())
			{
				const auto finished = l.mover.advance_time(interfaces::get()->m_globals->absoluteframetime);
				update_remix_light(&l);

				if (!edit_mode)
				{
					if (finished && l.def.run_once) { // destroy light on next frame
						l.is_marked_for_destruction = true;
					}
				}
			}
			else // single point lights
			{
				if (l.has_attach_parms())
				{
					if (l.is_attached()) { // update every frame when attached
						update_static_remix_light(&l, &l.def.points.front());
					}
					else if (l.ext.radius > 0.0f) // "disable" light when it gets unattached
					{
						auto temp_pt = l.def.points.front();
						temp_pt.radius = 0.0f;
						update_static_remix_light(&l, &temp_pt);
					}
				}
			}

			// if light is not yet spawned
			if (!l.handle)
			{
				// handle delayed triggering
				if (l.timer < l.def.trigger_delay) {
					l.timer += interfaces::get()->m_globals->absoluteframetime;
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

	// Draw all active map lights
	void remix_lights::draw_all_active_lights()
	{
		for (auto& l : m_active_lights)
		{
			if (l.handle) {
				remix_api::get()->m_bridge.DrawLightInstance(l.handle);
			}
		}
	}

	// #
	// #

	bool light_attachment_is_matching_model(const remix_lights::remix_light_s& light, const ModelRenderInfo_t& info)
	{
		const bool has_radius = light.def.attach_prop_radius != 0.0f;
		const bool has_name = !light.def.attach_prop_name.empty();

		if (!has_radius && !has_name) {
			return false;
		}

		// bounds check
		if (!light.def.attach_prop_mins.IsZero() || !light.def.attach_prop_maxs.IsZero())
		{
			if (!utils::vector::is_point_in_aabb(info.origin, light.def.attach_prop_mins, light.def.attach_prop_maxs)) {
				return false;
			}
		}

		// radius check if specified
		if (has_radius && !utils::float_equal(info.pModel->radius, light.def.attach_prop_radius)) {
			return false;
		}

		// name substring check if specified
		if (has_name && !std::string_view(info.pModel->szPathName).contains(light.def.attach_prop_name)) {
			return false;
		}

		return true;
	}

	// called from model_renderer::DrawModelExecute::Detour
	void remix_lights::on_draw_model_exec(const ModelRenderInfo_t& info)
	{
		if (map_settings::get_map_settings().using_any_light_attached_to_prop)
		{
			for (auto& light : m_active_lights)
			{
				if (light.attachframe != m_attachframe_counter)
				{
					if (light_attachment_is_matching_model(light, info))
					{
						light.attachframe = m_attachframe_counter; // mark as processed this frame
						light.attached_offset = info.origin;
						//break; // we might have other lights attached to this model
					}
				}
			}
		}
	}

	// called from: choreo_events::scene_ent_on_start_event_hk
	void remix_lights::on_event_start(const std::string_view& name, const std::string_view& actor, const std::string_view& event, const std::string_view& param1)
	{
		// no event trigger in edit mode
		if (imgui::get()->m_light_edit_mode) {
			return;
		}

		auto& msettings = map_settings::get_map_settings();
		for (auto it = msettings.remix_lights.begin(); it != msettings.remix_lights.end();)
		{
			if (!it->trigger_choreo_name.empty() && name.contains(it->trigger_choreo_name))
			{
				// check if opt. actor is defined and matches event actor
				if (!it->trigger_choreo_actor.empty() && !actor.contains(it->trigger_choreo_actor)) {
					++it; continue;
				}

				// check if opt. event is defined and matches event string
				if (!it->trigger_choreo_event.empty() && !event.contains(it->trigger_choreo_event)) {
					++it; continue;
				}

				// check if opt. param1 is defined and matches event param1
				if (!it->trigger_choreo_param1.empty() && !param1.contains(it->trigger_choreo_param1)) {
					++it; continue;
				}

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
		// no event trigger in edit mode
		if (imgui::get()->m_light_edit_mode) {
			return;
		}

		for (auto& l : m_active_lights)
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

	void remix_lights::on_sound_start(const std::uint32_t hash)
	{
		// no event trigger in edit mode
		if (imgui::get()->m_light_edit_mode) {
			return;
		}

		// check for kill trigger
		for (auto& l : m_active_lights)
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
		const auto rml = remix_lights::get();
		const auto& glob = interfaces::get()->m_globals;

		// check if paused
		rml->m_is_paused = utils::float_equal(glob->frametime, 0.0f);

		if (!rml->m_is_paused) 
		{
			rml->update_all_active_lights();
			rml->debug_print_player_pos_time();

			++m_attachframe_counter;
		}

		rml->draw_all_active_lights();

		if (cmd::show_api_lights)
		{
			bool first_done = false;
			for (const auto& l : m_active_lights)
			{
				const Vector circle_pos = &l.ext.position.x;
				const float radius = l.ext.radius;
				const Vector color = { 1.0f, 1.0f, 1.0f };

				const auto remixapi = remix_api::get();

				// we only need to craft one circle instance - everything else is instanced
				if (!first_done)
				{
					first_done = true;
					remixapi->add_debug_circle(circle_pos, Vector(0.0f, 0.0f, 1.0f), radius - 0.02f, radius * 0.1f, color);
				}
				else {
					remixapi->add_debug_circle_based_on_previous(circle_pos, Vector(0, 0, 90), Vector(1.0f, 1.0f, 1.0f));
				}

				remixapi->add_debug_circle_based_on_previous(circle_pos, Vector(0, 90, 0), Vector(1.0f, 1.0f, 1.0f));
				remixapi->add_debug_circle_based_on_previous(circle_pos, Vector(90, 0, 90), Vector(1.0f, 1.0f, 1.0f));
			}
		}
		
	}

	// called before map_settings
	void remix_lights::on_map_load()
	{
		// reset spawn tracker
		m_active_light_spawn_tracker = 0u;
		m_attachframe_counter = 0u;
	}

	void remix_lights::debug_print_player_pos_time()
	{
		if (cmd::debug_pos_time)
		{
			const auto& glob = interfaces::get()->m_globals;

			m_dbgpos_last_curtime = glob->curtime;

			// update print timer
			m_dbgpos_print_timer += glob->absoluteframetime;

			const auto* curpos = game::get_current_view_origin();

			// check if player moves
			if (m_dbgpos_last_pos == *curpos)
			{
				m_dbgpos_timer_since_movement = 0.0f;

				if (m_dbgpos_on_steady_once) {
					game::print_ingame("[POS] Movement End\n");
				}

				m_dbgpos_on_steady_once = false;
			}
			else 
			{
				if (!m_dbgpos_on_steady_once)
				{
					m_dbgpos_on_steady_once = true;
					m_dbgpos_timepoint_on_movement = glob->curtime;
					game::print_ingame("[POS] Movement Start\n");
				}

				m_dbgpos_timer_since_movement += glob->absoluteframetime;

				game::print_ingame("> POS [%.2f, %.2f, %.2f] @ timer [%.3f] -- @ delta [%.3f] -- @ curtime [%.3f]\n",
					curpos->x, curpos->y, curpos->z, 
					m_dbgpos_timer_since_movement, 
					glob->curtime - m_dbgpos_timepoint_on_movement,
					glob->curtime);
			}

			// print every 0.2s
			/*if (m_dbgpos_print_timer >= 0.2f)
			{
				m_dbgpos_print_timer = 0.0f;
			}*/

			m_dbgpos_last_pos = *game::get_current_view_origin();
		}
	}

	ConCommand xo_debug_toggle_pos_time_cmd {};
	void xo_debug_toggle_pos_time_fn()
	{
		cmd::debug_pos_time = !cmd::debug_pos_time;
	}

	ConCommand xo_debug_toggle_show_api_lights_cmd {};
	void xo_debug_toggle_show_api_lights_fn()
	{
		cmd::show_api_lights = !cmd::show_api_lights;
	}

	remix_lights::remix_lights()
	{
		p_this = this;

		// #
		// commands

		game::con_add_command(&xo_debug_toggle_pos_time_cmd, "xo_debug_toggle_pos_time", xo_debug_toggle_pos_time_fn, "Toggle debug prints about player position and time (useful for animated lights)");
		game::con_add_command(&xo_debug_toggle_show_api_lights_cmd, "xo_debug_toggle_show_api_lights", xo_debug_toggle_show_api_lights_fn, "Toggle debug vis for lights added via the remixapi");
	}
}
