#include "std_include.hpp"
#include "imgui_internal.h"
#include "imgui_helper.hpp"

namespace common::imgui
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

	void draw_blur(ImDrawList* draw_list)
	{
		const auto dev = game::get_d3d_device();

		const ImVec2 img_size = { (float)blur::backbuffer_width, (float)blur::backbuffer_height };

		draw_list->AddCallback(blur::begin_blur, dev);
		for (int i = 0; i < 8; ++i)
		{
			draw_list->AddCallback(blur::first_blur_pass, dev);
			draw_list->AddImage((ImTextureID)blur::texture, { 0.0f, 0.0f }, img_size);
			draw_list->AddCallback(blur::second_blur_pass, dev);
			draw_list->AddImage((ImTextureID)blur::texture, { 0.0f, 0.0f }, img_size);
		}

		draw_list->AddCallback(blur::end_blur, dev);
		draw_list->AddImageRounded((ImTextureID)blur::texture,
			{ 0.0f, 0.0f },
			img_size,
			{ 0.00f, 0.00f },
			{ 1.00f, 1.00f },
			IM_COL32(255, 255, 255, 255),
			0.f);
	}

	// Blur window background
	void draw_window_blur()
	{
		// only blur the window, clip everything else
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGui::PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

		draw_blur(ImGui::GetWindowDrawList());
		ImGui::PopClipRect();
	}

	// Blur entire background
	void draw_background_blur()
	{
		draw_blur(ImGui::GetBackgroundDrawList());
	}
}

namespace ImGui
{
	void Spacing(const float& x, const float& y) {
		Dummy(ImVec2(x, y));
	}

	void PushFont(common::imgui::font::FONTS font)
	{
		ImGuiIO& io = GetIO();

		if (io.Fonts->Fonts[font]) {
			PushFont(io.Fonts->Fonts[font]);
		}
		else {
			PushFont(GetDefaultFont());
		}
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

		const bool narrow = GetContentRegionAvail().x < 100.0f;

		PushID(id);

		if (!narrow) 
		{
			if (Button("-##Remove"))
			{
				common::imgui::get_and_remove_integers_from_set(buffer, set, buffer_len, true);
				main_module::trigger_vis_logic();
			}
			SetCursorScreenPos(ImVec2(GetItemRectMax().x, GetItemRectMin().y));
		}

		const auto spos = GetCursorScreenPos();
		
		SetNextItemWidth(GetContentRegionAvail().x - (narrow ? 0.0f : 40.0f));
		if (InputText("##Input", buffer, buffer_len, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll)) {
			common::imgui::get_and_add_integers_to_set(buffer, set, buffer_len, true);
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

		if (narrow) 
		{
			// next line :>
			Dummy(ImVec2(0, GetFrameHeight()));
			if (Button("-##Remove"))
			{
				common::imgui::get_and_remove_integers_from_set(buffer, set, buffer_len, true);
				main_module::trigger_vis_logic();
			}
		}

		SetCursorScreenPos(ImVec2(GetItemRectMax().x, GetItemRectMin().y));
		if (Button("+##Add")) {
			common::imgui::get_and_add_integers_to_set(buffer, set, buffer_len, true);
		}
		SetCursorScreenPos(ImVec2(GetItemRectMax().x + 1.0f, GetItemRectMin().y));
		if (Button("P##Picker"))
		{
			const auto c_str = utils::va("%d", flag == Widget_UnorderedSetModifierFlags_Leaf ? g_current_leaf : g_current_area);
			common::imgui::get_and_add_integers_to_set((char*)c_str, set);
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

	float CalcWidgetWidthForChild(const float label_width)
	{
		return GetContentRegionAvail().x - 4.0f - (label_width + GetStyle().ItemInnerSpacing.x + GetStyle().FramePadding.y);
	}

	void CenterText(const char* text, bool disabled)
	{
		SetCursorPosX(GetContentRegionAvail().x * 0.5f - CalcTextSize(text).x * 0.5f);
		if (!disabled) {
			TextUnformatted(text);
		}
		else {
			TextDisabled("%s", text);
		}
	}

	bool Widget_PrettyDragVec3(const char* ID, float* vec_in, bool show_label, const float speed, const float min, const float max,
		const char* x_str, const char* y_str, const char* z_str)
	{
		auto left_label_button = [](const char* label, const ImVec2& button_size, const ImVec4& text_color, const ImVec4& bg_color)
			{
				bool clicked = false;

				//PushFont(common::imgui::font::REGULAR);
				PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 5.0f));
				PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

				PushStyleColor(ImGuiCol_Text, text_color);
				PushStyleColor(ImGuiCol_Border, GetColorU32(ImGuiCol_Border));
				PushStyleColor(ImGuiCol_Button, bg_color); // GetColorU32(ImGuiCol_FrameBg)
				PushStyleColor(ImGuiCol_ButtonHovered, bg_color);

				if (ButtonEx(label, button_size, ImGuiButtonFlags_MouseButtonMiddle)) {
					clicked = true;
				}

				PopStyleColor(4);
				PopStyleVar(2);
				//PopFont();

				SameLine();
				SetCursorPosX(GetCursorPosX() - 1.0f);

				return clicked;
			};

		// ---------------
		bool dirty = false;

		ImGui::PushID(ID);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));

