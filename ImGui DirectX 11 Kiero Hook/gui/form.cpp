#include "form.h"
#include "../../render/render_d3/render_d3d9.hpp"
#include "gui.hpp"
#include "tiny_format.h"
#include "help.hpp"
#include "notify.h"

using namespace h;
Notify g_notify;

void gain_mouse_pos2( vec2_t& last, vec2_t& new_ ) {
    POINT p_mouse_pos;
    GetCursorPos( &p_mouse_pos );
    ScreenToClient( FindWindow( 0, ( "workspace" ) ), &p_mouse_pos );
    last = new_;
    new_ = vec2_t( static_cast< int >( p_mouse_pos.x ), static_cast< int >( p_mouse_pos.y ) );
}

void keybind( ) {

    struct key_binds_t {
        std::string text;
        std::string mode;
    };
    std::vector< key_binds_t > keys{ };

    // keybind_t
    key_binds_t key{ };

    // we re going to do dymanic position
    vec2_t position = vec2_t( 10, 624 );
    static vec2_t main_mouse, last_mouse, s_drag;
    bool is_dragging;
    auto x = position.x;
    auto y = position.y;

    gain_mouse_pos2( main_mouse, last_mouse );

    // dragging logic.
    if ( main_mouse.x > position.x - s_drag.x && main_mouse.y > position.y - s_drag.y && main_mouse.x < ( position.x - s_drag.x ) + 200 && main_mouse.y < ( position.y - s_drag.y ) + 20 && GetAsyncKeyState( VK_LBUTTON ) ) {
        s_drag.x += main_mouse.x - last_mouse.x;
        s_drag.y += main_mouse.y - last_mouse.y;
        is_dragging = true;
    }

    x -= s_drag.x;
    y -= s_drag.y;

    auto translate_mode = [ ]( int style ) -> std::string {
        switch ( style ) {
        case 0: {
            return "active";
        } break;
        case 1: {
            return "hold";
        } break;
        case 2: {
            return "toggled";
        } break;
        case 3: {
            return "force off";
        } break;
        }
    };


    if ( GetAsyncKeyState(VK_HOME) ) {
        key_binds_t ind{ };
        ind.mode = translate_mode( 2 );
        ind.text = "double-tap";
        keys.push_back( ind );
    }

    int pixel = keys.empty( ) ? 18 : 18 + ( 14 * keys.size( ) ) + 1;

    ui_render::rect_filled( x, y, 210, pixel, color_t( 35, 35, 35, 150 ) );
    ui_render::rect_filled( x, y + 1, 210, 17, color_t( 20, 20, 20 ) );

    std::string indicators2 = "keybinds";
    std::transform( indicators2.begin( ), indicators2.end( ), indicators2.begin( ), ::toupper );
    auto text_size_indicators2 = g_render->get_text_width( indicators2.c_str( ), g_render->fonts.indicators );
    int centerX = x + ( 210 - text_size_indicators2 ) / 2;
    g_render->text( centerX, y + 4, g_render->fonts.indicators, indicators2, color_t( 255, 255, 255 ), false, 0 );

    ui_render::rect_filled( x, y + 1, 210, 1, g_gui.m_color );
    ui_render::rect( x, y, 210, pixel, color_t( 15, 15, 15 ) );

    for ( size_t i{ }; i < keys.size( ); ++i ) {
        auto& indicator = keys[ i ];

        std::string indicators2 = indicator.mode;
        std::transform( indicators2.begin( ), indicators2.end( ), indicators2.begin( ), ::toupper );
        auto text_size_indicators2 = g_render->get_text_width( indicators2.c_str( ), g_render->fonts.indicators );

        int render_x = x + 7; // Starting position of the text
        int render_y = y + pixel - 13;
        int text_render_pos = x + 207 - text_size_indicators2; // Position to render the text

        g_render->text( x + 5, y + pixel - 13, g_render->fonts.indicators, indicator.text, color_t( 255, 255, 255 ), false, 0 );
        g_render->text( text_render_pos, y + pixel - 13, g_render->fonts.indicators, indicator.mode, color_t( 255, 255, 255 ), false, 0 );
    }

}

