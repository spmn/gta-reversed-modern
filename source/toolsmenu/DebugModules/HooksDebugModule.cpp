#include "StdInc.h"

#include <reversiblehooks/RootHookCategory.h>
#include "HooksDebugModule.h"
#include "Utility.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <TristateCheckbox.h>
#include <string>
#include <ranges>
#include <optional>

namespace RH = ReversibleHooks;
namespace rng = std::ranges;
using namespace ImGui;

// Namespace for the hook filter
// It is intentionally not a class/struct (As it's a singleton and static classes are stupid)
namespace HookFilter {
    // If either of these separators is changed make sure to alter the regex as well.
    // Also all the comments :D
    static constexpr std::string_view NAMESPACE_SEP{ "/" };
    static constexpr std::string_view HOOKNAME_SEP{ "::" };

    // Maybe one day.. For now it's fine
    // Regex used for checking if the input is valid, see `ProcessImGui` in this class.
    //static inline const std::regex INPUT_VALID_REGEX{
    //    R"r(\/?(?:\w+\/)*\w+(?:\:{2}\w*|\:))r", // We're using in r-string here to avoid the need of escaping all `\`
    //    std::regex::optimize | std::regex::icase
    //}; 

    // Used as char buffer for the ImGUI input
    std::string m_input{};          

    // Are the string checks case sensitive?
    // It is used, but there's no GUI to change it for now. It is case-insensitive by defult (false).
    bool m_caseSensitive{};

    // Contains all the tokens on the left side split by `NAMESPACE_SEP` of the input split by `HOOKNAME_SEP`
    // Eg `m_input` => content:
    // - `Name/Space/` => `Name`, `Space`, `` (<= empty string)
    // - `Name/Space/::` => -||- (Same as the above example)
    // - `/` - `` (empty string) - Indicates root namespace (See `IsRelativeToRootNamespace`)
    // - `///` - 4 empty strings
    std::vector<std::string_view> m_namespaceTokens{};

    // Filter of hook name
    // If `nullopt` means there was no `::` (HOOKNAME_SEP) in the user input
    // othewise if there was, it contains whatever was after it (Which might be nothing - So it's empty)
    std::optional<std::string_view> m_hookFilter{}; 

    // Clears both filters
    // Making all items visible again is done by `DoFilter`
    void ClearFilters() {
        m_namespaceTokens.clear(); // Clear only, so allocated memory is kept
        m_hookFilter = {};
    }

    // Are we filtering namespaces
    bool IsNamespaceFilterActive() {
        return !m_namespaceTokens.empty();
    }

    // If empty it won't filter anything
    bool IsHookFilterEmpty() {
        return m_hookFilter->empty();
    }

    // Check if hook filter is present.
    // even in case it's present it might be empty
    // in which case it wouldn't filter out anything.
    // Usually you want to use `IsHookFilterActive` which checks both.
    bool IsHookFilterPresent() {
        return m_hookFilter.has_value();
    }

    bool IsHookFilterActive() {
        return IsHookFilterPresent() && !IsHookFilterEmpty();
    }

    // Are either filters active
    bool EitherFiltersActive() {
        return IsNamespaceFilterActive() || IsHookFilterActive();
    }

    // Should the current filtered namespace be relative to the root namespace.
    // This is the case when the user prepends the namespace tokens with a `/` (NAMESPACE_SEP).
    // Eg.: `/Entity` should only show the `Entity` namespace under `Root` (But not, for example, `Audio/AEVehicleAudioEntity`)
    bool IsRelativeToRootNamespace() {
        return m_namespaceTokens.size() >= 1 && m_namespaceTokens.front().empty();
    }

    // Make all categories and their items possibly visible and/or open
    void MakeAllVisibleAndOpen(RH::HookCategory& cat, bool visible, bool open) {
        cat.Visible(true);
        cat.Open(open);

        cat.m_anyItemsVisible = true;
        for (auto& i : cat.Items()) {
            i->m_isVisible = true;
        }

        for (auto& sc : cat.SubCategories()) {
            MakeAllVisibleAndOpen(sc, visible, open);
        }
    }

