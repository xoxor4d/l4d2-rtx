#pragma once

namespace components
{
	namespace cmd
	{
		extern bool model_info_vis;
		extern bool unbake_model_info_vis;
		extern std::uint32_t ms_unbake_info;
	}

	extern std::vector<Vector> g_sunoverlay_color;

	namespace tbl_hk::model_renderer
	{
		inline utils::vtable table;
		inline struct IVModelRender* _interface = nullptr;

		namespace DrawModelExecute
		{
			constexpr uint32_t index = 19u;
			using FN = void(__fastcall*)(void*, void*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
			void __fastcall Detour(void* ecx, void* edx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
		}
	}

	enum class RemixModifier : std::uint16_t
	{
		None = 0,
		InfectedShader = 1 << 0,
		EmissiveScalar = 1 << 1,
		Free02 = 1 << 2,
		Free03 = 1 << 3,
		Free04 = 1 << 4,
		Free05 = 1 << 5,
		Free06 = 1 << 6,
		Free07 = 1 << 7,
		Free08 = 1 << 8,
		Free09 = 1 << 9,
		Free10 = 1 << 10,
		Free11 = 1 << 11,
		Free12 = 1 << 12,
		Free13 = 1 << 13,
		Free14 = 1 << 14,
		Free15 = 1 << 15,
	};

	enum remix_custom_rs
	{
		RS_42_TEXTURE_CATEGORY = 42,
		RS_149_REMIX_MODIFIER = 149,
		RS_150_TEXTURE_HASH = 150,
		RS_169_EMISSIVE_SCALE = 169,
		RS_177_INFECTED_SHEET_UV = 177, // uint16 + uint16
		RS_196_INFECTED_SKIN_GRAD = 196, // uint32 -> float
		RS_197_INFECTED_GRAD_SELECT = 197, // uint16 + uint16
		RS_210_PARAMS_PACKED = 210,
		RS_211_INFECTED_NORMAL_ROUGH_BOOST = 211, // uint16 + uint16
		RS_212_FREE = 212,
		RS_213_FREE = 213,
		RS_214_FREE = 214,
		RS_215_FREE = 215,
		RS_216_FREE = 216,
		RS_217_FREE = 217,
		RS_218_FREE = 218,
		RS_219_FREE = 219,
		RS_220_HASH_MODIFIER_SEED = 220,
	};

	enum remix_hash_seed
	{
		ZERO_EMISSION_SEED = 1337,
	};

	constexpr RemixModifier operator|(RemixModifier lhs, RemixModifier rhs) {
		return static_cast<RemixModifier>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
	}

	constexpr RemixModifier& operator|=(RemixModifier& lhs, RemixModifier rhs) {
		lhs = static_cast<RemixModifier>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
		return lhs;
	}

	constexpr RemixModifier operator&(RemixModifier lhs, RemixModifier rhs) {
		return static_cast<RemixModifier>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
	}

	constexpr RemixModifier& operator&=(RemixModifier& lhs, RemixModifier rhs) {
		lhs = static_cast<RemixModifier>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
		return lhs;
	}

	constexpr RemixModifier operator~(RemixModifier e) {
		return static_cast<RemixModifier>(~static_cast<std::uint32_t>(e));
	}

	// can't use remixapi_InstanceCategoryFlags as they don't match up with InstanceCategories
	enum class InstanceCategories : uint32_t
	{
		WorldUI = 1 << 0,
		WorldMatte = 1 << 1,
		Sky = 1 << 2,
		Ignore = 1 << 3,
		IgnoreLights = 1 << 4,
		IgnoreAntiCulling = 1 << 5,
		IgnoreMotionBlur = 1 << 6,
		IgnoreOpacityMicromap = 1 << 7,
		IgnoreAlphaChannel = 1 << 8,
		Hidden = 1 << 9,
		Particle = 1 << 10,
		Beam = 1 << 11,
		DecalStatic = 1 << 12,
		DecalDynamic = 1 << 13,
		DecalSingleOffset = 1 << 14,
		DecalNoOffset = 1 << 15,
		AlphaBlendToCutout = 1 << 16,
		Terrain = 1 << 17,
		AnimatedWater = 1 << 18,
		ThirdPersonPlayerModel = 1 << 19,
		ThirdPersonPlayerBody = 1 << 20,
		IgnoreBakedLighting = 1 << 21,
		IgnoreTransparencyLayer = 1 << 22,
		ParticleEmitter = 1 << 23,
		DisableBackfaceCulling = 1 << 24,
		Count = 24,
		None = 0u
	};

