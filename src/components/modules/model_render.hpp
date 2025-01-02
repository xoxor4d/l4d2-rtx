#pragma once

namespace components
{
	class prim_fvf_context {
	public:

		// retrieve information about the current pass - returns true if successful
		bool get_info_for_pass(IShaderAPIDX8* shaderapi)
		{
			if (shaderapi)
			{
				shaderapi->vtbl->GetBufferedState(shaderapi, nullptr, &info.buffer_state);

				if (info.material = shaderapi->vtbl->GetBoundMaterial(shaderapi, nullptr); 
					info.material)
				{
					info.material_name = info.material->vftable->GetName(info.material);
					info.shader_name = info.material->vftable->GetShaderName(info.material);

					return true;
				}
			}

			return false;
		}

		// set texture 0 transform
		void set_texture_transform(IDirect3DDevice9* device, const D3DXMATRIX* matrix)
		{
			if (matrix)
			{
				device->SetTransform(D3DTS_TEXTURE0, matrix);
				tex0_transform_set = true;
			}
		}

		// save vertex shader
		void save_vs(IDirect3DDevice9* device)
		{
			device->GetVertexShader(&vs_);
			vs_set = true;
		}

		// save texture at stage 0 or 1
		void save_texture(IDirect3DDevice9* device, const bool stage)
		{
			if (!stage)
			{
#if DEBUG
				if (tex0_set) {
					OutputDebugStringA("save_texture:: tex0 was already saved\n"); return;
				}
#endif

				device->GetTexture(0, &tex0_);
				tex0_set = true;
			}
			else
			{
#if DEBUG
				if (tex1_set) {
					OutputDebugStringA("save_texture:: tex1 was already saved\n"); return;
				}
#endif

				device->GetTexture(1, &tex1_);
				tex1_set = true;
			}
		}

		// save render state (e.g. D3DRS_TEXTUREFACTOR)
		void save_rs(IDirect3DDevice9* device, const D3DRENDERSTATETYPE& state)
		{
			if (saved_render_state_.contains(state)) {
				return;
			}

			DWORD temp;
			device->GetRenderState(state, &temp);
			saved_render_state_[state] = temp;
		}

		// save sampler state (D3DSAMPLERSTATETYPE)
		void save_ss(IDirect3DDevice9* device, const D3DSAMPLERSTATETYPE& state)
		{
			if (saved_sampler_state_.contains(state)) {
				return;
			}

			DWORD temp;
			device->GetSamplerState(0, state, &temp);
			saved_sampler_state_[state] = temp;
		}

		// save texture stage 0 state (e.g. D3DTSS_ALPHAARG1)
		void save_tss(IDirect3DDevice9* device, const D3DTEXTURESTAGESTATETYPE& type)
		{
			if (saved_texture_stage_state_.contains(type)) {
				return;
			}

			DWORD temp;
			device->GetTextureStageState(0, type, &temp);
			saved_texture_stage_state_[type] = temp;
		}

		// save D3DTS_VIEW
		void save_view_transform(IDirect3DDevice9* device)
		{
			device->GetTransform(D3DTS_VIEW, &view_transform_);
			view_transform_set_ = true;
		}

		// save D3DTS_PROJECTION
		void save_projection_transform(IDirect3DDevice9* device)
		{
			device->GetTransform(D3DTS_PROJECTION, &projection_transform_);
			projection_transform_set_ = true;
		}

		//// save steamsource data
		//void save_streamsource_data(IDirect3DVertexBuffer9* buffer, UINT offset, UINT stride)
		//{
		//	streamsource_ = buffer;
		//	streamsource_offset_ = offset;
		//	streamsource_stride_ = stride;
		//}

		// restore vertex shader
		void restore_vs(IDirect3DDevice9* device)
		{
			if (vs_set)
			{
				device->SetVertexShader(vs_);
				vs_set = false;
			}
		}

		// restore texture at stage 0 or 1
		void restore_texture(IDirect3DDevice9* device, const bool stage)
		{
			if (!stage)
			{
				if (tex0_set)
				{
					device->SetTexture(0, tex0_);
					tex0_set = false;
				}
			}
			else
			{
				if (tex1_set)
				{
					device->SetTexture(1, tex1_);
					tex1_set = false;
				}
			}
		}

		// restore a specific render state (e.g. D3DRS_TEXTUREFACTOR)
		void restore_render_state(IDirect3DDevice9* device, const D3DRENDERSTATETYPE& state)
		{
			if (saved_render_state_.contains(state)) {
				device->SetRenderState(state, saved_render_state_[state]);
			}
		}

		// restore a specific sampler state (D3DSAMPLERSTATETYPE)
		void restore_sampler_state(IDirect3DDevice9* device, const D3DSAMPLERSTATETYPE& state)
		{
			if (saved_sampler_state_.contains(state)) {
				device->SetSamplerState(0, state, saved_sampler_state_[state]);
			}
		}

		// restore a specific texture stage 0 state (e.g. D3DTSS_ALPHAARG1)
		void restore_texture_stage_state(IDirect3DDevice9* device, const D3DTEXTURESTAGESTATETYPE& type)
		{
			if (saved_texture_stage_state_.contains(type)) {
				device->SetTextureStageState(0, type, saved_texture_stage_state_[type]);
			}
		}

