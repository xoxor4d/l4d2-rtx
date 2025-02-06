#include "std_include.hpp"
#include "imgui_internal.h"
#include "imgui_helper.hpp"

namespace common
{
	void get_and_add_integers_to_set(char* str, std::unordered_set<std::uint32_t>& set, const std::uint32_t& buf_len, const bool clear_buf)
	{
		std::vector<int> temp_vec;
		utils::extract_integer_words(str, temp_vec, true);
		set.insert(temp_vec.begin(), temp_vec.end());

		if (clear_buf) {
			memset(str, 0, buf_len);
		}
	}

	void get_and_remove_integers_from_set(char* str, std::unordered_set<std::uint32_t>& set, const std::uint32_t& buf_len, const bool clear_buf)
	{
		std::vector<int> temp_vec;
		utils::extract_integer_words(str, temp_vec, true);

		for (const auto& v : temp_vec) {
			set.erase(v);
		}

		if (clear_buf) {
			memset(str, 0, buf_len);
		}
	}

	namespace blur
	{
		namespace
		{
			IDirect3DSurface9* rt_backup = nullptr;
			IDirect3DPixelShader9* shader_x = nullptr;
			IDirect3DPixelShader9* shader_y = nullptr;
			IDirect3DTexture9* texture = nullptr;
			std::uint32_t backbuffer_width = 0u;
			std::uint32_t backbuffer_height = 0u;

			void begin_blur([[maybe_unused]] const ImDrawList* parent_list, const ImDrawCmd* cmd)
			{
				const auto device = static_cast<IDirect3DDevice9*>(cmd->UserCallbackData);

				if (!shader_x) {
					device->CreatePixelShader(reinterpret_cast<const DWORD*>(blur_x.data()), &shader_x);
				}

				if (!shader_y) {
					device->CreatePixelShader(reinterpret_cast<const DWORD*>(blur_y.data()), &shader_y);
				}

				IDirect3DSurface9* backBuffer;
				device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);

				D3DSURFACE_DESC desc;
				backBuffer->GetDesc(&desc);

				if (backbuffer_width != desc.Width || backbuffer_height != desc.Height)
				{
					if (texture) {
						texture->Release();
					}

					backbuffer_width = desc.Width;
					backbuffer_height = desc.Height;
					device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr);
				}

				device->GetRenderTarget(0, &rt_backup);

				{
					IDirect3DSurface9* surface;
					texture->GetSurfaceLevel(0, &surface);
					device->StretchRect(backBuffer, nullptr, surface, nullptr, D3DTEXF_NONE);
					device->SetRenderTarget(0, surface);
					surface->Release();
				}

				backBuffer->Release();

				device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
				device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			}

			void first_blur_pass([[maybe_unused]] const ImDrawList* parent_list, const ImDrawCmd* cmd)
			{
				const auto device = static_cast<IDirect3DDevice9*>(cmd->UserCallbackData);

				device->SetPixelShader(shader_x);
				const float params[4] = { 1.0f / (float)backbuffer_width };
				device->SetPixelShaderConstantF(0, params, 1);
			}

			void second_blur_pass([[maybe_unused]] const ImDrawList* parent_list, const ImDrawCmd* cmd)
			{
				const auto device = static_cast<IDirect3DDevice9*>(cmd->UserCallbackData);

				device->SetPixelShader(shader_y);
				const float params[4] = { 1.0f / (float)backbuffer_height };
				device->SetPixelShaderConstantF(0, params, 1);
			}

