#pragma once
#include "../../resources/main_includes.hpp"

#define BUTTON_X_OFFSET      0
#define BUTTON_BOX_HEIGHT    28
#define BUTTON_ITEM_X_OFFSET 10

class Button : public Element {
	friend class GUI;

public:
	__forceinline Button( ) : m_label{}, show_confirm_msg{ false }, should_confirm{true}, last_time_pressed{ 0.f } {
		m_flags = ElementFlags::DRAW | ElementFlags::CLICK;
		m_type = ElementTypes::BUTTON;
		m_base_h = m_h = BUTTON_BOX_HEIGHT;
		m_use_label = false;
		//show_confirm_msg = false;
		//last_time_pressed = 0.f;
	}

	__forceinline void setup( const std::string& label ) {
		m_label = label;
	}

protected:
	std::string m_label;
	bool show_confirm_msg;
	bool should_confirm;
	float last_time_pressed;
protected:
	void draw( ) override;
	void think( ) override;
	void click( ) override;
};