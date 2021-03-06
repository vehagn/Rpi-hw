/* 
    Title --- keypad/base.hpp

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


#ifndef _RPI_HW_KEYPAD_BASE_HPP_
#define _RPI_HW_KEYPAD_BASE_HPP_

#include <memory>
#include <vector>

#include <thread>
#include <mutex>

#include <rpi-hw/types.hpp>
#include <rpi-hw/exception.hpp>
#include <rpi-hw/time.hpp>

#include <rpi-hw/iface/base.hpp>
#include <rpi-hw/iface/input.hpp>

namespace rpihw { // Begin main namespace

/*!
	@namespace rpihw::keypad
	@brief Namespace of the keypads.
*/

namespace keypad { // Begin keypads namespace

/*!
	@class base
	@brief Generic keypad controller.
*/
class base {

public:

	/*!
		@brief Constructor method.
		@param[in] total Number of the buttons.
		@param[in] pins Sequence of `uint8_t` containing the input GPIOs.
	*/
	base( size_t total, std::initializer_list< uint8_t > pins );

	/*!
		@brief Constructor method.
		@param[in] total Number of the buttons.
		@param[in] pins Vector containing the input GPIO pins.
	*/
	base( size_t total, const std::vector< uint8_t > &pins );

	//! Destructor method.
	virtual ~base();

	/*!
		@brief Returns a button state.
		@param[in] index The index position of the input pin.
		@return The state of the button.
	*/
	virtual bool state( size_t index ) const;

	/*!
		@brief Checks if a button is pressed.
		@param[in] index The index position of the input pin.
		@return Return \c true if button is pressed.
	*/
	virtual bool pressed( size_t index ) const;

	/*!
		@brief Checks if a button is released.
		@param[in] index The index position of the input pin.
		@return Return \c true if button is released.
	*/
	virtual bool released( size_t index ) const;

	//! Returns the `std::vector< bool >` of button states.
	virtual const std::vector< bool > &state() const;

	//! Returns the number of keys.
	virtual size_t numOfKeys() const;

protected:

	//! Number of the keys.
	size_t m_nkeys;

	//! Buttons input interface.
	std::unique_ptr< iface::input > m_input;

	//! Button states (0 = up, 1 = down).
	std::vector< bool > m_keystate;

	//! Pressed buttons (0 = none, 1 = pressed).
	std::vector< bool > m_pressed;

	//! Pressed buttons (0 = none, 1 = released).
	std::vector< bool > m_released;

	//! Updating thread.
	std::unique_ptr< std::thread > m_thread;

	//! Mutex of the updating thread.
	std::unique_ptr< std::mutex > m_mutex;

	//! Updates the state of buttons (threading function).
	virtual void update();
};

} // End of keypads namespace

} // End of main namespace


// Include inline methods 
#include <rpi-hw/keypad/base-inl.hpp>

#endif /* _RPI_HW_KEYPAD_BASE_HPP_ */
