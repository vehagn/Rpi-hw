/* 
    Title --- display/hd44780.cpp

    Copyright (C) 2013 Giacomo Trudu - wicker25[at]gmail[dot]com

    This file is part of Rpi-hw.

    Rpi-hw is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation version 3 of the License.

    Rpi-hw is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Rpi-hw. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _RPI_HW_DISPLAY_HD44780_CPP_
#define _RPI_HW_DISPLAY_HD44780_CPP_

#include <rpi-hw/display/hd44780.hpp>

namespace rpihw { // Begin main namespace

namespace display { // Begin displays namespace

hd44780::hd44780( uint8_t rs, uint8_t e,
				  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7 )

	: m_mode( new iface::output( { rs, e } ) )
	, m_data( new iface::output( { d4, d5, d6, d7 } ) ) {

}

hd44780::hd44780( uint8_t rs, uint8_t e,
				  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
				  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7 )

	: m_mode( new iface::output( { rs, e } ) )
	, m_data( new iface::output( { d0, d1, d2, d3, d4, d5, d6, d7 } ) ) {
}

hd44780::~hd44780() {

}

void
hd44780::init( uint8_t cols, uint8_t rows, bool font ) {

	// Store the size of the display
	m_screen_w = cols;
	m_screen_h = rows;

	// Limit the number of rows
	rows = math::min< uint8_t >( rows, 4 );

	// Set the address locations
	// see: http://web.alfredstate.edu/weimandn/lcd/lcd_addressing/lcd_addressing_index.html
	if ( rows == 4 && cols == 16 )
		m_lines = {{ 0x00, 0x40, 0x10, 0x50 }};
	else
		m_lines = {{ 0x00, 0x40, 0x14, 0x54 }};

	// Store the font size
	m_font_height = ( font ? 10 : 8 );

	// Create a new character buffer
	m_buffer.assign( m_screen_w * m_screen_h, ' ' );

	// Set the initial cursor position
	move( 0, 0 );

	// Wait 20ms for LCD to power up
	time::msleep( 20 );

	// Initialize the display
	uint8_t flags = FUNC | FUNC_DL;

	// If required, enable the 4-bit mode
	if ( m_data->numOfPins() == 4 ) {

		flags >>= 4;

		send( flags, 5000 );
		send( flags, 200 );
		send( flags, 200 );

		flags = FUNC;
		send( flags >> 4, 5000 );

	} else {

		send( flags, 5000 );
		send( flags, 200 );
		send( flags, 200 );
	}

	// Apply settings
	if ( m_screen_h > 1 )		flags |= FUNC_N;
	if ( m_font_height == 10 )	flags |= FUNC_F; 

	cmd( flags );
	cmd( POWER | POWER_D );
	cmd( ENTRY | ENTRY_ID );
	cmd( CLEAR );

	setAutoscroll( VSCROLL );
}

void
hd44780::strobe() {

	// Toggle the enable pin
	m_mode->write( 1, 1 );
	time::usleep( 50 );
	m_mode->write( 1, 0 );
}

void
hd44780::send( uint8_t data, size_t delay ) {

	// Send nibble to the display
	m_data->write( data );

	// Toggle the enable pin
	strobe();

	// Wait some time
	time::usleep( (size_t) delay );
}

void
hd44780::sendSerial( uint8_t data, size_t delay ) {

	// If 4-bit mode is used, send high nibble to the display
	if ( m_data->numOfPins() == 4 )
		send( data >> 4, delay );

	// Send rest of data to the display
	send( data, delay );
}

void
hd44780::putChar( uint8_t chr ) {

	// Set RS value to high
	m_mode->write( 0, 1 );

	// Send the character to the display
	sendSerial( chr, 200 );

	// Store the character into the buffer
	m_buffer[ m_pos_x + m_pos_y * m_screen_w ] = chr;

	// Update the cursor position
	++m_pos_x;
}

void
hd44780::home() {

	// Home the cursor
	cmd( HOME );

	// Update the cursor position
	m_pos_x = m_pos_y = 0;;
}

void
hd44780::move( uint8_t x, uint8_t y ) {

	// Check if position exists
	if ( x >= m_screen_w || y >= m_screen_h )
		throw exception( utils::format( "(Error) `hd44780::move`: the position (%u,%u) does not exists\n", x, y ) );

	// Update the cursor position
	m_pos_x = x;
	m_pos_y = y;

	// Set the position of the cursor on the display
	cmd( DDRAM | ( m_lines[y] + x ) );
}

void
hd44780::newLine() {

	// Check if there are still lines available
	if ( m_pos_y < m_screen_h - 1 )
		move( 0, m_pos_y + 1 );

	// Scroll up the contents and move the cursor to the new line
	else if ( m_autoscroll & VSCROLL ) {

		move( 0, m_pos_y );
		scrollUp( false );

	// Clear the display
	} else clear();
}

void
hd44780::write( uint8_t chr ) {

	// Check if there are no still spaces available in the row
	if ( isLineFull() ) {

		// Make new space for the character
		if ( m_autoscroll & HSCROLL )
			scrollLeft();

		else if ( m_autoscroll & HSCROLL_LINE )
			scrollLeftLine( m_pos_y );

		else newLine();

		// Write the character on the display, but ignore other new line character
		if ( chr != '\n' ) putChar( chr );

	} else {

		// Write the character on the display
		putChar( chr );
	}
}

void
hd44780::write( const std::string &text, size_t delay ) {

	// Iterators
	std::string::const_iterator it = text.begin(), end = text.end();

	// Write the character on the display
	for ( ; it != end; ++it ) {

		// Parse newline characters
		if ( *it == '\n' ) {

			// Move the cursor to the new line
			newLine();

		} else {

			// Write the character on the display and wait some time
			write( (uint8_t) *it );

			if ( *it != ' ' )
				time::msleep( delay );
		}
	}
}

void
hd44780::scrollLeft( bool cursor ) {

	// Hardware: cmd( SHIFT | SHIFT_SC );

	// Store the cursor position
	uint8_t x = m_pos_x - 1, y = m_pos_y;

	// Iterator
	uint8_t i = 0;

	// Move left the contents and delete the first column
	for ( ; i < m_buffer.size(); ++i ) {

		if ( i % m_screen_w == 0 )
			move( 0, i / m_screen_w );

		putChar( ( i + 1 ) % m_screen_w != 0 ? m_buffer[ i + 1 ] : ' ' );
	}

	// Restore the cursor position
	move( x, y );
}

void
hd44780::scrollRight( bool cursor ) {

	// Hardware: cmd( SHIFT | SHIFT_SC | SHIFT_RL );

	// Store the cursor position
	uint8_t x = m_pos_x + (uint8_t) cursor,
			y = m_pos_y;

	// Copy the current character buffer
	std::vector< uint8_t > tmp = m_buffer;

	// Iterator
	uint8_t i = 0;

	// Move right the contents and delete the first column
	for ( ; i < m_buffer.size(); ++i ) {

		if ( i % m_screen_w == 0 )
			move( 0, i / m_screen_w );

		putChar( i % m_screen_w != 0 ? tmp[ i - 1 ] : ' ' );
	}

	// Restore the cursor position
	move( x, y );
}

void
hd44780::scrollUp( bool cursor ) {

	// Store the cursor position
	uint8_t x = m_pos_x,
			y = m_pos_y - (uint8_t) cursor;

	// Iterator
	uint8_t i = 0;

	// Move up the contents and delete the last row
	for ( ; i < m_buffer.size() - m_screen_w; ++i ) {

		if ( i % m_screen_w == 0 )
			move( 0, i / m_screen_w );

		putChar( m_buffer[ i + m_screen_w ] );
	}

	move( 0, m_pos_y + 1 );

	for ( ; i < m_buffer.size(); ++i ) putChar( ' ' );

	// Restore the cursor position
	move( x, y );
}

void
hd44780::scrollDown( bool cursor ) {

	// Store the cursor position
	uint8_t x = m_pos_x,
			y = m_pos_y + (uint8_t) cursor;

	// Copy the current character buffer
	std::vector< uint8_t > tmp = m_buffer;

	// Move the cursor to the first position
	move( 0, 0 );

	// Iterator
	uint8_t i = 0;

	// Move down the contents and delete the first row
	for ( ; i < m_screen_w; ++i )
		putChar( ' ' );

	for ( ; i < m_buffer.size(); ++i ) {

		if ( i % m_screen_w == 0 )
			move( 0, i / m_screen_w );

		putChar( tmp[ i - m_screen_w ] );
	}

	// Restore the cursor position
	move( x, y );
}

void
hd44780::scrollLeftLine( uint8_t line, bool cursor ) {

	// Store the cursor position
	uint8_t x = m_pos_x - (uint8_t) cursor,
			y = m_pos_y;

	// Iterator
	uint8_t i;

	// Scroll the line to the left
	move( 0, line );

	for ( i = 0; i < m_screen_w - 1; ++i )
		putChar( m_buffer[ line * m_screen_w + i + 1 ] );

	putChar( ' ' );

	// Restore the cursor position
	move( x, y );
}

void
hd44780::scrollRightLine( uint8_t line, bool cursor ) {

	// Store the cursor position
	uint8_t x = m_pos_x + (uint8_t) cursor,
			y = m_pos_y;

	// Copy the current character buffer
	std::vector< uint8_t > tmp = m_buffer;

	// Iterator
	uint8_t i;

	// Scroll the line to the right
	move( 0, line );

	putChar( ' ' );

	for ( i = 1; i < m_screen_w; ++i )
		putChar( tmp[ line * m_screen_w + i - 1 ] );

	// Restore the cursor position
	move( x, y );
}

void
hd44780::defChar( uint8_t index, const uint8_t *data ) {

	// Check if custom character
	if ( index > 7 )
		throw exception( "(Error) `hd44780::defChar`: could not define new custom character, index out of range\n" );

	// Start definition of the custom character
	cmd( CGRAM | ( index * 8 ) );

	// Set RS value to high
	m_mode->write( 0, 1 );

	// Iterator
	uint8_t i = 0;

	// Define the custom character
	for ( ; i < m_font_height; ++i )
		sendSerial( data[i], 5000 );

	// Restore the cursor position
	move( m_pos_x, m_pos_y );
}

void
hd44780::setCursor( uint8_t mode ) {

	// Set the cursor mode
	uint8_t flags = POWER | POWER_D;

	if ( mode & CURSOR_SOLID )		flags |= POWER_C; 
	if ( mode & CURSOR_BLINKING )	flags |= POWER_B; 

	cmd( flags );
}

void
hd44780::clear() {

	// Clear the display
	cmd( CLEAR );

	// Update the cursor position
	m_pos_x = m_pos_y = 0;

	// Clear the character buffer
	m_buffer.assign( m_buffer.size(), ' ' );
}

} // End of displays namespace

} // End of main namespace

#endif /* _RPI_HW_DISPLAY_HD44780_CPP_ */