    // Returns `pair<visible, open>` of this category
    auto DoFilter_Internal(RH::HookCategory& cat, size_t depth = 0) -> std::pair<bool, bool> {
        // Will be set to the appropriate values on return
        cat.Visible(false);
        cat.Open(false);

        const auto hasSubCategories = !cat.SubCategories().empty();

        // Process all sub-categories, and return if any category is pair<visible, open>
        const auto ProcessSubCategories = [&] {
            bool anyVisible{}, anyOpen{};
            for (auto& sc : cat.SubCategories()) {
                const auto [visible, open] = DoFilter_Internal(sc, depth + 1);
                anyVisible |= visible;
                anyOpen |= open;
            }
            return std::make_pair(anyVisible, anyOpen);
        };

        // If `doFilter` argument is `false` all items are set visible, and either true (if we have hooks) or false (if `cat.Items().empty()`) is returned.
        // otherwise items are filtered and true if returned if at least 1 item is visible.
        const auto ProcessItems = [&](bool allowFilter) {
            if (allowFilter && IsHookFilterActive()) {
                cat.m_anyItemsVisible = false;
                for (auto& i : cat.Items()) {
                    i->m_isVisible = StringContainsString(i->Name(), *m_hookFilter, m_caseSensitive);
                    cat.m_anyItemsVisible |= i->m_isVisible;
                }
            } else { // Otherwise make sure all items are visible (if any)
                if (cat.Items().empty()) { // No items, make sure flag is set correctly.
                    cat.m_anyItemsVisible = false;
                } else {
                    cat.m_anyItemsVisible = true;
                    for (auto& i : cat.Items()) {
                        i->m_isVisible = true; 
                    }
                }
            }
            return cat.m_anyItemsVisible;
        };

        if (IsNamespaceFilterActive()) {
            if (IsRelativeToRootNamespace()) { // Eg.: (Notice the trailing `/`) /Entity/Ped/Player/ (Should show `Root/Entity/Ped/CPlayerPed`)
                // Using root namespace.
                // In this case all tokens must match up to root

                const auto ProcessFilter = [&]() -> bool {
                    if (depth == 0) { // Special case - Root namespace depth, no need to check any tokens
                        assert(m_namespaceTokens.front().empty()); // This codepath should be unreachable unless first token is empty
                        return true; 
                    } else if (depth < m_namespaceTokens.size()) {
                        if (   hasSubCategories                      // Unless we're last category to match we must have more sub-categories so all tokens can match
                            || depth == m_namespaceTokens.size() - 1 // Last token to match, so there will be no more, thus it's fine if there are no more subcategories.
                            || m_namespaceTokens.back().empty()      // Or last token is empty (Empty strings always match everything) - This way a trailing `/` opens the category (like `::` does)
                        ) {
                            return StringContainsString(cat.Name(), m_namespaceTokens[depth], m_caseSensitive);
                        } else {
                            return false; // Not enough children to statify all tokens
                        }
                    } else { // Our parents fully matched the tokens, they just want to make us visible, but not open
                        return true;
                    }
                };

                const auto byFilterVisible = ProcessFilter();
                if (!byFilterVisible) {
                    return {};
                }

                // If botom level, and hook filtering is present: make us open, and dont show any more sub-categories
                if (depth == m_namespaceTokens.size() - 1 /*bottom level*/ && IsHookFilterPresent()) {
                    MakeAllVisibleAndOpen(cat, false, false);

                    (void)ProcessItems(true);

                    cat.Visible(true);
                    cat.Open(true);

                    return { true, true };
                }

                const auto [anySCVisible, anySCOpen] = ProcessSubCategories();

                const bool open    = anySCOpen || depth + 1 < m_namespaceTokens.size() /*All categories before the bottom level should be open*/;
                const bool visible = anySCVisible || byFilterVisible;

                cat.Visible(visible);
                cat.Open(open);

                return { visible, open };
            } else { // Eg.: Ped/Player/ (Should show `Root/Entity/Ped/CPlayerPed`)
                // Not in the root namespace
                // All tokens should match in reverse order starting at us, eg.:
                // Entity::Ped::Ped
                // So, in order to be visible* `cat`s name should contain `Ped`
                // it's parent's name should contain `Ped` and it's parent's name should contain `Entity`
                // *We may be visible if there are subcategories visible even if this function returns false

                // Example:
                // Parents: Root-Entity-Ped-CPed
                // Depth:   0     1       2    3   <= Also number of tokens
                // Tokens:        Entity::Ped::Ped

                const auto ProcessFilter = [&]() -> bool {
                    if (depth < m_namespaceTokens.size()) { // Optimization: Not enough categories to possibly staisfy all tokens
                        return false;
                    }

                    for (auto iterCat{ &cat }; auto&& token : m_namespaceTokens | rng::views::reverse) {
                        if (!StringContainsString(iterCat->Name(), token, m_caseSensitive)) { // Couldn't statify all tokens - Remember: `contains` always returns true if `token.empty()`
                            return false;
                        }
                        iterCat = iterCat->Parent();
                    }

                    return true;
                };

                const auto byFilterVisible = ProcessFilter();
                const auto itemsVisible = ProcessItems(true); // Filter items
                const auto [anySCVisible, anySCOpen] = ProcessSubCategories();

                const bool open    = anySCOpen || anySCVisible || hasSubCategories && (byFilterVisible) || IsHookFilterPresent() && itemsVisible;
                const bool visible = byFilterVisible && itemsVisible || anySCVisible;

                cat.Visible(visible);
                cat.Open(open);

                return { visible, open };
            }
;       } else {
            // Filter by hook names
            // Category is visible if it:
            // - It has visible hooks (After filtering) 
            // - Or it has visible sub-categories

            const auto itemsVisible = ProcessItems(true); // Filter items
            const auto [anySCVisible, anySCOpen] = ProcessSubCategories(); 

            const auto open = itemsVisible || anySCOpen;

            const bool visible = open || anySCVisible;

            cat.Visible(visible);
            cat.Open(open);

            return {visible, open};
        }
    }

