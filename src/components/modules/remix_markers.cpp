#include "std_include.hpp"

namespace components
{
	// draw 'nocull' map_setting marker meshes
	void remix_markers::draw_nocull_markers()
	{
		g_sunoverlay_color.clear(); // TODO: this should be moved somewhere else

		// -----

		const auto& msettings = map_settings::get_map_settings();
		const auto dev = game::get_d3d_device();

		struct vertex { D3DXVECTOR3 position; D3DCOLOR color; float tu, tv; };

		// early out - nope -> always render a single tri to register tex_addon texture
		/*if (msettings.map_markers.empty()) {
			return;
		}*/

		// save & restore after drawing
		IDirect3DVertexShader9* og_vs = nullptr;
		dev->GetVertexShader(&og_vs);
		dev->SetVertexShader(nullptr);

		IDirect3DBaseTexture9* og_tex = nullptr;
		dev->GetTexture(0, &og_tex);
		dev->SetTexture(0, tex_addons::white);

		DWORD og_rs;
		dev->GetRenderState((D3DRENDERSTATETYPE)150, &og_rs);

		dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		//D3DXMATRIX mtx = game::IDENTITY;

		for (auto& m : msettings.map_markers)
		{
			// ignore normal markers
			if (!m.no_cull) {
				continue;
			}

			// main_module::pre_recursive_world_node
			if (m.is_hidden) {
				continue;
			}

			const float f_index = static_cast<float>(m.index);
			const vertex mesh_verts[4] =
			{
				D3DXVECTOR3(-1.337f - (f_index * 0.01f), -1.337f - (f_index * 0.01f), 0), D3DCOLOR_XRGB(m.index, 0, 0), 0.0f, f_index / 100.0f,
				D3DXVECTOR3(1.337f + (f_index * 0.01f), -1.337f - (f_index * 0.01f), 0), D3DCOLOR_XRGB(0, m.index, 0), f_index / 100.0f, 0.0,
				D3DXVECTOR3(1.337f + (f_index * 0.01f),  1.337f + (f_index * 0.01f), 0), D3DCOLOR_XRGB(0, 0, m.index), 0.0f, f_index / 100.0f,
				D3DXVECTOR3(-1.337f - (f_index * 0.01f),  1.337f + (f_index * 0.01f), 0), D3DCOLOR_XRGB(m.index, 0, m.index), 0.0f, f_index / 100.0f,
			};

			D3DXMATRIX scale_matrix, rotation_x, rotation_y, rotation_z, mat_rotation, mat_translation, world;

			D3DXMatrixScaling(&scale_matrix, m.scale.x, m.scale.y, m.scale.z);
			D3DXMatrixRotationX(&rotation_x, m.rotation.x); // pitch
			D3DXMatrixRotationY(&rotation_y, m.rotation.y); // yaw
			D3DXMatrixRotationZ(&rotation_z, m.rotation.z); // roll
			mat_rotation = rotation_z * rotation_y * rotation_x; // combine rotations (order: Z * Y * X)

			D3DXMatrixTranslation(&mat_translation, m.origin.x, m.origin.y, m.origin.z);
			world = scale_matrix * mat_rotation * mat_translation;

			// set remix texture hash ~req. dxvk-runtime changes - not really needed
			dev->SetRenderState((D3DRENDERSTATETYPE)150, 100 + m.index);

			dev->SetTransform(D3DTS_WORLD, &world);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, mesh_verts, sizeof(vertex));
		}

