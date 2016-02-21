/****************************************************************
 *
 * Copyright (C) 2013 Alessandro Pignotti <alessandro@leaningtech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ***************************************************************/

#ifndef _CHEERP_CONNECTION_H
#define _CHEERP_CONNECTION_H

#define BOOST_NO_CXX11_NOEXCEPT
#define BOOST_ASIO_DISABLE_MOVE
#include <pion/http/plugin_server.hpp>
#include <pion/http/response_writer.hpp>

// This file contains the backend specific connection data
// Currently we assume pion
namespace cheerp
{

class SerializationInterface
{
protected:
	enum { BUFFER_SIZE = 128 };
	char buffer[BUFFER_SIZE];
	uint32_t offset;
public:
	SerializationInterface():offset(0)
	{
	}
	virtual void flush() = 0;
	void write(const char* buf, uint32_t length)
	{
		while(length)
		{
			uint32_t cur = (length<(128-offset))?length:128-offset;
			memcpy(buffer+offset, buf, cur);
			buf+=cur;
			length-=cur;
			offset+=cur;
			if(length)
				flush();
		}
	}
};

class Connection: public SerializationInterface
{
public:
	const pion::http::response_writer_ptr& writer;
	Connection(const pion::http::response_writer_ptr &writer) : writer(writer)
	{
	}

	void flush()
	{
		writer->write(buffer, offset);
		offset=0;
	}
	void send()
	{
		writer->send();
	}
};

class Server : public pion::http::plugin_server {
public:
	Server(const unsigned int tcp_port) : pion::http::plugin_server(tcp_port) {
	}
	virtual ~Server() {
	}
};

extern Connection* connection;

extern Server* server;

}
#endif