    void DoFilter(RH::HookCategory& cat) {
        if (EitherFiltersActive()) {
            DoFilter_Internal(cat);
        } else {
            MakeAllVisibleAndOpen(cat, true, false); // Make all visible, but closed
        }        
    }

    void OnInputUpdate() {
        const std::string_view inputsv{ m_input };

        ClearFilters();

        if (inputsv.empty()) {
            return; // No filters
        }

        {   
            const auto sepPos = inputsv.rfind(HOOKNAME_SEP);

            // First half contains the namespace filter tokens
            for (auto t : SplitStringView(inputsv.substr(0, sepPos), NAMESPACE_SEP)) {
                m_namespaceTokens.emplace_back(t);
            }

            // Second half (if any) contains the hook/function name filter
            if (sepPos != std::string_view::npos) {
                m_hookFilter = inputsv.substr(sepPos + HOOKNAME_SEP.size());
            }
        }

        // In case user passes in a string with multiple `/` with nothing inbetween we will have quite a few empty tokens.
        //// We have have to remove all the leading empty tokens up until the last empty one.
        while (m_namespaceTokens.size() >= 2 && m_namespaceTokens[0].empty() && m_namespaceTokens[1].empty()) {
            m_namespaceTokens.erase(m_namespaceTokens.begin());
        }

        // Don't delete this please //
        //
        /*std::cout << "update input: ";
        for (auto&& t : m_namespaceTokens) {
            std::cout << '`' << t << "`,";
        }
        std::cout << ";" << m_hookFilter.value_or("None") << "\n";*/

        // If we're using the root namespace only but there's no hook filter we paractically dont't filter anything
        // Example user inputs: `/`, `/::`, `::`
        if (IsRelativeToRootNamespace() && m_namespaceTokens.size() == 1 && IsHookFilterActive()) {
            ClearFilters();
        }
    }

