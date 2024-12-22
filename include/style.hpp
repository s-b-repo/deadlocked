#include <imgui.h>

void Style() {
    ImGuiStyle &style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6000000238418579f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 8.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ChildRounding = 8.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 8.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 8.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 5.0f);
    style.ItemInnerSpacing = ImVec2(5.0f, 5.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 20.0f;
    style.ColumnsMinSpacing = 5.0f;
    style.ScrollbarSize = 16.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabMinSize = 8.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;
    style.TabBorderSize = 0.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.7058823704719543f, 0.7058823704719543f, 0.7058823704719543f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1176470592617989f, 0.1176470592617989f, 0.1568627506494522f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2980392277240753f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.7058823704719543f, 0.7058823704719543f, 0.7058823704719543f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1725490242242813f, 0.1725490242242813f, 0.2313725501298904f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.3686274588108063f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.3686274588108063f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] =
        ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.7058823704719543f, 0.7058823704719543f, 0.7058823704719543f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.1725490242242813f, 0.1725490242242813f, 0.2313725501298904f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.3686274588108063f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.1725490242242813f, 0.1725490242242813f, 0.2313725501298904f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.3686274588108063f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.3686274588108063f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.7058823704719543f, 0.7058823704719543f, 0.7058823704719543f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_Tab] =
        ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2980392277240753f, 0.9882352948188782f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_TabActive] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 0.9803921580314636f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2980392277240753f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] =
        ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.7058823704719543f, 0.7058823704719543f, 0.7058823704719543f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] =
        ImVec4(0.9411764740943909f, 0.5490196347236633f, 0.3529411852359772f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.9411764740943909f, 0.7843137383460999f, 0.4705882370471954f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] =
        ImVec4(0.9411764740943909f, 0.5490196347236633f, 0.3529411852359772f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.3686274588108063f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] =
        ImVec4(0.7058823704719543f, 0.7058823704719543f, 0.7058823704719543f, 0.4980392158031464f);
    style.Colors[ImGuiCol_TableBorderLight] =
        ImVec4(0.7058823704719543f, 0.7058823704719543f, 0.7058823704719543f, 0.4980392158031464f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.1725490242242813f, 0.1725490242242813f, 0.2313725501298904f, 1.0f);
    style.Colors[ImGuiCol_TextSelectedBg] =
        ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.3921568691730499f, 0.5882353186607361f, 0.9411764740943909f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.4980392158031464f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.3137255012989044f);
}
