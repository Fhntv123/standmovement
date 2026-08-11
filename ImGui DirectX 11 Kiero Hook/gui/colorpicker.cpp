#include "colorpicker.h"
#include "form.h"
#include "gui.hpp"
#include "../../render/render_d3/render_d3d9.hpp"

#define Color color_t

// static variables.
int Colorpicker::texture = 0;;
std::unique_ptr< Color[ ] > Colorpicker::gradient = nullptr;;

void rgb_to_hsv( float r, float g, float b, float& out_h, float& out_s, float& out_v )
{
	float K = 0.f;
	if ( g < b )
	{
		const auto temp = b;
		b = g;
		g = temp;
		K = -1.f;
	}
	if ( r < g )
	{
		const auto temp = r;
		r = g;
		g = temp;
		K = -2.f / 6.f - K;
	}

	const float chroma = r - ( g < b ? g : b );
	out_h = fabsf( K + ( g - b ) / ( 6.f * chroma + 1e-20f ) );
	out_s = chroma / ( r + 1e-20f );
	out_v = r;
}
void hsv_to_rgb( float h, float s, float v, float& out_r, float& out_g, float& out_b )
{
	if ( s == 0.0f )
	{
		// gray
		out_r = out_g = out_b = v;
		return;
	}

	h = fmodf( h, 1.0f ) / ( 60.0f / 360.0f );
	int   i = ( int )h;
	float f = h - ( float )i;
	float p = v * ( 1.0f - s );
	float q = v * ( 1.0f - s * f );
	float t = v * ( 1.0f - s * ( 1.0f - f ) );

	switch ( i )
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
}

