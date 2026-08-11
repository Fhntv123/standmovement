#include "edit.h"
#include "form.h"
#include "gui.hpp"
#include "../../render/render_d3/render_d3d9.hpp"
#include "help.hpp"

void Edit::draw( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// draw label.
	ui_render::string( p.x + EDIT_X_OFFSET, p.y - 2, { 205, 205, 205, m_parent->m_alpha }, m_label, g_render->fonts.test_font2 );

	auto box_draw = [ ]( float x, float y, float w, float h, int p_al ) -> void {
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
	box_draw( p.x, p.y + 15, m_w, 20, m_parent->m_alpha );

	ui_render::string( p.x + EDIT_X_OFFSET + EDIT_ITEM_X_OFFSET, p.y + 15 + 4, { 152, 152, 152, m_parent->m_alpha }, m_text, g_render->fonts.test_font2 );

	auto size = g_render->get_text_width( m_text.c_str( ), g_render->fonts.test_font2 );//ui_render::menu.size( m_text );

	if ( m_typing && ( GetTickCount( ) / 500 ) % 2 )
		ui_render::rect_filled( p.x + EDIT_X_OFFSET + EDIT_ITEM_X_OFFSET + size + 1, p.y + 15 + 13, 6, 2, { 152, 152, 152, m_parent->m_alpha } );
}

void Edit::think( ) {
	if ( m_typing ) {
		// if we want to exit typing? user presses escape or enter. he is likely done
		if ( g_input.GetKeyPress( VK_RETURN ) || g_input.GetKeyPress( VK_ESCAPE ) ) {
			m_typing = false;

			if ( m_callback )
				m_callback( );

			return;
		}

		// same applies here.
		if ( m_parent->m_active_element != this || !m_parent->m_open ) {
			m_typing = false;

			if ( m_callback )
				m_callback( );

			return;
		}

		if ( !m_old_typing )
			g_input.m_buffer = m_text;

		else
			m_text = g_input.m_buffer;

		if ( m_text.size( ) >= m_limit )
			m_text = m_text.substr( 0, m_limit );

		m_old_typing = true;
	}

	else m_old_typing = false;
}

void Edit::click( ) {
	if ( !m_typing ) {
		Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
		Point p{ area.x + m_pos.x, area.y + m_pos.y };

		// area where user has to click.
		Rect edit = { p.x + EDIT_X_OFFSET, p.y + 15, m_w - EDIT_X_OFFSET, EDIT_BOX_HEIGHT };

		if ( g_input.IsCursorInRect( edit ) )
			m_typing = true;
	}
}