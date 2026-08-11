#include "keybind.h"
#include "form.h"
#include "gui.hpp"
#include "../../render/render_d3/render_d3d9.hpp"

const std::string keynames[ ] = {
	"", ( "mouse1" ), ( "mouse2" ), ( "cancel" ), ( "mouse3" ), ( "mouse4" ), ( "mouse5" ), ( "[7]" ),
	( "backspace" ), ( "tab" ), ( "[10]" ), ( "[11]" ), ( "num 5" ), ( "enter" ), ( "[14]" ), ( "[15]" ), ( "shift" ),
	( "ctrl" ), ( "alt" ), ( "[19]" ), ( "capslock" ), ( "[21]" ), ( "[22]" ), ( "[23]" ), ( "[24]" ), ( "[25]" ),
	( "[26]" ), ( "esc" ), ( "[28]" ), ( "[29]" ), ( "[30]" ), ( "[31]" ), ( "space" ), ( "pgup" ), ( "pgdown" ), ( "end" ),
	( "home" ), ( "left" ), ( "up" ), ( "right" ), ( "down" ), ( "[41]" ), ( "[42]" ), ( "[43]" ), ( "[44]" ), ( "insert" ),
	( "delete" ), ( "[47]" ), ( "0" ), ( "1" ), ( "2" ), ( "3" ), ( "4" ), ( "5" ), ( "6" ), ( "7" ), ( "8" ), ( "9" ),
	( "[58]" ),  ( "[59]" ), ( "[60]" ), ( "[61]" ), ( "[62]" ), ( "[63]" ), ( "[64]" ), ( "a" ), ( "b" ), ( "c" ), ( "d" ),
	( "e" ),  ( "f" ), ( "g" ), ( "h" ), ( "i" ), ( "j" ), ( "k" ), ( "l" ), ( "m" ), ( "n" ), ( "o" ), ( "p" ), ( "q" ),
	( "r" ), ( "s" ), ( "t" ), ( "u" ), ( "v" ), ( "w" ), ( "x" ), ( "y" ), ( "z" ),	( "left win" ), ( "right win" ),
	( "application" ), ( "[94]" ),  ( "[95]" ), ( "num 0" ), ( "num 1" ), ( "num 2" ), ( "num 3" ), ( "num 4" ), ( "num 5" ),
	( "num 6" ), ( "num 7" ), ( "num 8" ),  ( "num 9" ), ( "num mult" ), ( "num plus" ), ( "[108]" ), ( "num sub" ), ( "num decimal" ),
	( "num divide" ), ( "f1" ), ( "f2" ),  ( "f3" ), ( "f4" ), ( "f5" ), ( "f6" ), ( "f7" ), ( "f8" ), ( "f9" ), ( "f10" ),
	( "f11" ), ( "f12" ), ( "f13" ), ( "f14" ), ( "f15" ), ( "f16" ), ( "f17" ),  ( "f18" ), ( "f19" ), ( "f20" ), ( "f21" ),
	( "f22" ), ( "f23" ), ( "f24" ), ( "[136]" ), ( "[137]" ), ( "[138]" ), ( "[139]" ), ( "[140]" ), ( "[141]" ), ( "[142]" ),
	( "[143]" ), ( "num lock" ), ( "break" ), ( "[146]" ), ( "[147]" ), ( "[148]" ), ( "[149]" ), ( "[150]" ), ( "[151]" ), ( "[152]" ),
	( "[153]" ), ( "[154]" ), ( "[155]" ), ( "[156]" ),  ( "[157]" ), ( "[158]" ), ( "[159]" ), ( "[160]" ), ( "[161]" ), ( "ctrl" ),
	( "ctrl" ), ( "alt" ), ( "alt" ), ( "[166]" ), ( "[167]" ), ( "[168]" ), ( "[169]" ), ( "[170]" ), ( "[171]" ), ( "m" ), ( "d" ),
	( "c" ), ( "b" ),  ( "p" ), ( "q" ), ( "j" ), ( "g" ), ( "[180]" ), ( "[181]" ), ( "[182]" ), ( "f" ), ( "[184]" ), ( "[185]" ),
	( ";" ), ( "=" ), ( " )," ), ( "-" ), ( "." ), ( "/" ), ( "grave" ), ( "[193]" ), ( "[194]" ), ( "[195]" ), ( "[196]" ),
	( "[197]" ), ( "[198]" ),  ( "[199]" ), ( "[200]" ), ( "[201]" ), ( "[202]" ), ( "[203]" ), ( "[204]" ), ( "[205]" ), ( "[206]" ),
	( "[207]" ), ( "[208]" ), ( "[209]" ), ( "[210]" ), ( "[211]" ), ( "[212]" ), ( "[213]" ), ( "[214]" ), ( "[215]" ), ( "[216]" ),
	( "[217]" ),  ( "[218]" ), ( "[" ), ( "\\" ), ( "]" ), ( "[222]" ), ( "[223]" ), ( "[224]" ), ( "[225]" ), ( "\\" ), ( "[227]" ),
	( "[228]" ), ( "[229]" ), ( "[230]" ), ( "[231]" ), ( "[232]" ), ( "[233]" ), ( "right win" ), ( "[235]" ), ( "[236]" ), ( "[237]" ),
	( "[238]" ), ( "[239]" ), ( "[240]" ), ( "left win" ), ( "[242]" ), ( "[243]" ), ( "[244]" ), ( "[245]" ), ( "[246]" ), ( "[247]" ),
	( "[248]" ), ( "application" ), ( "[250]" ), ( "[251]" ), ( "[252]" ), ( "[253]" )
};;

#include "help.hpp"
void Keybind::draw( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// get gui color.
	Color color = g_gui.m_color;
	color.alpha( m_parent->m_alpha );// = m_parent->m_alpha;

	// draw label.
	ui_render::string( p.x + KEYBIND_X_OFFSET, p.y - 2, { 205, 205, 205, m_parent->m_alpha }, m_label, g_render->fonts.test_font2 );

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


	// todo; animate with '...'
	if ( m_set )
		ui_render::string( p.x + KEYBIND_X_OFFSET + KEYBIND_ITEM_X_OFFSET, p.y + 15 + 4, color, ( "press key" ), g_render->fonts.test_font2 );

	// we have a key assigned.
	else if ( m_key >= 0 && m_key <= 0xFE )
		ui_render::string( p.x + KEYBIND_X_OFFSET + KEYBIND_ITEM_X_OFFSET, p.y + 15 + 4, { 152, 152, 152, m_parent->m_alpha }, keynames[ m_key ], g_render->fonts.test_font2 );
}

void Keybind::think( ) {
	// we want to set a key.
	if ( m_set ) {

		// iterate all keystates.
		for ( int i{}; i <= 0xFE; ++i ) {

			// find pressed key.
			if ( g_input.GetKeyPress( i ) ) {
				// escape is our wildcard.
				if ( i == VK_ESCAPE ) {
					m_key = -1;
					m_set = false;

					if ( m_callback )
						m_callback( );

					return;
				}

				m_key = i;
				m_set = false;
				m_old_set = true;

				if ( m_callback )
					m_callback( );

				return;
			}
		}
	}

	else m_old_set = false;
}

void Keybind::click( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// area where user has to click.
	Rect bind = { p.x + KEYBIND_X_OFFSET, p.y + 15, m_w - KEYBIND_X_OFFSET, KEYBIND_BOX_HEIGHT };

	if ( !m_set && !m_old_set && g_input.IsCursorInRect( bind ) )
		m_set = true;
}