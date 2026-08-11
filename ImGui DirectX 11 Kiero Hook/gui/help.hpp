#pragma once
#include "../../resources/main_includes.hpp"
#include <sstream>

namespace h {
    __forceinline color_t hex_to_rgb( std::string hex_color ) {
        // Remove the '#' character if present
        if ( hex_color[ 0 ] == '#' ) {
            hex_color.erase( 0, 1 );
        }

        // Convert the hex string to an integer
        std::stringstream ss;
        ss << std::hex << hex_color;
        unsigned int hex_value;
        ss >> hex_value;

        // Extract the red, green, and blue values
        int red = ( hex_value >> 16 ) & 0xff;
        int green = ( hex_value >> 8 ) & 0xff;
        int blue = hex_value & 0xff;

        // Create a color_t object with the RGB values and return it
        return color_t( red, green, blue );
    }

	__forceinline float lerp( float a, float b, float t ) {
		return ( 1 - t ) * a + t * b;
	}
}