#include "help.hpp"
using namespace h;
void Colorpicker::draw( ) {
	color_t backround = hex_to_rgb( "#FF141414" );
	color_t outline = hex_to_rgb( "#FF1E1E1E" );
	color_t gradient_1_topbar = hex_to_rgb( "#FF1E1E1E" );
	color_t gradient_2_topbar = hex_to_rgb( "#FF151515" );
	color_t gradient_3_topbar = hex_to_rgb( "#FF0F0F0F" );

	color_t gradient_1_tabs = hex_to_rgb( "#FF0F0F0F" );
	color_t gradient_2_tabs = hex_to_rgb( "#FF121212" );

	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point pos{ area.x + m_pos.x, area.y + m_pos.y };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };
	Point picker_start_pos{ area.x + m_pos.x, area.y + m_pos.y };
	Color picker_col = m_color;

	ui_render::string( pos.x + LABEL_OFFSET, pos.y, { 205, 205, 205, m_parent->m_alpha }, m_label, g_render->fonts.test_font2 );

	ui_render::rect_filled( pos.x + m_w - 18 + 12, pos.y + 5, 12, 6, picker_col );
	ui_render::rect( pos.x + m_w - 19 + 12, pos.y + 5, 12, 5, outline );

	if ( m_open ) {
		// this should be fixed now
		const int t_height = m_w + 10;

		/* main pos */
		int x = p.x;
		int y = p.y;
		int w = m_w;
		int h = m_h;
		int cp_y = y + 12 + m_h;
		int width = w;

		ui_render::rect_filled( x - 1, cp_y, width + 2, t_height + 2, backround.alpha( m_parent->m_alpha ) );

		// now we render the rounded color preview
		const float colpalwidth = m_w - 8;
		const float colpalwidth2 = m_w - 10;
		const float colpalheight = m_h + 3;

		for ( int i = 0; i < colpalwidth2; ++i ) {
			ui_render::rect_filled( p.x + i + 4, cp_y + 5, 1, 8, Color( ).hsb_to_rgb( i / colpalwidth2, 1.f, 1.f ) );
		}

		// color pal
		const auto shade_height = ( t_height - colpalheight - 18 );
		const int box_size = 3;
		for ( int c = 0; c < colpalwidth; c += box_size ) {
			for ( int r = 0; r < shade_height; r += box_size ) {
				const auto cpos = vec2_t( c, r );
				const auto saturation_percent = std::clamp( cpos.x / colpalwidth, 0.f, 1.f );
				const auto brightness_percent = std::clamp( 1.f - ( cpos.y / shade_height ), 0.f, 1.f );
				auto size_x = box_size;
				auto size_y = box_size;

				{
					if ( p.x + c + 4 + size_x > p.x + m_w - 3 )
						size_x = ( p.x + m_w - 4 ) - ( p.x + c + 4 );
					if ( p.y + 14 + m_h + colpalheight + 4 + r + size_y > p.y + 14 + m_h + colpalheight + 4 + shade_height )
						size_y = ( p.y + 14 + m_h + colpalheight + 4 + shade_height ) - ( p.y + 14 + m_h + colpalheight + 4 + r );
				}


				float hue, saturation, value;
				rgb_to_hsv( float( m_color.get_red( ) ) / 255.f, float( m_color.get_green( ) ) / 255.f, float( m_color.get_blue( ) ) / 255.f, hue, saturation, value );
				const auto pixel_color = Color( ).hsb_to_rgb( hue, saturation_percent, brightness_percent );
				ui_render::rect_filled( p.x + c + 4, p.y + 16 + m_h + colpalheight + 4 + r, size_x, size_y, pixel_color );
			}
		}

		// alpha bar
		int alpha_x = x + 4;
		int alpha_y = y + 30 + m_h + shade_height + 3;

		ui_render::rect_filled( x + 4, y + 30 + m_h + shade_height + 3, colpalwidth, colpalheight - 4, m_color );
		ui_render::rect( alpha_x, alpha_y, colpalwidth, colpalheight - 4, outline );

		// hsv system
		if ( DoDragHue ) {
			if ( GetAsyncKeyState( VK_LBUTTON ) ) {
				if ( g_input.Mousein( vec3_t( p.x + 4, cp_y + 5, 0 ), vec3_t( colpalwidth2, 8, 0 ) ) ) {
					m_hue_pos.x = g_input.m_mouse.x - ( p.x + 4 );
					m_hue_pos.y = g_input.m_mouse.y - ( cp_y + 5 );
					m_hue = ( g_input.m_mouse.x - p.x + 4 ) / ( colpalwidth2 );
				}
			}
			else
			{
				DoDragHue = false;
			}
		}

		if ( m_hue_pos.x && m_hue_pos.y )
		{
			m_hue_pos.x = std::clamp( m_hue_pos.x, 2.f, colpalwidth2 - 2 );
			ui_render::rect_filled( m_hue_pos.x - 2 + ( p.x + 4 ), cp_y + 3, 5, colpalheight + 1, color_t( 153, 153, 159 ) );
			ui_render::rect( m_hue_pos.x - 2 + ( p.x + 4 ), cp_y + 3, 4, colpalheight, outline );

		}

		if ( do_drag_alpha ) {
			if ( GetAsyncKeyState( VK_LBUTTON ) ) {
				if ( g_input.Mousein( vec3_t( alpha_x, alpha_y, 0 ), vec3_t( colpalwidth, colpalheight - 4, 0 ) ) ) {
					m_alpha_pos.x = std::clamp( int( g_input.m_mouse.x - ( alpha_x ) ), 0, 255 );
					m_alpha_pos.y = g_input.m_mouse.x - ( alpha_y ); // mouse.x was
					m_color.alpha( int( 255 * float( m_alpha_pos.x / colpalwidth ) ) );

				}
			}
			else if ( !GetAsyncKeyState( VK_LBUTTON ) ) {
				do_drag_alpha = false;
			}
		}

		if ( m_alpha_pos.x && m_alpha_pos.y ) {
			m_alpha_pos.x = std::clamp( m_alpha_pos.x, 2.f, colpalwidth - 2 );
			ui_render::rect( m_alpha_pos.x - 2 + ( alpha_x ), alpha_y, 4, colpalheight - 4, Color( 43, 44, 46, 255 ) );
		}

		// drag color
		if ( DoDragCol ) {
			if ( GetAsyncKeyState( VK_LBUTTON ) ) {
				if ( g_input.Mousein( vec3_t( p.x + 4, p.y + 16 + m_h + colpalheight + 4, 0 ), vec3_t( colpalwidth, shade_height, 0 ) ) ) {

					m_saturation = std::clamp( ( g_input.m_mouse.x - ( p.x + 4 ) ) / colpalwidth, 0.f, 1.f );
					m_value = std::clamp( 1.f - ( g_input.m_mouse.y - ( p.y + 16 + m_h + colpalheight + 4 ) ) / shade_height, 0.f, 1.f );
					ColorPos.y = g_input.m_mouse.y - p.y;
					ColorPos.x = g_input.m_mouse.x - p.x;

				}
			}
			else {
				DoDragCol = false;
			}
		}

		if ( ColorPos.x && ColorPos.y )
		{
			ColorPos.x = std::clamp( ColorPos.x, 0.f, colpalwidth - 5 );
			ColorPos.y = std::clamp( ColorPos.y, 0.f, shade_height + 35 );
			ui_render::rect( ColorPos.x + p.x, ColorPos.y + p.y, 5, 5, Color( 43, 44, 46, 255 ) );
		}

		ui_render::rect( x - 2, cp_y - 1, width + 2, t_height + 2, outline.alpha( m_parent->m_alpha ) );
	}
}

