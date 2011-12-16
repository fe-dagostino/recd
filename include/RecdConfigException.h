/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2011  Francesco Emanuele D'Agostino <fedagostino@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef __RECD_CONFIG_EXCEPTION_H__
#define __RECD_CONFIG_EXCEPTION_H__

#include "FException.h"

USING_NAMESPACE_FED

/***/
class RecdConfigException : public FException
{
	ENABLE_FRTTI( RecdConfigException )
public:
	/**
	 * Exceptions types.
	 */
	enum EXCEPTION_CODES {
                                   OBJECT_NOT_INITIALIZED,
                                   OBJECT_ALREADY_INITIALIZED,
                                   NOT_ENOUGH_MEMORY,
                                   CFG_FILE_NOT_FOUND_OR_INACCESSIBLE
                           };

	/**
	 * Constructor.
	 * Require only a single parameter.
	 * @param nErrorCode is and exception type code.
	 */
	RecdConfigException( DWORD nErrorCode );

	/**
	 * Constructor.
	 * Require two parameters.
	 * @sErrorMsg error message
	 * @param nErrorCode is and exception type code.
	 */
	RecdConfigException( const FString& sErrorMsg, DWORD nErrorCode );

	/** Destructor */
	~RecdConfigException();

protected:

private:
	/***/
	VOID		AddMessageCodeToErrorMsg( DWORD nErrorCode );

};


#endif