	constexpr InstanceCategories operator|(InstanceCategories lhs, InstanceCategories rhs) {
		return static_cast<InstanceCategories>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
	}

	constexpr InstanceCategories& operator|=(InstanceCategories& lhs, InstanceCategories rhs) {
		lhs = static_cast<InstanceCategories>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
		return lhs;
	}

	constexpr InstanceCategories operator&(InstanceCategories lhs, InstanceCategories rhs) {
		return static_cast<InstanceCategories>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
	}

	constexpr InstanceCategories& operator&=(InstanceCategories& lhs, InstanceCategories rhs) {
		lhs = static_cast<InstanceCategories>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
		return lhs;
	}

	constexpr InstanceCategories operator~(InstanceCategories e) {
		return static_cast<InstanceCategories>(~static_cast<std::uint32_t>(e));
	}

	class prim_fvf_context
	{
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
		bool save_rs(IDirect3DDevice9* device, const D3DRENDERSTATETYPE& state)
		{
			if (saved_render_state_.contains(state)) {
				return false;
			}

			DWORD temp;
			device->GetRenderState(state, &temp);
			saved_render_state_[state] = temp;
			return true;
		}

		bool save_rs(IDirect3DDevice9* device, const uint32_t& state)
		{
			return save_rs(device, (D3DRENDERSTATETYPE)state);
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

		// save D3DTS_WORLD
		void save_world_transform(IDirect3DDevice9* device, D3DXMATRIX* other = nullptr)
		{
			if (other) {
				memcpy_s(&world_transform_, sizeof(D3DXMATRIX), other, sizeof(D3DMATRIX));
			} else {
				device->GetTransform(D3DTS_WORLD, &world_transform_);
			}
			world_transform_set_ = true;
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

		// restore saved D3DTS_WORLD
		void restore_world_transform(IDirect3DDevice9* device)
		{
			if (world_transform_set_)
			{
				device->SetTransform(D3DTS_WORLD, &world_transform_);
				world_transform_set_ = false;
			}
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
			restore_world_transform(device);
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
			world_transform_set_ = false;
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

			float og_mesh_z_offset = 0.0f;

			bool as_temp_unused = false;
			bool dual_render_with_basetexture2 = false; // render prim a second time with tex2 set as tex1
			bool dual_render_with_specified_texture = false; // render prim a second time with tex defined in 'dual_render_texture'
			bool dual_render_with_specified_texture_blend_add = false; // renders second prim using blend mode ADD
			IDirect3DBaseTexture9* dual_render_texture = nullptr;
			float dual_render_texture_z_offset = 0.0f;

			InstanceCategories remix_instance_categories = InstanceCategories::None;
			RemixModifier remix_modifier = RemixModifier::None;

			void reset()
			{
				do_not_render = false;
				with_high_gamma = false;
				as_sky = false;
				as_water = false;
				og_mesh_z_offset = 0.0f;

				as_temp_unused = false;
				dual_render_with_basetexture2 = false;
				dual_render_with_specified_texture = false;
				dual_render_texture = nullptr;
				dual_render_texture_z_offset = 0.0f;

				remix_instance_categories = InstanceCategories::None;
				remix_modifier = RemixModifier::None;
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
		D3DMATRIX world_transform_ = {};
		D3DMATRIX view_transform_ = {};
		D3DMATRIX projection_transform_ = {};
		bool world_transform_set_ = false;
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
		extern LPDIRECT3DTEXTURE9 rain_drop;
		extern LPDIRECT3DTEXTURE9 black;
		extern LPDIRECT3DTEXTURE9 white;
		extern LPDIRECT3DTEXTURE9 berry;
	}

	class model_render final : public loader::component_module
	{
	public:
		model_render();
		~model_render() = default;

		static inline model_render* p_this = nullptr;
		static model_render* get() { return p_this; }

		static void xo_debug_toggle_model_info_fn();
		//static void draw_nocull_markers();
		static void on_present();

		static void init_texture_addons(bool release = false);

		static void set_remix_modifier(IDirect3DDevice9* dev, RemixModifier mod, bool remove_mod = false);
		static void set_remix_emissive_intensity(IDirect3DDevice9* dev, float intensity);
		static void set_remix_texture_categories(IDirect3DDevice9* dev, const InstanceCategories& cat, bool remove_category = false);
		static void set_remix_texture_hash(IDirect3DDevice9* dev, const std::uint32_t& hash);
		static void set_remix_texture_hash_modifier(IDirect3DDevice9* dev, const std::uint32_t& seed);

		static inline prim_fvf_context primctx {};

		bool m_drew_model = false;
	};
}
