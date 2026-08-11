#include "button.h"
#include "form.h"
#include "gui.hpp"
#include "../../render/render_d3/render_d3d9.hpp"
#include "help.hpp"

void Button::draw( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

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
	box_draw( p.x, p.y + 15, m_w, BUTTON_BOX_HEIGHT, m_parent->m_alpha );

	auto current_time = std::chrono::system_clock::now( );
	auto current_time_seconds = std::chrono::duration_cast< std::chrono::seconds >( current_time.time_since_epoch( ) ).count( );

	color_t text = h::hex_to_rgb( "#FFE2E2E2" );
	color_t text2 = h::hex_to_rgb( "#FF4C4C4C" );

	ui_render::string( p.x + ( ( BUTTON_X_OFFSET + m_w ) / 2 ), p.y + 23, text2.alpha( m_parent->m_alpha ), m_label, g_render->fonts.test_font2, true );
}

void Button::think( ) {}

void Button::click( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// area where user has to click.
	Rect btn = { p.x + BUTTON_X_OFFSET, p.y, m_w - BUTTON_X_OFFSET, BUTTON_BOX_HEIGHT };

	if ( g_input.IsCursorInRect( btn ) && m_callback )
		m_callback( );
}