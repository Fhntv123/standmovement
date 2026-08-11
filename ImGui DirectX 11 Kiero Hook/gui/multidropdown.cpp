#include "multidropdown.h"
#include "form.h"
#include "gui.hpp"
#include "../../render/render_d3/render_d3d9.hpp"
#include <iosfwd>
#include <sstream>
#include <iomanip>

void MultiDropdown::arrow1( Point p ) {
	int y = 4;

	ui_render::rect_filled( p.x + m_w - 11, p.y + m_offset + 9 + 2 + y, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( p.x + m_w - 10, p.y + m_offset + 9 + 2 + y, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( p.x + m_w - 9, p.y + m_offset + 9 + 2 + y, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( p.x + m_w - 8, p.y + m_offset + 9 + 2 + y, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( p.x + m_w - 7, p.y + m_offset + 9 + 2 + y, 1, 1, { 152, 152, 152, m_parent->m_alpha } );

	ui_render::rect_filled( p.x + m_w - 10, p.y + m_offset + 9 + y + 1, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( p.x + m_w - 9, p.y + m_offset + 9 + y + 1, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( p.x + m_w - 8, p.y + m_offset + 9 + y + 1, 1, 1, { 152, 152, 152, m_parent->m_alpha } );

	ui_render::rect_filled( p.x + m_w - 9, p.y + m_offset + 9 + y, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
}

void MultiDropdown::arrow2( Point l ) {
	int y = 4;
	ui_render::rect_filled( l.x + m_w - 11, l.y + m_offset + y + 9, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( l.x + m_w - 10, l.y + m_offset + y + 9, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( l.x + m_w - 9, l.y + m_offset + y + 9, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( l.x + m_w - 8, l.y + m_offset + y + 9, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( l.x + m_w - 7, l.y + m_offset + y + 9, 1, 1, { 152, 152, 152, m_parent->m_alpha } );

	ui_render::rect_filled( l.x + m_w - 10, l.y + m_offset + y + 9 + 1, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( l.x + m_w - 9, l.y + m_offset + y + 9 + 1, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
	ui_render::rect_filled( l.x + m_w - 8, l.y + m_offset + y + 9 + 1, 1, 1, { 152, 152, 152, m_parent->m_alpha } );

	ui_render::rect_filled( l.x + m_w - 9, l.y + m_offset + y + 9 + 2, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
}

#include "help.hpp"
void MultiDropdown::draw( ) {
	Rect	area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point	p{ area.x + m_pos.x, area.y + m_pos.y };

	// get gui color.
	Color color = g_gui.m_color;
	color.alpha( m_parent->m_alpha );// = m_parent->m_alpha;

	// draw label.
	ui_render::string( p.x + LABEL_OFFSET, p.y - 2, { 205, 205, 205, m_parent->m_alpha }, m_label, g_render->fonts.test_font2, false, 1, color_t( 5, 5, 5 ) );

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

	// arrow.
	m_open ? arrow1( p ) : arrow2( p );

	// does this dropdown have any items?
	if ( !m_items.empty( ) ) {

		// is this dropdown open?
		if ( m_open ) {

			combo_drawing( p.x, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR, m_w, m_anim_height + 1 - 2, m_parent->m_alpha );

			// iterate items.
			for ( size_t i{}; i < m_items.size( ); ++i ) {

				// get offset to current item.
				int item_offset = ( i * DROPDOWN_ITEM_HEIGHT );

				// has this item been animated yet?
				if ( m_anim_height > item_offset ) {
					// check.
					bool active = std::find( m_active_items.begin( ), m_active_items.end( ), i ) != m_active_items.end( );

					// yet again, it won't use list init inside the ternary conditional.
					ui_render::string( p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_ITEM_Y_OFFSET + item_offset,
						active ? color : Color{ 152, 152, 152, m_parent->m_alpha },
						m_items[ i ], g_render->fonts.test_font2 );
				}
			}
		}

		// no items.
		if ( !m_active_items.size( ) )
			ui_render::string( p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET,
				p.y + m_offset + 7, text2.alpha( m_parent->m_alpha ), ( "-" ), g_render->fonts.test_font2 );

		// one item.
		else if ( m_active_items.size( ) == 1 )
			ui_render::string( p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET,
				p.y + m_offset + 7, text2.alpha( m_parent->m_alpha ), m_items[ m_active_items.front( ) ], g_render->fonts.test_font2 );

		// more then one item.
		else {
			std::stringstream tmp, ss;
			auto			  items = GetActiveItems( );

			for ( size_t i{}; i < items.size( ); ++i ) {
				std::string item = items[ i ];

				// first item should not have a comma infront.
				if ( i >= 1 )
					tmp << ( ", " );

				tmp << item;

				// does it still fit in?

				auto size = g_render->get_text_width( ( tmp.str( ) + "..." ).c_str( ), g_render->fonts.test_font2 );
				if ( size >= ( m_w - DROPDOWN_X_OFFSET - DROPDOWN_ITEM_X_OFFSET - 10 ) ) {
					ss << ( "..." );
					break;
				}

				if ( i >= 1 )
					ss << ( ", " );

				ss << item;
			}

			ui_render::string( p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + 7, text2.alpha( m_parent->m_alpha ), ss.str( ), g_render->fonts.test_font2 );
		}
	}
}

void MultiDropdown::think( ) {
	// fucker can be opened.
	if ( !m_items.empty( ) ) {
		int total_size = DROPDOWN_ITEM_HEIGHT * m_items.size( );

		// we need to travel 'total_size' in 300 ms.
		float frequency = total_size / 0.3f;

		// the increment / decrement per frame.
		float step = frequency * 0.005f;

		// if open		-> increment
		// if closed	-> decrement
		m_open ? m_anim_height += step : m_anim_height -= step;

		// clamp the size.
		math::clamp< float >( m_anim_height, 0, total_size );

		// open
		if ( m_open ) {
			// clicky height.
			m_h = m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR + total_size;

			// another element is in focus.
			if ( m_parent->m_active_element != this )
				m_open = false;
		}

		// closed.
		else
			m_h = m_offset + DROPDOWN_BOX_HEIGHT;
	}

	// no items, no open.
	else
		m_h = m_offset + DROPDOWN_BOX_HEIGHT;
}

void MultiDropdown::click( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// bar.
	Rect bar{ p.x + DROPDOWN_X_OFFSET, p.y + m_offset, m_w - DROPDOWN_X_OFFSET, DROPDOWN_BOX_HEIGHT };

	// toggle if bar pressed.
	if ( g_input.IsCursorInRect( bar ) )
		m_open = !m_open;

	// check item clicks.
	if ( m_open ) {
		if ( !g_input.IsCursorInRect( bar ) ) {
			// iterate items.
			for ( size_t i{}; i < m_items.size( ); ++i ) {
				Rect item{ p.x + DROPDOWN_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR + ( i * DROPDOWN_ITEM_HEIGHT ), m_w - DROPDOWN_X_OFFSET, DROPDOWN_ITEM_HEIGHT };

				// click was in context of current item.
				if ( g_input.IsCursorInRect( item ) ) {
					bool active = std::find( m_active_items.begin( ), m_active_items.end( ), i ) != m_active_items.end( );

					if ( active ) {
						m_active_items.erase( std::remove( m_active_items.begin( ), m_active_items.end( ), i ), m_active_items.end( ) );

						if ( m_callback )
							m_callback( );
					}


					else {
						m_active_items.push_back( i );

						if ( m_callback )
							m_callback( );
					}
				}
			}
		}
	}
}