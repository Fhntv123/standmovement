#include "listbox.h"
#include "form.h"
#include "gui.hpp"
#include "../../render/render_d3/render_d3d9.hpp"
#include "help.hpp"

void ListBox::draw( ) {
	Rect	area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point	p{ area.x + m_pos.x, area.y + m_pos.y };

	// get gui color.
	Color color = g_gui.m_color;
	color.alpha( m_parent->m_alpha );// = m_parent->m_alpha;

	auto combo_drawing = [ ]( float x, float y, float w, float h, int p_al ) -> void {
		color_t c1 = h::hex_to_rgb( "#FF171717" );
		color_t c2 = h::hex_to_rgb( "#FF141414" );
		color_t c3 = h::hex_to_rgb( "#FF090909" );
		color_t c4 = h::hex_to_rgb( "#FF1E1E1E" );
		color_t text = h::hex_to_rgb( "#FFE2E2E2" );
		color_t text2 = h::hex_to_rgb( "#FF4C4C4C" );

		ui_render::gradient( x, y, w, h, c1.alpha( p_al ), c2.alpha( p_al ) );
		ui_render::rect( x + 1, y + 1, w - 2, h - 2, c4.alpha( p_al ) );
		ui_render::rect( x, y, w, h, c3.alpha( p_al ) );
	};

	color_t c1 = h::hex_to_rgb( "#FF171717" );
	color_t c2 = h::hex_to_rgb( "#FF141414" );
	color_t c3 = h::hex_to_rgb( "#FF090909" );
	color_t c4 = h::hex_to_rgb( "#FF1E1E1E" );
	color_t text = h::hex_to_rgb( "#FFE2E2E2" );
	color_t text2 = h::hex_to_rgb( "#FF4C4C4C" );

	combo_drawing( p.x, p.y + m_offset, m_w, DROPDOWN_BOX_HEIGHT, m_parent->m_alpha );

	if ( !m_items.empty( ) ) {
		// Calculate the total width of all the items and the space between them.
		auto total_width = 0;
		for ( size_t i = 0; i < m_items.size( ); ++i ) {
			auto size = g_render->get_text_width( m_items[ i ].c_str( ), g_render->fonts.test_font2 );
			total_width += size + 6;
		}
		auto space_between_items = ( m_w - total_width ) / ( m_items.size( ) - 1 );

		// Draw the items with equal spacing between them.
		int item_offset = 0;
		for ( size_t i = 0; i < m_items.size( ); ++i ) {
			auto size = g_render->get_text_width( m_items[ i ].c_str( ), g_render->fonts.test_font2 );
			ui_render::string( p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET + item_offset, p.y + m_offset + 7,
				( i == m_active_item ) ? color : Color{ 152, 152, 152, m_parent->m_alpha },
				m_items[ i ], g_render->fonts.test_font2 );
			item_offset += size + space_between_items;
		}
	}
}

void ListBox::think() {}

void ListBox::click( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// Calculate the total width of all the items and the space between them.
	auto total_width = 0;
	for ( size_t i = 0; i < m_items.size( ); ++i ) {
		auto size = g_render->get_text_width( m_items[ i ].c_str( ), g_render->fonts.test_font2 );
		total_width += size + 6;
	}
	auto space_between_items = ( m_w - total_width ) / ( m_items.size( ) - 1 );

	int item_offset = 0;
	for ( size_t i{}; i < m_items.size( ); ++i ) {

		auto size = g_render->get_text_width( m_items[ i ].c_str( ), g_render->fonts.test_font2 );

		Rect item{ p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET + item_offset, p.y + m_offset + 7, size, DROPDOWN_ITEM_HEIGHT };

		// click was in context of current item.
		if ( g_input.IsCursorInRect( item ) ) {
			m_active_item = i;

			if ( m_callback )
				m_callback( );
		}

		item_offset += size + space_between_items;
	}
}
