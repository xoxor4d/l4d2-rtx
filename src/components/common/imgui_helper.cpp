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
	void Spacing(const float& x, const float& y) {
		ImGui::Dummy(ImVec2(x, y));
	}

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

	// #

	void Style_DeleteButtonPush()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.05f, 0.05f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.68f, 0.05f, 0.05f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 1.0f));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	}

	void Style_DeleteButtonPop()
	{
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(4);
	}

	// #

	void Style_ColorButtonPush(const ImVec4& base_color, bool black_border)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, base_color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, base_color * ImVec4(1.4f, 1.4f, 1.4f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, base_color * ImVec4(1.8f, 1.8f, 1.8f, 1.0f));

		ImGui::PushStyleColor(ImGuiCol_Border, black_border 
			? ImVec4(0, 0, 0, 1.0f) 
			: ImGui::GetStyleColorVec4(ImGuiCol_Border));
	}

	void Style_ColorButtonPop() {
		ImGui::PopStyleColor(4);
	}

	// #

	void Style_InvisibleSelectorPush() {
		PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
		PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
	}

	void Style_InvisibleSelectorPop() {
		PopStyleColor(2);
	}

	// #

	/// Custom Collapsing Header with changeable height - Background color: ImGuiCol_HeaderActive 
	/// @param title_text	Label
	/// @param height		Header Height
	/// @param border_color Border Color
	/// @param pre_spacing	8y Spacing in-front of Header
	/// @return				False if Header collapsed
	bool Widget_WrappedCollapsingHeader(const char* title_text, const float height, const ImVec4& border_color, bool pre_spacing)
	{
		if (pre_spacing) {
			Spacing(0.0f, 8.0f);
		}

		PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
		PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, height));
		PushStyleColor(ImGuiCol_Border, border_color);

		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();
		ImGuiID storage_id = (g.NextItemData.HasFlags & ImGuiNextItemDataFlags_HasStorageID) ? g.NextItemData.StorageId : window->GetID(title_text);
		const bool is_open = TreeNodeUpdateNextOpen(storage_id, ImGuiTreeNodeFlags_DefaultOpen);
		const auto state = CollapsingHeader(title_text, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth);

		// just toggled
		if (state && is_open != state) {
			ImGui::SetScrollHereY(0.0f); 
		}

		PopStyleColor();
		PopStyleVar(2);

		return state;
	}

	float Widget_ContainerWithTitleRounded(const char* child_name, const float child_height, const std::function<void()>& callback, const ImVec4* bg_col, const ImVec4* border_col)
	{
		const std::string child_str = "[ "s + child_name + " ]"s;
		const float child_indent = 2.0f;

		const ImVec4 background_color = bg_col ? *bg_col : ImVec4(0.220f, 0.220f, 0.220f, 0.863f);
		const ImVec4 border_color = border_col ? *border_col : ImVec4(0.099f, 0.099f, 0.099f, 0.901f);
		
		const auto window = GetCurrentWindow();
		const auto min_x = window->WorkRect.Min.x - GetStyle().WindowPadding.x * 0.5f + 1.0f;
		const auto max_x = window->WorkRect.Max.x + GetStyle().WindowPadding.x * 0.5f - 1.0f;

		BeginGroup();
		const auto expanded = Widget_WrappedCollapsingHeader(child_str.c_str(), 12.0f, border_color, false);
		if (expanded)
		{
			const auto min = ImVec2(min_x, GetCursorScreenPos().y - GetStyle().ItemSpacing.y);
			const auto max = ImVec2(max_x, min.y + child_height);

			GetWindowDrawList()->AddRect(min + ImVec2(-1, 1), max + ImVec2(1, 1), ColorConvertFloat4ToU32(border_color), 10.0f, ImDrawFlags_RoundCornersBottom);
			GetWindowDrawList()->AddRectFilled(min, max, ColorConvertFloat4ToU32(background_color), 10.0f, ImDrawFlags_RoundCornersBottom);

			PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 4.0f));
			BeginChild(child_name, ImVec2(max.x - min.x - GetStyle().FramePadding.x - 2.0f, 0.0f), 
				/*ImGuiChildFlags_Borders | */ ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeY);

			Indent(child_indent);
			PushClipRect(min, max, true);
			callback();
			PopClipRect();
			Unindent(child_indent);

			EndChild();
			PopStyleVar();
		}
		EndGroup();
		SetCursorScreenPos(GetCursorScreenPos() + ImVec2(0, expanded ? 28.0f : 16.0f));
		return GetItemRectSize().y - 28.0f;
	}


}