		const float line_height = GetFrameHeight();
		const auto  button_size = ImVec2(line_height - 2.0f, line_height);
		const float widget_spacing = 4.0f;

		//ImVec2 label_size = CalcTextSize(ID, nullptr, true);
		//label_size.x = ImMax(label_size.x, 80.0f);

		const float widget_width_horz = (GetContentRegionAvail().x - 3.0f * button_size.x - 2.0f * widget_spacing -
			(show_label ? /*label_size.x*/ 80.0f + GetStyle().ItemInnerSpacing.x + GetStyle().FramePadding.y : 0.0f)) * 0.333333f;

		/*const float widget_width_vert = (GetContentRegionAvail().x - 3.0f * button_size.x - 2.0f * widget_spacing -
			(show_label ? label_size.x + GetStyle().ItemInnerSpacing.x + GetStyle().FramePadding.y : 0.0f));*/

		const bool  narrow_window = GetWindowWidth() < 440.0f;

		// label if window width < min
		if (narrow_window) {
			ImGui::SeparatorText(ID);
		}

		// -------
		// -- X --

		if (left_label_button(x_str, button_size, ImVec4(0.84f, 0.55f, 0.53f, 1.0f), ImVec4(0.21f, 0.16f, 0.16f, 1.0f))) {
			vec_in[0] = 0.0f; dirty = true;
		}

		SetNextItemWidth(!narrow_window ? widget_width_horz : -1);
		if (DragFloat("##X", &vec_in[0], speed, min, max, "%.2f")) {
			dirty = true;
		}


		// -------
		// -- Y --

		if (!narrow_window) {
			SameLine(0, widget_spacing);
		}

		if (left_label_button(y_str, button_size, ImVec4(0.73f, 0.78f, 0.5f, 1.0f), ImVec4(0.17f, 0.18f, 0.15f, 1.0f))) {
			vec_in[1] = 0.0f; dirty = true;
		}

		SetNextItemWidth(!narrow_window ? widget_width_horz : -1);
		if (DragFloat("##Y", &vec_in[1], speed, min, max, "%.2f")) {
			dirty = true;
		}

		// -------
		// -- Z --

		if (!narrow_window) {
			SameLine(0, widget_spacing);
		}

		if (left_label_button(z_str, button_size, ImVec4(0.67f, 0.71f, 0.79f, 1.0f), ImVec4(0.18f, 0.21f, 0.23f, 1.0f))) {
			vec_in[2] = 0.0f; dirty = true;
		}

		SetNextItemWidth(!narrow_window ? widget_width_horz : -1);
		if (DragFloat("##Z", &vec_in[2], speed, min, max, "%.2f")) {
			dirty = true;
		}

		PopStyleVar();
		PopID();

		// right label if window width > min 
		if (!narrow_window)
		{
			SameLine(0, GetStyle().ItemInnerSpacing.x);
			TextUnformatted(ID);
		}

