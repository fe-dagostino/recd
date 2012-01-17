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


#include "RecdChannelFactory.h"
#include "IThreadFactory.h"

GENERATE_CLASSINFO( RecdChannelFactory, FChannelFactory )

// Threads coming from RCI runs in concurrency with render thread that are 
// all with TP_CRITICAL priority. Creating RCI thread with the same priority
// will force the scheduler to provide the same cpu time occupation avoiding
// RCI threads from waiting long time before to be served.
class RecdThreadFactory : public IThreadFactory
{
public:
  FThread*	Create( IRunnable* pRunnable )
  {
    return new FThread( NULL, pRunnable, FThread::TP_CRITICAL, 1024 );
  }
};

FChannel*	RecdChannelFactory::CreateChannel( IConnection* pIConnection ) const
{
  return new RecdChannel( pIConnection, new RecdThreadFactory() );
}




