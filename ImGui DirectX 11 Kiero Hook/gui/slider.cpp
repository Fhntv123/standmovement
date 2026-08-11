#include "slider.h"
#include "form.h"
#include "gui.hpp"
#include "../../render/render_d3/render_d3d9.hpp"
#include <iosfwd>
#include <sstream>
#include <iomanip>
#include "help.hpp"

float m_target_fill = 0.f;

void Slider::draw( ) {
	color_t c1 = h::hex_to_rgb( "#FF171717" );
	color_t c2 = h::hex_to_rgb( "#FF141414" );
	color_t c3 = h::hex_to_rgb( "#FF090909" );
	color_t c4 = h::hex_to_rgb( "#FF1E1E1E" );
	color_t text = h::hex_to_rgb( "#FFE2E2E2" );
	color_t text2 = h::hex_to_rgb( "#FF4C4C4C" );

	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };
	Rect slider{ p.x + SLIDER_X_OFFSET, p.y + m_offset, m_w - SLIDER_X_OFFSET, SLIDER_HEIGHT };

	// get gui color.
	Color color = g_gui.m_color;
	color.alpha( m_parent->m_alpha );// = m_parent->m_alpha;

	ui_render::gradient( p.x + 1, p.y + m_offset, m_w, SLIDER_HEIGHT, c1.alpha( m_parent->m_alpha ), c2.alpha( m_parent->m_alpha ) );
	ui_render::gradient( p.x + 1, p.y + m_offset, m_fill + 4, SLIDER_HEIGHT, color.alpha( m_parent->m_alpha ), color.alpha( m_parent->m_alpha ) );
	ui_render::rect_filled_fade( p.x + 1, p.y + m_offset + 1, m_fill + 4, SLIDER_HEIGHT, { 50, 50, 35, m_parent->m_alpha }, 0, 150 );


	ui_render::rect( p.x + 2, p.y + m_offset + 1, m_w - 2, SLIDER_HEIGHT - 2, c4.alpha( m_parent->m_alpha ) );
	ui_render::rect( p.x + 1, p.y + m_offset, m_w, SLIDER_HEIGHT, c3.alpha( m_parent->m_alpha ) );

	// draw label.
	ui_render::string( p.x, p.y, { 205, 205, 205, m_parent->m_alpha }, m_label, g_render->fonts.test_font2, false, 1, color_t( 5, 5, 5 ) );

	// to stringstream.
	std::wstringstream ss;
	ss << std::fixed << std::setprecision( m_precision ) << m_value << m_suffix;

	std::wstring str = ss.str( );
	std::string narrow_str( str.begin( ), str.end( ) );

	// get size.
	auto size = g_render->get_text_width( narrow_str.c_str( ), g_render->fonts.menu_shade );

	if ( g_input.IsCursorInRect( slider ) )
		ui_render::string( p.x + g_render->get_text_width( m_label.c_str( ), g_render->fonts.menu_shade ) + 5, p.y, { 255, 255, 255, m_parent->m_alpha }, ": " + narrow_str, g_render->fonts.test_font2 );
}

void Slider::think( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// how many steps do we have?
	float steps = ( m_max - m_min ) / m_step;

	// compute the amount of pixels for one step.
	float pixels = ( m_w - SLIDER_X_OFFSET ) / steps;

	// clamp the current value.
	math::clamp( m_value, m_min, m_max );

	int target_fill = ( int )std::floor( std::fmax( ( ( m_value - m_min ) / m_step ) * pixels, 0.f ) );
	m_fill = h::lerp( m_fill, target_fill, 0.2f );

	// we are draggin this mofo!
	if ( m_drag ) {
		// left mouse is still down.
		if ( g_input.GetKeyState( VK_LBUTTON ) ) {

			// compute the new value.
			float updated = m_min + ( ( ( g_input.m_mouse.x - SLIDER_X_OFFSET ) - p.x ) / pixels * m_step );

			// set updated value to closest step.
			float remainder = std::fmod( updated, m_step );

			if ( remainder > ( m_step / 2 ) )
				updated += m_step - remainder;

			else
				updated -= remainder;

			m_value = updated;

			// clamp the value.
			math::clamp( m_value, m_min, m_max );

			if ( m_callback )
				m_callback( );
		}

		// left mouse has been released.
		else
			m_drag = false;
	}
}
void Slider::click( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// get slider area.
	Rect slider{ p.x + SLIDER_X_OFFSET, p.y + m_offset, m_w - SLIDER_X_OFFSET, SLIDER_HEIGHT };

	// detect dragging.
	if ( g_input.IsCursorInRect( slider ) )
		m_drag = true;

	// clamp the updated value.
	math::clamp( m_value, m_min, m_max );
}