		return dirty;
	}

	/// Custom Collapsing Header with changeable height - Background color: ImGuiCol_HeaderActive 
	/// @param title_text	Label
	/// @param height		Header Height
	/// @param border_color Border Color
	/// @param default_open	True to collapse by default
	/// @param pre_spacing	8y Spacing in-front of Header
	/// @return				False if Header collapsed
	bool Widget_WrappedCollapsingHeader(const char* title_text, const float height, const ImVec4& border_color, const bool default_open, const bool pre_spacing)
	{
		if (pre_spacing) {
			Spacing(0.0f, 8.0f);
		}

		PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
		PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, height));
		PushStyleColor(ImGuiCol_Border, border_color);

		const auto open_flag = default_open ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None;

		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();
		ImGuiID storage_id = (g.NextItemData.HasFlags & ImGuiNextItemDataFlags_HasStorageID) ? g.NextItemData.StorageId : window->GetID(title_text);
		const bool is_open = TreeNodeUpdateNextOpen(storage_id, open_flag);
		const auto state = CollapsingHeader(title_text, open_flag | ImGuiTreeNodeFlags_SpanFullWidth);

		if (IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Middle, false)) {
			SetScrollHereY(0.0f);
		}

		// just toggled
		if (state && is_open != state) {
			//SetScrollHereY(0.0f); 
		}

		PopStyleColor();
		PopStyleVar(2);

		return state;
	}

	float Widget_ContainerWithCollapsingTitle(const char* child_name, const float child_height, const std::function<void()>& callback, const bool default_open, const char* icon, const ImVec4* bg_col, const ImVec4* border_col)
	{
		const std::string child_str = "[ "s + child_name + " ]"s;
		const float child_indent = 2.0f;

		const ImVec4 background_color = bg_col ? *bg_col : ImVec4(0.220f, 0.220f, 0.220f, 0.863f);
		const ImVec4 border_color = border_col ? *border_col : ImVec4(0.099f, 0.099f, 0.099f, 0.901f);

		const auto& style = GetStyle();

		const auto window = GetCurrentWindow();
		const auto min_x = window->WorkRect.Min.x - style.WindowPadding.x * 0.5f + 1.0f;
		const auto max_x = window->WorkRect.Max.x + style.WindowPadding.x * 0.5f - 1.0f;

		PushFont(common::imgui::font::BOLD);

		const auto spos_pre_header = GetCursorScreenPos();
		const auto expanded = Widget_WrappedCollapsingHeader(child_str.c_str(), 12.0f, border_color, default_open, false);

		PopFont();

		if (icon)
		{
			const auto spos_post_header = GetCursorScreenPos();
			const auto header_dims = GetItemRectSize();
			const auto icon_dims = CalcTextSize(icon);
			SetCursorScreenPos(spos_pre_header + ImVec2(header_dims.x - icon_dims.x - style.WindowPadding.x - 8.0f, header_dims.y * 0.5f - icon_dims.y * 0.5f));
			TextUnformatted(icon);
			SetCursorScreenPos(spos_post_header);
		}

		if (expanded)
		{
			const auto min = ImVec2(min_x, GetCursorScreenPos().y - style.ItemSpacing.y);
			const auto max = ImVec2(max_x, min.y + child_height);

			GetWindowDrawList()->AddRect(min + ImVec2(-1, 1), max + ImVec2(1, 1), ColorConvertFloat4ToU32(border_color), 10.0f, ImDrawFlags_RoundCornersBottom);
			GetWindowDrawList()->AddRectFilled(min, max, ColorConvertFloat4ToU32(background_color), 10.0f, ImDrawFlags_RoundCornersBottom);

			PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 4.0f));
			PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(6.0f, 8.0f));
			BeginChild(child_name, ImVec2(max.x - min.x - style.FramePadding.x - 2.0f, 0.0f),
				/*ImGuiChildFlags_Borders | */ ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeY);

			Indent(child_indent);
			PushClipRect(min, max, true);
			callback();
			PopClipRect();
			Unindent(child_indent);

			EndChild();
			PopStyleVar(2);
		}
		SetCursorScreenPos(GetCursorScreenPos() + ImVec2(0, expanded ? 36.0f : 8.0f));
		return GetItemRectSize().y + 6.0f/*- 28.0f*/;
	}


}