		// restore texture 0 transform to identity
		void restore_texture_transform(IDirect3DDevice9* device)
		{
			device->SetTransform(D3DTS_TEXTURE0, &game::IDENTITY);
			tex0_transform_set = false;
		}

		// restore saved D3DTS_VIEW
		void restore_view_transform(IDirect3DDevice9* device)
		{
			if (view_transform_set_)
			{
				device->SetTransform(D3DTS_VIEW, &view_transform_);
				view_transform_set_ = false;
			}
		}

		// restore saved D3DTS_PROJECTION
		void restore_projection_transform(IDirect3DDevice9* device)
		{
			if (projection_transform_set_)
			{
				device->SetTransform(D3DTS_PROJECTION, &projection_transform_);
				projection_transform_set_ = false;
			}
		}

		// restore all changes
		void restore_all(IDirect3DDevice9* device)
		{
			restore_vs(device);
			restore_texture(device, 0);
			restore_texture(device, 1);
			restore_texture_transform(device);
			restore_view_transform(device);
			restore_projection_transform(device);

			for (auto& rs : saved_render_state_) {
				device->SetRenderState(rs.first, rs.second);
			}

			for (auto& ss : saved_sampler_state_) {
				device->SetSamplerState(0, ss.first, ss.second);
			}

			for (auto& tss : saved_texture_stage_state_) {
				device->SetTextureStageState(0, tss.first, tss.second);
			}
		}

		// reset the stored context data
		void reset_context()
		{
			vs_ = nullptr; vs_set = false;
			tex0_ = nullptr; tex0_set = false;
			tex1_ = nullptr; tex1_set = false;
			tex0_transform_set = false;
			view_transform_set_ = false;
			projection_transform_set_ = false;
			saved_render_state_.clear();
			saved_sampler_state_.clear();
			saved_texture_stage_state_.clear();
			modifiers.reset();
			info.reset();
		}

		struct modifiers_s
		{
			bool do_not_render = false;
			bool with_high_gamma = false;
			bool as_sky = false;
			bool as_water = false;
			bool as_transport_beam = false;
			bool as_portalgun_pickup_beam = false;
			Vector2D emancipation_offset = {};
			Vector2D emancipation_scale = { 1.0f, 1.0f };
			float emancipation_color_scale = 1.0f;
			bool dual_render_with_basetexture2 = false; // render prim a second time with tex2 set as tex1
			bool dual_render_with_specified_texture = false; // render prim a second time with tex defined in 'dual_render_texture'
			bool dual_render_with_specified_texture_blend_add = false; // renders second prim using blend mode ADD
			IDirect3DBaseTexture9* dual_render_texture = nullptr;

			void reset()
			{
				do_not_render = false;
				with_high_gamma = false;
				as_sky = false;
				as_water = false;
				as_transport_beam = false;

				as_portalgun_pickup_beam = false;
				emancipation_offset = { 0.0f, 0.0f };
				emancipation_scale = { 1.0f, 1.0f };
				emancipation_color_scale = 1.0f;
				dual_render_with_basetexture2 = false;
				dual_render_with_specified_texture = false;
				dual_render_texture = nullptr;
			}
		};

		// special handlers for the next prim/s
		modifiers_s modifiers;

		struct info_s
		{
			IMaterialInternal* material = nullptr;
			std::string_view material_name;
			std::string_view shader_name;
			BufferedState_t buffer_state {};

			void reset()
			{
				material = nullptr;
				material_name = "";
				shader_name = "";
				memset(&buffer_state, 0, sizeof(BufferedState_t));
			}
		};

		// holds information about the current pass
		// use 'get_info_for_pass()' to populate struct
		info_s info;

		// constructor for singleton
		prim_fvf_context() = default;

	private:
		// Render states to save
		IDirect3DVertexShader9* vs_ = nullptr;
		IDirect3DBaseTexture9* tex0_ = nullptr;
		IDirect3DBaseTexture9* tex1_ = nullptr;
		bool vs_set = false;
		bool tex0_set = false;
		bool tex1_set = false;
		bool tex0_transform_set = false;
		D3DMATRIX view_transform_ = {};
		D3DMATRIX projection_transform_ = {};
		bool view_transform_set_ = false;
		bool projection_transform_set_ = false;

		// store saved render states (with the type as the key)
		std::unordered_map<D3DRENDERSTATETYPE, DWORD> saved_render_state_;

		// store saved render states (with the type as the key)
		std::unordered_map<D3DSAMPLERSTATETYPE, DWORD> saved_sampler_state_;

		// store saved texture stage states (with type as the key)
		std::unordered_map<D3DTEXTURESTAGESTATETYPE, DWORD> saved_texture_stage_state_;
	};

	namespace tex_addons
	{
		extern LPDIRECT3DTEXTURE9 glass_shards;
		extern LPDIRECT3DTEXTURE9 black;
		extern LPDIRECT3DTEXTURE9 white;
	}

	class model_render : public component
	{
	public:
		model_render();
		~model_render() = default;

		static inline model_render* p_this = nullptr;
		static model_render* get() { return p_this; }

		static void init_texture_addons(bool release = false);
		static inline prim_fvf_context primctx {};

		bool m_drew_hud = false;
	};
}
