#pragma once
#include "form.h"

// modelled after the original valve 'developer 1' debug console
// https://github.com/LestaD/SourceEngine2007/blob/master/se2007/engine/console.cpp

class NotifyText {
public:
	std::string m_text;
	color_t		m_color;
	float		m_time;
	float previous_y{};
public:
	__forceinline NotifyText( const std::string& text, color_t color, float time ) : m_text{ text }, m_color{ color }, m_time{ time } {}
};

class Notify {
private:
	std::vector< std::shared_ptr< NotifyText > > m_notify_text;

public:
	__forceinline Notify( ) : m_notify_text{} {}

	__forceinline void add( const std::string& text, color_t color = color_t( 255, 255, 255 ), float time = 8.f, bool console = true ) {
		m_notify_text.push_back( std::make_shared<NotifyText>( text, color, time ) );

		if ( m_notify_text.size( ) > 1 ) {
			m_notify_text.back( )->previous_y = m_notify_text[ m_notify_text.size( ) - 2 ]->previous_y;
		}
	}

	__forceinline float lerp2( float a, float b, float t ) {
		return ( 1 - t ) * a + t * b;
	}

	__forceinline float ease_in_quadratic( float t )
	{
		return t * t;
	}

	// modelled after 'CConPanel::DrawNotify' and 'CConPanel::ShouldDraw'
    void think( )
    {
        int x = 8;
        int y = 5;
        int size = 15;
        float animation_time = 0.f;
        const float duration = 1.f;
        const int start_alpha = 0;
        const int target_alpha = 255;

        // Update lifetimes and remove expired notifications.
        for ( size_t i = 0; i < m_notify_text.size( ); ++i ) {
            auto& notify = m_notify_text[ i ];
            notify->m_time -= 0.03f;

            if ( notify->m_time <= 0.f ) {
                m_notify_text.erase( m_notify_text.begin( ) + i );
                --i;
            }
        }

        // Draw notifications with fade-in animation.
        for ( size_t i = 0; i < m_notify_text.size( ); ++i ) {
            auto& notify = m_notify_text[ i ];
            float alpha = notify->m_time * ( target_alpha - start_alpha ) / duration;
            alpha = std::fmax( std::fmin( alpha, ( float )target_alpha ), ( float )start_alpha );
            ui_render::string( x, y, notify->m_color.manage_alpha( ( int )alpha ), notify->m_text, g_render->fonts.test_font2 );
            y += size + 5;
        }

       // // Check if there is a new notification to add.
       // if ( !m_new_notify_text.empty( ) ) {
       //     // Add new notification to list with starting alpha of 0.
       //     m_notify_text.emplace_back( std::move( m_new_notify_text.front( ) ), 0.f, color_t(255, 255, 255) );
       //     m_new_notify_text.pop_front( );
       //     animation_time = 0.f;
       // }

        // Animate the alpha of the most recent notification.
        if ( !m_notify_text.empty( ) ) {
            auto& notify = m_notify_text.back( );
            float t = animation_time / duration;
            int alpha = ( int )lerp2( ( float )start_alpha, ( float )target_alpha, t );
            notify->m_time += 0.03f;
            notify->m_time = std::fmin( notify->m_time, duration );
            notify->m_color = color_t( 255, 255, 255 );
            notify->m_color.set_alpha( alpha );
            animation_time += 0.016f;

            // If the animation is complete, reset the animation time.
            if ( animation_time > duration ) {
                animation_time = 0.f;
            }
        }
    }
};

extern Notify g_notify;