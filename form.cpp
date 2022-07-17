#include "includes.h"

void Form::draw() {
	m_alpha = 0xff;
	if (!m_open)
		return;

	// get gui color.
	Color color = g_gui.m_color;
	color.a() = m_alpha;
	Color color2 = g_gui.m_color;
	Color color3 = g_gui.m_color;
	Color color4 = colors::white;

	if (m_open) {
		color2.a() = 50;
		color3.a() = 120;
		color4.a() = 120;
	}
	else if (!m_open) {
		color2.a() = 0;
		color3.a() = 0;
		color4.a() = 0;
	}
	// background.
	int top_bar_size = 22;
	render::rect_filled(m_x, m_y, m_width, m_height, { 18,18,18, m_alpha });
	render::rect_filled(m_x, m_y, m_width, top_bar_size, { 16,16,16, m_alpha });
	render::gradient(m_x, m_y + top_bar_size, m_width / 2, 1, { 0,0,0,0 }, color, true);
	render::gradient(m_x + m_width / 2, m_y + top_bar_size, m_width / 2, 1, color, { 0,0,0,0 }, true);
	render::FontSize_t text_size = render::menu_shade.size("ambien");
	render::menu_shade.string(m_x + m_width / 2, m_y + (top_bar_size / 2) - (text_size.m_height / 2), color4, "ambien", render::ALIGN_CENTER);
	render::rect(m_x + 1, m_y + 1, m_width - 2, m_height - 2, { 26,26,26, m_alpha });
	// border.
	render::rect(m_x, m_y, m_width, m_height, { 5, 5, 5, m_alpha });
	render::gradient(m_x, m_y + m_height - 40, m_width / 2, 1, { 0,0,0,0 }, color, true);
	render::gradient(m_x + m_width / 2, m_y + m_height - 40, m_width / 2, 1, color, { 0,0,0,0 }, true);
	Rect tabs_area = GetTabsRect();
	render::menu_shade.string(tabs_area.x + tabs_area.w - 8, tabs_area.y + 9, { 255,255,255,50 }, "debug | v2", render::ALIGN_RIGHT);
	// draw tabs if we have any.
	if (!m_tabs.empty()) {
		// tabs background and border.
		size_t font_w = 8;
		for (size_t i{}; i < m_tabs.size(); ++i) {
			const auto& t = m_tabs[i];
			if (t == m_active_tab) {
				render::menu_shade.string(tabs_area.x + font_w, tabs_area.y + 5, color, t->m_title);
				render::menu_shade.string(tabs_area.x + font_w, tabs_area.y + 15, { 255,255,255,50 }, t->m_desc);
			}
			else
				render::menu_shade.string(tabs_area.x + font_w, tabs_area.y + 9, color4, t->m_title);
			int descwidth = render::menu_shade.size(t->m_desc).m_width;
			int titlewidth = render::menu_shade.size(t->m_title).m_width;

			font_w += ((descwidth > titlewidth && t == m_active_tab) ? descwidth : titlewidth) + 10;
		}

		// this tab has elements.
		if (!m_active_tab->m_elements.empty()) {
			// elements background and border.
			Rect el = GetElementsRect();

			render::rect_filled(el.x, el.y, el.w, el.h, { 18,18,18, m_alpha });
			render::rect(el.x, el.y, el.w, el.h, { 5,5,5, m_alpha });
			render::rect(el.x + 1, el.y + 1, el.w - 2, el.h - 2, { 26,26,26, m_alpha });

			// iterate elements to display.
			for (const auto& e : m_active_tab->m_elements) {

				// draw the active element last.
				if (!e || (m_active_element && e == m_active_element))
					continue;

				if (!e->m_show)
					continue;

				// this element we dont draw.
				if (!(e->m_flags & ElementFlags::DRAW))
					continue;

				e->draw();
			}

			// we still have to draw one last fucker.
			if (m_active_element && m_active_element->m_show)
				m_active_element->draw();
		}
	}
}