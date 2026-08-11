#pragma once
#include "../../resources/main_includes.hpp"

#define COLORPICKER_WIDTH		20
#define COlORPICKER_HEIGHT		8
#define COLORPICKER_PICKER_SIZE 256

class Colorpicker : public Element {
public:
	__forceinline Colorpicker( ) : m_open{ false }, m_label{}, m_color{}, on_checkbx{}, m_ptr{ nullptr } {
		m_flags = ElementFlags::DRAW | ElementFlags::CLICK | ElementFlags::ACTIVE | ElementFlags::SAVE | ElementFlags::DEACIVATE;
		m_type = ElementTypes::COLORPICKER;
		m_h = m_base_h = 8;
		m_show = true;
	}

	__forceinline void setup( const std::string& label, const std::string& file_id, color_t color, color_t* ptr = nullptr ) {
		m_label = label;
		m_file_id = file_id;
		m_color = color;
		m_ptr = ptr;

		m_use_label = true;

		if ( m_ptr )
			*m_ptr = m_color;
	}

	__forceinline void set( color_t color ) {
		bool changed = m_color.u32( ) != color.u32( ); // im not sure if thats right

		m_color = color;

		if ( m_ptr )
			*m_ptr = m_color;

		if ( changed && m_callback )
			m_callback( );
	}

	__forceinline color_t get( ) {
		return m_color;
	}

	static void init( );

	static __forceinline color_t ColorFromPos( int x, int y ) {
		return *( color_t* )( gradient.get( ) + x + y * COLORPICKER_PICKER_SIZE );
	}

public:
	static int texture;
	static std::unique_ptr< color_t[ ] > gradient;

protected:
	vec2_t m_hue_pos;
	vec2_t m_alpha_pos;
	vec2_t ColorPos;
	bool		m_open;
	std::string m_label;
	bool on_checkbx;
	color_t		m_color;
	color_t* m_ptr;
	bool DoDragHue = false;
	bool DoDragCol = false;
	bool do_drag_alpha = false;
	float m_hue = 1.f;
	float m_saturation = 1.f;
	float m_value = 1.f;
	float m_alpha = 1.f;
	//vec2_t ColorPos;
	//vec2_t m_alpha_pos;

protected:
	void draw( ) override;
	void think( ) override;
	void click( ) override;
};
