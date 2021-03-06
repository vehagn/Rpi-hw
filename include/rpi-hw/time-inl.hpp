/* 
    Title --- time-inl.hpp

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


#ifndef _RPI_HW_TIME_INL_HPP_
#define _RPI_HW_TIME_INL_HPP_

namespace rpihw { // Begin main namespace

namespace time { // Begin time namespace

inline void
sleep( size_t seconds ) {

	// Wait some seconds
	::usleep( (useconds_t) ( seconds * 1000000 ) );
}

inline void
msleep( size_t mseconds ) {

	// Wait some milliseconds
	::usleep( (useconds_t) ( mseconds * 1000 ) );
}

inline void
usleep( size_t useconds ) {

	// Wait some microseconds
	::usleep( (useconds_t) useconds );
}

} // End of times namespace

} // End of main namespace

#endif /* _RPI_HW_TIME_INL_HPP_ */