			void end_blur([[maybe_unused]] const ImDrawList* parent_list, const ImDrawCmd* cmd)
			{
				const auto device = static_cast<IDirect3DDevice9*>(cmd->UserCallbackData);

				device->SetRenderTarget(0, rt_backup);
				rt_backup->Release();

				device->SetPixelShader(nullptr);
				device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			}
		}
	}

	void draw_background_blur(ImDrawList* drawList, IDirect3DDevice9* device)
	{
		drawList->AddCallback(blur::begin_blur, device);

		for (int i = 0; i < 8; ++i) 
		{
			drawList->AddCallback(blur::first_blur_pass, device);
			drawList->AddImage((ImTextureID)blur::texture, { 0.0f, 0.0f }, { (float)blur::backbuffer_width, (float)blur::backbuffer_height });
			drawList->AddCallback(blur::second_blur_pass, device);
			drawList->AddImage((ImTextureID)blur::texture, { 0.0f, 0.0f }, { (float)blur::backbuffer_width, (float)blur::backbuffer_height });
		}

		drawList->AddCallback(blur::end_blur, device);
		drawList->AddImageRounded((ImTextureID)blur::texture, 
			{ 0.0f, 0.0f }, 
			{ (float)blur::backbuffer_width, (float)blur::backbuffer_height }, 
			{ 0.01f, 0.01f },
			{ 0.99f, 0.99f }, 
			IM_COL32(200, 200, 200, 255),
			0.f);
	}
}

namespace ImGui
{
	// Draw wrapped text containing all unsigned integers from the provided unordered_set
	void TextWrapped_IntegersFromUnorderedSet(const std::unordered_set<std::uint32_t>& set)
	{
		std::string arr_str;
		for (auto it = set.begin(); it != set.end(); ++it)
		{
			if (it != set.begin()) {
				arr_str += ", ";
			} arr_str += std::to_string(*it);
		}
		if (arr_str.empty()) {
			arr_str = "// empty";
		}
		TextWrapped("%s", arr_str.c_str());
	}

	void Widget_UnorderedSetModifier(const char* id, Widget_UnorderedSetModifierFlags flag, std::unordered_set<std::uint32_t>& set, char* buffer, std::uint32_t buffer_len)
	{
		const auto txt_input_full = "Add/Remove..";
		const auto txt_input_full_width = CalcTextSize(txt_input_full).x;
		const auto txt_input_min = "...";
		const auto txt_input_min_width = CalcTextSize(txt_input_min).x;

		PushID(id);
		if (Button("-##Remove"))
		{
			common::get_and_remove_integers_from_set(buffer, set, buffer_len, true);
			main_module::trigger_vis_logic();
		}

		SetCursorScreenPos(ImVec2(GetItemRectMax().x, GetItemRectMin().y));
		const auto spos = GetCursorScreenPos();
		SetNextItemWidth(GetContentRegionAvail().x - 40.0f);
		if (InputText("##Input", buffer, buffer_len, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll)) {
			common::get_and_add_integers_to_set(buffer, set, buffer_len, true);
		}

		SetCursorScreenPos(spos);
		if (!buffer[0])
		{
			const auto min_content_area_width = GetContentRegionAvail().x - 40.0f;
			ImVec2 pos = GetCursorScreenPos() + ImVec2(8.0f, CalcTextSize("A").y * 0.45f);
			if (min_content_area_width > txt_input_full_width) {
				GetWindowDrawList()->AddText(pos, GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
			}
			else if (min_content_area_width > txt_input_min_width) {
				GetWindowDrawList()->AddText(pos, GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
			}
		}

		SetCursorScreenPos(ImVec2(GetItemRectMax().x, GetItemRectMin().y));
		if (Button("+##Add")) {
			common::get_and_add_integers_to_set(buffer, set, buffer_len, true);
		}
		SetCursorScreenPos(ImVec2(GetItemRectMax().x + 1.0f, GetItemRectMin().y));
		if (Button("P##Picker"))
		{
			const auto c_str = utils::va("%d", flag == Widget_UnorderedSetModifierFlags_Leaf ? g_current_leaf : g_current_area);
			common::get_and_add_integers_to_set((char*)c_str, set);
			main_module::trigger_vis_logic();
		}
		SetItemTooltip(flag == Widget_UnorderedSetModifierFlags_Leaf ? "Pick Current Leaf" : "Pick Current Area");
		PopID();
	}
}