void watermark( int x, int y ) {
    color_t backround = hex_to_rgb( "#FF141414" );
    color_t outline = hex_to_rgb( "#FF1E1E1E" );
    color_t gradient_1_topbar = hex_to_rgb( "#FF1E1E1E" );
    color_t gradient_2_topbar = hex_to_rgb( "#FF151515" );
    color_t gradient_3_topbar = hex_to_rgb( "#FF0F0F0F" );

    color_t gradient_1_tabs = hex_to_rgb( "#FF0F0F0F" );
    color_t gradient_2_tabs = hex_to_rgb( "#FF121212" );


    ui_render::rect_filled( x, y, 201, 41, backround );
    ui_render::rect( x, y, 200, 40, outline );
    ui_render::gradient( x + 1, y, 200, 20, gradient_1_topbar.alpha( 255 ), gradient_2_topbar.alpha( 255 ) );

    // cheat name
    ui_render::string( x + 201 / 2 - g_render->get_text_width( "memesis" , g_render->fonts.test_font2 ) / 2, y + 3, color_t( 150, 150, 150 ), "memesis", g_render->fonts.test_font2 );

    // build: 
    ui_render::string( x + 5, y + 22, color_t( 150, 150, 150 ), "build: ", g_render->fonts.test_font2 );
    ui_render::string( x + g_render->get_text_width( "developer", g_render->fonts.test_font2 ) - 13, y + 22, g_gui.m_color, "developer", g_render->fonts.test_font2 );
    ui_render::string( x + g_render->get_text_width( "build: developer ", g_render->fonts.test_font2 ) + 9, y + 22, color_t( 150, 150, 150 ), "/ user: ", g_render->fonts.test_font2 );
}