    void ProcessImGui() {
        PushItemWidth(GetWindowContentRegionMax().x - 10.f);
        if (InputText(" ", &m_input)) {
            OnInputUpdate();
            DoFilter(RH::GetRootCategory());
        }
        if (IsItemHovered()) {
            SetTooltip(
                "`::function`         - Filters only functions \n"
                "`cpy`                - Filter namespace - Will only show namespace with name containing \"cphy\"\n"
                "`player/ped`         - Should only show Ped/CPlayerPed\n"
                "`player/ped::busted` - Should only show `Ped/CPlayerPed` with the `busted` function visible only\n"
                "`/entity`            - Should only show the top level `Entity` namespace in Root\n"
                "For more tips see gta-reversed-modern/discussions/190\n"
            );
        }
        PopItemWidth();
    }
};

void RenderCategory(RH::HookCategory& cat) {
    if (!cat.Visible()) {
        return;
    }

    // NOTE: Won't work properly if `cat::m_items`s type is changed from `std::list`
    // Using this instead of the name as it's faster than encoding a string (And we care about performance, since this is mostly run in debug mode)
    PushID(&cat);

    const auto& name = cat.Name();

    const auto TreeNodeWithCheckbox = [](auto label, ImTristate triState, bool& cbStateOut, bool& cbClicked, bool& open) {
        // TODO/NOTE: The Tree's label is a workaround for when the label is shorter than the visual checkbox (otherwise the checkbox can't be clicked)
        open = TreeNodeEx("##         ", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth);

        SameLine();

        // Checkbox + it's label will be the tree name
        cbClicked = CheckboxTristate(label, triState, cbStateOut);
    };

    bool isCategoryOpen{};

    // Disable all hooks in category at once
    {
        SetNextItemOpen(cat.Open());

        bool cbClicked{};
        bool cbState{};
        TreeNodeWithCheckbox(cat.Name().c_str(), cat.OverallState(), cbState, cbClicked, isCategoryOpen);
        if (cbClicked) {
            cat.SetAllItemsEnabled(cbState);
        }

        cat.Open(isCategoryOpen);
    }

    // Draw hooks, and subcategories
    if (isCategoryOpen) {
        // Draw hooks (if any)
        if (!cat.Items().empty() && cat.m_anyItemsVisible) {
            const auto DrawItems = [&] {
                for (auto& item : cat.Items()) {
                    if (!item->m_isVisible) {
                        continue;
                    }

                    PushID(item.get()); // Use hook's address as unique ID - Hooks aren't dynamically created, so this should really be unique

                    // Draw hook symbol (`S` or `V`)
                    {
                        PushStyleVar(ImGuiStyleVar_Alpha, GetStyle().Alpha * 0.5f);
                        AlignTextToFramePadding();
                        Text(item->Symbol());
                        PopStyleVar();
                    }

                    // State checkbox
                    {
                        PushID("state");
                        bool hooked{ item->Hooked() };
                        if (SameLine(); Checkbox(item->Name().c_str(), &hooked)) {
                            cat.SetItemEnabled(item, hooked);
                        }
                        PopID(); // State checkbox
                    }

                    PopID(); // This hook
                }
            };

            if (cat.SubCategories().empty()) { // If there are no subcategories we can draw all items directly
                DrawItems();
            } else { // Otherwise use a tree node + checkbox
                bool hooksNodeOpen{};
                bool cbState{};
                bool cbClicked{};
                TreeNodeWithCheckbox("Hooks", cat.ItemsState(), cbState, cbClicked, hooksNodeOpen);
                if (cbClicked) {
                    cat.SetOurItemsEnabled(cbState);
                }
                if (hooksNodeOpen) {
                    DrawItems();
                    TreePop();
                }
            }
        }

        // Draw subcategories
        for (auto& v : cat.SubCategories()) {
            RenderCategory(v);
        }

        TreePop();
    }

    PopID();
}

namespace HooksDebugModule {
void ProcessImGui() {
    HookFilter::ProcessImGui();

    if (BeginChild("##hookstool")) {
        RenderCategory(RH::GetRootCategory());
        EndChild();
    }
}

void ProcessRender() {
}
};