		// #HACK: render single tri with rain_drop texture so remix loads the texture
		{
			const vertex mesh_verts[3] =
			{
				D3DXVECTOR3(-1.337f - 0.01f, -1.337f - 0.01f, 0), D3DCOLOR_XRGB(0, 0, 0), 0.0f, 0.0f,
				D3DXVECTOR3(1.337f + 0.01f, -1.337f - 0.01f, 0), D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0.0f,
				D3DXVECTOR3(1.337f + 0.01f,  1.337f + 0.01f, 0), D3DCOLOR_XRGB(0, 0, 0), 1.0f, 1.0f,
			};

			dev->SetTexture(0, tex_addons::rain_drop);
			dev->SetTransform(D3DTS_WORLD, &game::IDENTITY);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, mesh_verts, sizeof(vertex));
		}

		// restore
		dev->SetVertexShader(og_vs);
		dev->SetTexture(0, og_tex);
		dev->SetRenderState((D3DRENDERSTATETYPE)150, og_rs);
		dev->SetFVF(NULL);
		dev->SetTransform(D3DTS_WORLD, &game::IDENTITY);
	}

	// sound_events::on_start_sound_hk
	void remix_markers::on_sound_start(const std::uint32_t& hash, const std::string& sound_name)
	{
		auto& msettings = map_settings::get_map_settings();
		if (msettings.using_any_marker_sound_hash || msettings.using_any_marker_sound_name) 
		{
			for (auto& m : msettings.map_markers)
			{
				// check if marker uses sound trigger
				if (m.trigger_show.sound_hash && m.trigger_hide.sound_hash) {
					continue;
				}

				const bool is_show_trigger = m.trigger_show.sound_hash == hash || (!m.trigger_show.sound_name.empty() && sound_name.contains(m.trigger_show.sound_name));
				const bool is_hide_trigger = m.trigger_hide.sound_hash == hash || (!m.trigger_hide.sound_name.empty() && sound_name.contains(m.trigger_hide.sound_name));;

				if (m.trigger_always ||
					(is_show_trigger && !m.trigger_show.was_used) ||
					(is_hide_trigger && !m.trigger_hide.was_used))
				{
					if (is_show_trigger && m.is_hidden)
					{
						if (m.trigger_show.delay > 0.0f) {
							m.trigger_show.delay_start = true; // start delay logic
						}
						else {
							m.is_hidden = false; // set instantly
						}
						m.trigger_show.was_used = true;
					}

					if (is_hide_trigger && !m.is_hidden)
					{
						if (m.trigger_hide.delay > 0.0f) {
							m.trigger_hide.delay_start = true; // start delay logic
						}
						else {
							m.is_hidden = true; // set instantly
						}
						m.trigger_hide.was_used = true;
					}
				}
			}
		}
	}

	bool choreo_check_optionals(const map_settings::marker_trigger_s* t, const std::string_view& actor, const std::string_view& event, const std::string_view& param1)
	{
		if (t)
		{
			// check if opt. actor is defined and matches event actor
			if (!t->choreo_actor.empty() && !actor.contains(t->choreo_actor)) {
				return false;
			}

			// check if opt. event is defined and matches event string
			if (!t->choreo_event.empty() && !event.contains(t->choreo_event)) {
				return false;
			}

			// check if opt. param1 is defined and matches event param1
			if (!t->choreo_param1.empty() && !param1.contains(t->choreo_param1)) {
				return false;
			}
		}

		return true;
	}

	void remix_markers::on_event_start(const std::string_view& name, const std::string_view& actor, const std::string_view& event, const std::string_view& param1)
	{
		auto& msettings = map_settings::get_map_settings();
		if (msettings.using_any_marker_choreo)
		{
			for (auto& m : msettings.map_markers)
			{
				// check if marker uses choreo trigger
				if (m.trigger_show.choreo_name.empty() && m.trigger_hide.choreo_name.empty()) {
					continue;
				}

				const bool is_show_trigger = name.contains(m.trigger_show.choreo_name);
				const bool is_hide_trigger = name.contains(m.trigger_hide.choreo_name);

				if (m.trigger_always ||
					(is_show_trigger && !m.trigger_show.was_used) ||
					(is_hide_trigger && !m.trigger_hide.was_used))
				{
					if (is_show_trigger && m.is_hidden)
					{
						if (choreo_check_optionals(&m.trigger_show, actor, event, param1))
						{
							if (m.trigger_show.delay > 0.0f) {
								m.trigger_show.delay_start = true; // start delay logic
							}
							else {
								m.is_hidden = false; // set instantly
							}
							m.trigger_show.was_used = true;
						}
					}

					if (is_hide_trigger && !m.is_hidden)
					{
						if (choreo_check_optionals(&m.trigger_hide, actor, event, param1))
						{
							if (m.trigger_hide.delay > 0.0f) {
								m.trigger_hide.delay_start = true; // start delay logic
							}
							else {
								m.is_hidden = true; // set instantly
							}
							m.trigger_hide.was_used = true;
						}
					}
				}
			}
		}
	}

	void remix_markers::on_client_frame()
	{
		if (!interfaces::get()->m_engine->is_paused())
		{
			const auto globalv = interfaces::get()->m_globals;

			for (auto& m : map_settings::get_map_settings().map_markers)
			{
				if (m.trigger_show.delay_start)
				{
					if (m.trigger_show.delay <= m.trigger_show.delay_elapsed_time)
					{
						m.trigger_show.delay_start = false;
						m.trigger_show.delay_elapsed_time = 0.0f;
						m.is_hidden = false;
					}
					else {
						m.trigger_show.delay_elapsed_time += globalv->frametime;
					}
				}
				else if (m.trigger_hide.delay_start)
				{
					if (m.trigger_hide.delay <= m.trigger_hide.delay_elapsed_time)
					{
						m.trigger_hide.delay_start = false;
						m.trigger_hide.delay_elapsed_time = 0.0f;
						m.is_hidden = true;
					}
					else {
						m.trigger_hide.delay_elapsed_time += globalv->frametime;
					}
				}
			}
		}
	}

	remix_markers::remix_markers()
	{
		p_this = this;
	}
}