void Form::draw( ) {
    g_notify.think( );

    static bool once = false;
    if ( !once ) {
        g_notify.add( "hellop" );
        once = true;
    }

    color_t backround = hex_to_rgb( "#FF141414" ) ;
    color_t outline = hex_to_rgb( "#FF1E1E1E" );
    color_t gradient_1_topbar = hex_to_rgb( "#FF1E1E1E" );
    color_t gradient_2_topbar = hex_to_rgb( "#FF151515" );
    color_t gradient_3_topbar = hex_to_rgb( "#FF0F0F0F" );

    color_t gradient_1_tabs = hex_to_rgb( "#FF0F0F0F" );
    color_t gradient_2_tabs = hex_to_rgb( "#FF121212" );

    // Declare variables for the animation
    static float currentOpacity = 0.0f;
    static float targetOpacity = 0.0f;
    static float animationDuration = 1500.0f; // 500ms
    static float elapsedTime = 0.0f;
    static bool isOpening = false;
    static int bounceCount = 0;
    static int maxBounceCount = 3;


    // Determine whether the animation should be opening or closing
    if ( m_open && !isOpening )
    {
        targetOpacity = 1.0f;
        elapsedTime = 0.0f;
        isOpening = true;

        // @ fix alpha elements
        m_alpha = 255;
    }
    else if ( !m_open && isOpening )
    {
        targetOpacity = 0.0f;
        elapsedTime = 0.0f;
        isOpening = false;
    }

    // Calculate the new value of currentOpacity using lerping
    if ( elapsedTime < animationDuration )
    {
        float t = elapsedTime / animationDuration;
        currentOpacity = h::lerp( currentOpacity, targetOpacity, t );
        elapsedTime += 10.0f; // Assume 60 FPS, so 16ms per frame
    }

    // Clamp the opacity value
    math::clamp( currentOpacity, 0.0f, 1.0f );

    m_alpha = 0xff * currentOpacity;
    if ( !m_alpha )
        return;

    // Get gui color.
    Color color = g_gui.m_color;
    color.alpha( m_alpha );

	// background.
    ui_render::rect_filled( m_x, m_y, m_width, m_height, backround.alpha( m_alpha ) );

    // menu backroudn
    int grid_step = 5;
    int grid_size = 1;
    Color col_bg1( 20, 20, 20, 255 );
    Color col_bg2( 15, 15, 15, 183 );

    for ( int i = 0; i < m_width; i += grid_step * grid_size ) {
        // Ensure that the final square in the grid is fully within the canvas
        int x_max = std::fmin( i + grid_step * grid_size, m_width );

        for ( int j = 0; j < m_height; j += grid_step * grid_size )
        {
            // Ensure that the final square in the grid is fully within the canvas
            int y_max = std::fmin( j + grid_step * grid_size, m_height );

            // Draw a grid of squares
            for ( int ii = i; ii < x_max; ii += grid_step )
            {
                for ( int jj = j; jj < y_max; jj += grid_step )
                {
                    Color col = ( ( ii + jj ) / grid_step ) % 2 == 0 ? col_bg1 : col_bg2;
                    int width = std::fmin( grid_step, x_max - ii );
                    int height = std::fmin( grid_step, y_max - jj );
                    ui_render::rect_filled( m_x + ii, m_y + jj, width, height, col );
                }
            }
        }
    }

    // tabs.
    ui_render::gradient( m_x, m_y, 155, m_height, gradient_2_tabs.alpha( m_alpha ), gradient_2_tabs.alpha( m_alpha ) );
    ui_render::rect_filled( m_x + 155, m_y, 1, m_height, outline.alpha( m_alpha ) );

    // top bar.
    ui_render::gradient( m_x, m_y, m_width, 53, gradient_1_topbar.alpha( m_alpha ), gradient_2_topbar.alpha( m_alpha ) );
    ui_render::gradient( m_x, m_y + 53, m_width, 7, gradient_3_topbar.alpha( 50 * m_alpha ), gradient_3_topbar.alpha( 0 * m_alpha ) );


    // name
    // Declare variables for the animation
    static bool isBouncing = false;
    static int numBounces = 0;
    static int maxBounces = 1;
    static float bounceDuration = 1300.0f; // 100ms
    static float bounceElapsedTime = 0.0f;
    static float bounceHeight = 5.0f;
    static float bounceStartY = m_y + 7.0f;
    static float textOffsetY = 0.0f;

    // Determine whether the text should be bouncing
    if ( m_open && !isBouncing )
    {
        isBouncing = true;
        numBounces = 0;
        bounceElapsedTime = 0.0f;
        textOffsetY = 0.0f;
    }
    else if ( !m_open && isBouncing )
    {
        isBouncing = false;
        numBounces = 0;
        bounceElapsedTime = 0.0f;
        textOffsetY = 0.0f;
    }

    // Calculate the new value of textOffsetY for the bouncing animation
    if ( isBouncing && numBounces < maxBounces )
    {
        float t = bounceElapsedTime / bounceDuration;
        textOffsetY = -bounceHeight * sin( t * 3.14159265359 );
        bounceElapsedTime += 16.0f; // Assume 60 FPS, so 16ms per frame

        // Check if the bounce has ended
        if ( t >= 1.0f )
        {
            numBounces++;
            bounceElapsedTime = 0.0f;
        }
    }

    // Draw the text with the updated Y offset
    int x_pos = m_x + 8;
    int y_pos = m_y + 7;

    // create a string with the text to be rendered
    std::string text_to_render = "A";
    ui_render::string( x_pos, y_pos + 1, g_gui.m_color.alpha( m_alpha ), text_to_render, g_render->fonts.memeiss, false );
    ui_render::string( x_pos + 27, y_pos, g_gui.m_color.alpha( m_alpha ), "EMESIS", g_render->fonts.test_font, false );

    // developed by
    ui_render::string( m_x + 8, m_y + 32, color_t( 255, 255, 255, m_alpha - 150 ), "DEVELOPED BY", g_render->fonts.main_font, false, 2, color_t( 5, 5, 5, m_alpha ) );

    auto developed_width = g_render->get_text_width( "DEVELOPED BY", g_render->fonts.main_font );
    ui_render::string( m_x + 8 + developed_width + 4, m_y + 32, g_gui.m_color.alpha( m_alpha ), "DUTU1337", g_render->fonts.main_font, false, 2, color_t( 5, 5, 5, m_alpha ) );

    // owning since
    ui_render::string( m_x + 17, m_y + m_height - 17, color_t( 255, 255, 255, m_alpha - 150 ), "OWNING SINCE", g_render->fonts.main_font, false, 2, color_t( 5, 5, 5, m_alpha ) );

    auto developed_width2 = g_render->get_text_width( "OWNING SINCE", g_render->fonts.main_font );
    ui_render::string( m_x + 17 + developed_width2 + 4, m_y + m_height - 17, g_gui.m_color.alpha( m_alpha ), "2023", g_render->fonts.main_font, false, 2, color_t( 5, 5, 5, m_alpha ) );


    // reis bar
    ui_render::rect( m_x - 1, m_y - 1, m_width, m_height, outline.alpha( m_alpha ) );
    ui_render::rect_filled( m_x + 1, m_y - 1, m_width - 2, 1, gradient_2_topbar.alpha( m_alpha ) );
    ui_render::rect_filled( m_x - 1, m_y + 2, 1, 49, gradient_2_topbar.alpha( m_alpha ) );
    ui_render::rect_filled( m_x + m_width, m_y + 2, 1, 49, gradient_2_topbar.alpha( m_alpha ) );

    // Declare variables for the animation -> doesnt work, maybe u can fix it
    static float currentWidth = 0;
    static float targetWidth = 155.0f;
    static float animationDuration2 = 2500.0f; // 500ms
    static float elapsedTime2 = 0.0f;
    static bool isAnimating = false;

    if ( !m_tabs.empty( ) ) {
        // tabs background and border.
        Rect tabs_area = GetTabsRect( );
 
        for ( size_t i{}; i < m_tabs.size( ); ++i ) {
            color_t c1 = hex_to_rgb( "#FF161616" );

            const auto& t = m_tabs[ i ];
            auto tab_size = 31;
            RECT tab_position = { tabs_area.x + 5, tabs_area.y + 10 + ( i * tab_size ), tab_size, 500 };
            color_t color2 = t == m_active_tab ? color.alpha( m_alpha ) : color_t( 149, 149, 149, m_alpha );
            color_t color3 = t == m_active_tab ? color.alpha( m_alpha ) : color_t( 149, 149, 149, 0 );

            // Determine whether to animate
            if ( t == m_active_tab && !isAnimating ) {
                targetWidth = 200.0f;
                elapsedTime2 = 0.0f;
                isAnimating = true;
            }
            else if ( t != m_active_tab && isAnimating ) {
                targetWidth = 155.0f;
                elapsedTime2 = 0.0f;
                isAnimating = false;
            }

            // Calculate the new value of currentWidth using lerping
            if ( elapsedTime2 < animationDuration2 ) {
                float t = elapsedTime2 / animationDuration2;
                currentWidth = lerp( currentWidth, targetWidth, t );
                elapsedTime2 += 16.0f; // Assume 60 FPS, so 16ms per frame
            }

            // Clamp the width value
            math::clamp( currentWidth, 0.0f, 200.0f );

            ui_render::gradient_v( tab_position.left - 20, tab_position.top, currentWidth, 27, t == m_active_tab ? c1 : c1.alpha( 0 ), c1.alpha( 0 ) );
            ui_render::rect_filled( tab_position.left - 20, tab_position.top, 1, 27, color3 );
            g_render->text( tab_position.left, tab_position.top + 4, g_render->fonts.tabfont, t->m_title, color2.alpha( m_alpha ), false );

        }

        if ( !m_active_tab->m_elements.empty( ) ) {
            Rect el = GetElementsRect( m_scroll_pos );

            int numRectangles = 2; // Number of rectangles to render
            int spacing = 15; // Spacing between rectangles
            int rectWidth = 266; // Width of each rectangle

            for ( int i = 0; i < numRectangles; i++ ) {
                int x = m_x + 170 + i * ( rectWidth + spacing );
                int y = m_y + 68;

                color_t backround1 = hex_to_rgb( "#FF121212" );
                color_t outline1 = hex_to_rgb( "#FF1E1E1E" );
                color_t text1 = hex_to_rgb( "#FFE2E2E2" );

                color_t gradient_1_topbar1 = hex_to_rgb( "#FF1E1E1E" );
                color_t gradient_2_topbar1 = hex_to_rgb( "#FF151515" );
                color_t gradient_3_topbar1 = hex_to_rgb( "#FF0F0F0F" );

                ui_render::rect_filled( x, y + 20, rectWidth, el.h - 55, backround1.alpha( m_alpha ) );
                ui_render::gradient( x, y, rectWidth, 20, gradient_1_topbar.alpha( m_alpha ), gradient_2_topbar.alpha( m_alpha ) );

                // @documentation: this is the fading in group
                ui_render::gradient( x, y + 20, rectWidth, 7, gradient_3_topbar.alpha( m_alpha - 175 ), gradient_3_topbar.alpha( 0 ) );
                ui_render::gradient_v( x, y + 20, 7, el.h - 35 - 20, gradient_3_topbar.alpha( m_alpha - 175 ), gradient_3_topbar.alpha( 0 ) );
                ui_render::gradient_v( x + rectWidth - 7, y + 20, 7, el.h - 35 - 20, gradient_3_topbar.alpha( 0 ), gradient_3_topbar.alpha( m_alpha - 175 ) );
                ui_render::gradient( x, y + el.h - 35 - 10, rectWidth, 10, gradient_3_topbar.alpha( 0 ), gradient_3_topbar.alpha( m_alpha - 175 ) );

                // @documentation: we always do - 1 / + 1 on rects so it wont take from our size
                ui_render::rect( x - 1, y - 1, rectWidth, el.h - 35, outline.alpha( m_alpha ) );
            }

            // @todo:
            // add groupe1
            // add group 2 {
            //  - text
            // }

            // iterate elements to display.
            for ( const auto& e : m_active_tab->m_elements ) {

                // draw the active element last.
                if ( !e || ( m_active_element && e == m_active_element ) )
                    continue;

                if ( !e->m_show )
                    continue;

                // this element we dont draw.
                if ( !( e->m_flags & ElementFlags::DRAW ) )
                    continue;

                e->draw( );
            }

            // we still have to draw one last fucker.
            if ( m_active_element && m_active_element->m_show )
                m_active_element->draw( );
        }
    }
}