void Colorpicker::think( ) {
	Rect  area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	int x = p.x;
	int y = p.y;
	int w = m_w;
	int h = m_h;
	int cp_y = y + 12 + m_h;
	int width = w;

	const float colpalwidth = m_w - 8;
	const float colpalwidth2 = m_w - 10;
	const float colpalheight = m_h + 3;
	const int t_height = m_w + 20;
	const auto shade_height = ( t_height - colpalheight - 7 );
	int alpha_x = x + 4;
	int alpha_y = y + 27 + m_h + shade_height + 3;

	if ( m_open ) {
		Rect picker{ p.x + 4, p.y + 17, colpalwidth, colpalheight };

		if ( GetKeyState( VK_LBUTTON ) && g_input.IsCursorInRect( picker ) ) {
			m_open = true;
		}

		if ( !DoDragCol || !DoDragHue || !do_drag_alpha ) {
			if ( g_input.Mousein( vec3_t( p.x + 2, cp_y + 5, 0 ), vec3_t( colpalwidth2, colpalheight, 0 ) ) )
			{
				DoDragCol = false;
				DoDragHue = true;
				do_drag_alpha = false;

				m_color = Color( ).hsb_to_rgb( m_hue, m_saturation, m_value ).alpha( m_color.get_alpha( ) );

				// override alpha
				m_color.alpha( std::clamp( m_color.get_alpha( ), 0, 255 ) );

				m_parent->m_active_element = this;
			}

			if ( g_input.Mousein( vec3_t( p.x + 4, p.y + 16 + m_h + colpalheight + 4, 0 ), vec3_t( colpalwidth, shade_height, 0 ) ) )
			{
				DoDragCol = true;
				DoDragHue = false;
				do_drag_alpha = false;

				m_color = Color( ).hsb_to_rgb( m_hue, m_saturation, m_value ).alpha( m_color.get_alpha( ) );

				// override alpha
				m_color.alpha( std::clamp( m_color.get_alpha( ), 0, 255 ) );
				m_parent->m_active_element = this;
			}


			if ( g_input.Mousein( vec3_t( alpha_x, alpha_y - 20, 0 ), vec3_t( colpalwidth, colpalheight - 2, 0 ) ) ) {
				DoDragCol = false;
				DoDragHue = false;
				do_drag_alpha = true;

				m_color = Color( ).hsb_to_rgb( m_hue, m_saturation, m_value ).alpha( m_color.get_alpha( ) );

				// override alpha
				m_color.alpha( std::clamp( m_color.get_alpha( ), 0, 255 ) );
				m_parent->m_active_element = this;
			}

		}

		// thats closing colorpicker when u are not in the area
		/*
			if ( !g_input.Mousein( vec3_t( p.x - 10, p.y, 0 ), vec3_t( m_w + 20, m_h + m_h + shade_height + 30, 0 ) ) ) {
				DoDragHue = false;
				DoDragCol = false;
				do_drag_alpha = false;
				m_open = false;
				if ( m_callback )
					m_callback( );
			}
		*/

		if ( m_parent->m_active_element != this ) {
			m_open = false;
		}
	}


	/*
	//if ( m_open ) {
		Rect picker{ p.x + m_w - COLORPICKER_WIDTH, p.y + COlORPICKER_HEIGHT + 2, 140, COLORPICKER_PICKER_SIZE };

	//	if ( g_input.IsCursorInRect( picker ) && g_input.GetKeyState( VK_LBUTTON ) )
			//m_color = Colorpicker::ColorFromPos( g_input.m_mouse.x - ( p.x + m_w - COLORPICKER_WIDTH ), g_input.m_mouse.y - ( p.y + COlORPICKER_HEIGHT + 2 ) );

		if ( !g_input.GetKeyState( VK_LBUTTON ) || m_parent->m_active_element != this ) {
			m_open = false;

			if ( m_callback )
				m_callback( );
		}
	//}
	*/

	if ( m_ptr )
		*m_ptr = m_color;
}

void Colorpicker::click( ) {
	m_open = !m_open;
}