#include "checkbox.h"
#include "form.h"
#include "gui.hpp"
#include "../../render/render_d3/render_d3d9.hpp"
#include "help.hpp"

void Checkbox::draw( ) {
    color_t c1 = h::hex_to_rgb( "#FF171717" );
    color_t c2 = h::hex_to_rgb( "#FF141414" );
    color_t c3 = h::hex_to_rgb( "#FF090909" );
    color_t c4 = h::hex_to_rgb( "#FF1E1E1E" );
    color_t text = h::hex_to_rgb( "#FFE2E2E2" );
    color_t text2 = h::hex_to_rgb( "#FF4C4C4C" );

    Rect area{ m_parent->GetElementsRect( m_parent->m_scroll_pos ) };
    Point p{ area.x + m_pos.x, area.y + m_pos.y };

    // get gui color.
    Color color = g_gui.m_color;
    color.alpha( m_parent->m_alpha );

    if ( m_animating ) {
        m_animation_progress += 0.1f * 0.1;
        if ( m_animation_progress >= 1.f ) {
            m_animation_progress = 1.f;
            m_animating = false;
        }
    }

    int opacity_text = static_cast< int >( h::lerp( 0.f, 255.f, m_animation_progress ) );

    // checked color.
    color_t check_text = m_checked ? Color( 205, 205, 205 ).alpha( opacity_text ) : text2.alpha( m_parent->m_alpha );
    ui_render::string( p.x + 20 + 1, p.y, check_text, m_label, g_render->fonts.test_font2, false, 1, color_t( 5, 5, 5 ) );

    ui_render::gradient( p.x + 1, p.y + 1, CHECKBOX_SIZE, CHECKBOX_SIZE, c1.alpha( m_parent->m_alpha ), c2.alpha( m_parent->m_alpha ) );

    ui_render::rect( p.x + 2, p.y + 2, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, c4.alpha( m_parent->m_alpha ) );
    ui_render::rect( p.x + 1, p.y + 1, CHECKBOX_SIZE, CHECKBOX_SIZE, c3.alpha( m_parent->m_alpha ) );

   if ( m_checked ) {
       int opacity = static_cast< int >( h::lerp( 0.f, 255.f, m_animation_progress ) );

       ui_render::rect_filled( p.x + 3, p.y + 3, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, color.alpha( opacity ) );
       ui_render::rect_filled_fade( p.x + 3, p.y + 3, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, { 25, 25, 25, static_cast< int >( h::lerp( 0.f, 150.f, m_animation_progress ) ) }, 0, 150 );
   }
}

void Checkbox::set( bool checked ) {
    if ( m_checked != checked ) {
        m_checked = checked;
        m_animating = true;
        m_animation_progress = 0.f;
    }
}

void Checkbox::think( ) {
	// set the click area to the length of the string, so we can also press the string to toggle.
	m_w = LABEL_OFFSET + g_render->get_text_width( m_label.c_str( ), g_render->fonts.menu_shade ); //ui_render::menu_shade.size( m_label ).m_width;
}

void Checkbox::click( ) {
    set( !m_checked );
}