#pragma once
#include "form.h"
#include "gui.hpp"
#include "checkbox.h"
#include "slider.h"
#include "dropdown.h"
#include "multidropdown.h"
#include "keybind.h"
#include "edit.h"
#include "button.h"
#include <iostream>
#include "colorpicker.h"
#include "notify.h"
#include "listbox.h"
#include "tiny_format.h"
void loaddd( ) {
	g_notify.add( tfm::format( "hit Dutu1337 for %s in stomach!", std::to_string(rand() % 100) ) );
}

class AimbotTab : public Tab {
public:
	Checkbox enable;
	Checkbox enable2;
	Checkbox enable3;
	Checkbox enable4;
	Checkbox enable5;
	Checkbox enable6;

	Slider fov_amount;
	Slider fov_amount2;
	Dropdown	  selection;
	ListBox	  listboxx;
	MultiDropdown hitbox;
	Keybind       baim_key;
	Edit     id_deagle;
	Button   load;
	Colorpicker menu_col;
public:
	void init( ) {
		// title.
		SetTitle( ( "RAGEBOT" ) );

		listboxx.setup( ( "TABSA" ), ( "asdast" ), { ( "Players" ), ( "Visuals" ), ( "World" ) } );
		RegisterElement( &listboxx );

		std::function<bool( )> show = [ = ]( ) -> bool {
			return listboxx.get() == 1;
		};

		enable.setup( ( "Enable" ), ( "enable" ) );
		enable.AddShowCallback( show );
		RegisterElement( &enable );
		enable2.setup( ( "Enable2" ), ( "enable" ) );
		enable2.AddShowCallback( show );
		RegisterElement( &enable2 );
		enable3.setup( ( "Enable3" ), ( "enable" ) );
		enable3.AddShowCallback( show );
		RegisterElement( &enable3 );
		enable4.setup( ( "Enable4" ), ( "enable" ) );
		RegisterElement( &enable4 );
		enable5.setup( ( "Enable5" ), ( "enable" ) );
		RegisterElement( &enable5 );
		enable6.setup( ( "Enable6" ), ( "enable" ) );
		RegisterElement( &enable6 );


		fov_amount2.setup( "Hitchance", ( "fov_amount" ), 1.f, 180.f, false, 0, 180.f, 1.f, ( L"°" ) );
		RegisterElement( &fov_amount2 );

		selection.setup( ( "Sorting" ), ( "selection" ), { ( "Distance" ), ( "Crosshair" ), ( "Damage" ), ( "Health" ), ( "Lag" ), ( "Height" ) } );
		RegisterElement( &selection );
	
		hitbox.setup( ( "Hitbox" ), ( "hitbox" ), { ( "Head" ), ( "Chest" ), ( "Body" ), ( "Arms" ), ( "Legs" ) } );
		RegisterElement( &hitbox );
		
		baim_key.setup( ( "Body aim on key" ), ( "body aim on key" ) );
		RegisterElement( &baim_key );
		
		id_deagle.setup( ( "Paintkit id" ), ( "id_deagle" ), 3 );
		RegisterElement( &id_deagle );
	
		load.setup( ( "load" ) );
		load.SetCallback( loaddd );
		RegisterElement( &load );

		menu_col.setup( ( "Menu color" ), ( "menu_color" ), { 255, 255, 0 }, &g_gui.m_color );
		RegisterElement( &menu_col );


	}
};

class AntiAimTab : public Tab {
public:
public:
	void init( ) {
		// title.
		SetTitle( ( "ANTI-AIM" ) );
	}
};


class PlayersTab : public Tab {
public:
public:
	void init( ) {
		// title.
		SetTitle( ( "PLAYERS" ) );
	}
};

class VisualsTab : public Tab {
public:
public:
	void init( ) {
		// title.
		SetTitle( ( "VISUALS" ) );
	}
};

class MovementTab : public Tab {
public:
public:
	void init( ) {
		// title.
		SetTitle( ( "MISC" ) );
	}
};

class SkinsTab : public Tab {
public:
public:
	void init( ) {
		// title.
		SetTitle( ( "SKINS" ) );
	}
};

class MiscTab : public Tab {
public:
public:
	void init( ) {
		// title.
		SetTitle( ( "OTHER" ) );
	}
};

class ConfigTab : public Tab {
public:
public:
	void init( ) {
		// title.
		SetTitle( ( "CONFIG" ) );
	}
};

class MainForm : public Form {
public:
	// aimbot.
	AimbotTab    aimbot;
	AntiAimTab   antiaim;

	// visuals.
	PlayersTab	 players;
	VisualsTab	 visuals;

	// misc.
	MovementTab  movement;
	SkinsTab     skins;
	MiscTab	     misc;
	ConfigTab	 config;

public:
	void init( ) {
		SetPosition( 550, 350 );
		SetSize( 733, 550 );

		// aim.
		RegisterTab( &aimbot );
		aimbot.init( );

		RegisterTab( &antiaim );
		antiaim.init( );

		// visuals.
		RegisterTab( &players );
		players.init( );

		RegisterTab( &visuals );
		visuals.init( );

		// misc.
		RegisterTab( &movement );
		movement.init( );

		RegisterTab( &skins );
		skins.init( );

		RegisterTab( &misc );
		misc.init( );

		RegisterTab( &config );
		config.init( );
	}
};

class Menu {
public:
	MainForm main;

public:
	void init( ) {
	

		main.init( );
		g_gui.RegisterForm( &main, VK_INSERT );
	}
};

extern Menu g_menu;