/****************************************************************
 *
 * Copyright (C) 2012-2016 Alessandro Pignotti <alessandro@leaningtech.com>
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

#define BOOST_NO_CXX11_NOEXCEPT
//#define BOOST_ASIO_DISABLE_MOVE
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <pion/error.hpp>
#include <pion/process.hpp>
#include <pion/http/server.hpp>
#include <pion/http/response.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/http/plugin_server.hpp>
#include <pion/scheduler.hpp>
#include <pion/http/types.hpp>

#include "cheerp/server.h"
#include "cheerp/promise.h"
#include "cheerp/connection.h"

using namespace pion;
using namespace std;

namespace cheerp
{
	Connection* connection;
	Server* server;
}

typedef cheerp::PromiseBase* (*entryPointSig)(cheerp::SerializationInterface* inData, const char* outData);

struct CheerpMap
{
	char* funcName;
	entryPointSig entryPoint;
};

extern CheerpMap cheerpFuncMap[];

void requestHandler(const http::request_ptr http_request_ptr, const tcp::connection_ptr tcp_conn)
{
	const string& callName=http_request_ptr->get_query("f");
	const string& callArgs=pion::algorithm::url_decode(http_request_ptr->get_query("a"));
	entryPointSig callFunc=NULL;
	for(CheerpMap* cur=cheerpFuncMap; cur->funcName!=NULL; cur++)
	{
		if(callName==cur->funcName)
		{
			cout << "Found " << cur->funcName << endl;
			callFunc=cur->entryPoint;
			break;
		}
	}
	if(callFunc==NULL)
	{
		cout << "Invalid call " << callName << endl;
		http::response http_response(*http_request_ptr);
		http_response.set_status_code(400);
		http_response.set_status_message(http::types::RESPONSE_MESSAGE_BAD_REQUEST);
		boost::system::error_code error_code;
		http_response.send(*tcp_conn, error_code);
		tcp_conn->finish();
		return;
	}

	//tcp_conn->set_lifecycle(pion::tcp::connection::LIFECYCLE_CLOSE);
	cheerp::connection = new cheerp::Connection(
		http::response_writer_ptr(http::response_writer::create(const_cast<tcp::connection_ptr&>(tcp_conn),
		                                                        *http_request_ptr,
		                                                        boost::bind(&tcp::connection::finish, tcp_conn))));
	//Send the data using the serialization interface
	cheerp::PromiseBase* promise=callFunc(cheerp::connection, callArgs.c_str());
	if(!promise)
	{
		cheerp::connection->flush();
		cheerp::connection->send();
	}
}

int main ()
{
	// initialize signal handlers, etc.
	process::initialize();

	// initialize log system (use simple configuration)
	logger main_log(PION_GET_LOGGER("cheerpserver"));
	logger pion_log(PION_GET_LOGGER("pion"));
	PION_LOG_SETLEVEL_INFO(main_log);
	PION_LOG_SETLEVEL_INFO(pion_log);
	PION_LOG_CONFIG_BASIC;

	try {
		// create a new server to handle the Hello TCP protocol
		single_service_scheduler sched;
		sched.set_num_threads(10);
		http::plugin_server_ptr server(new cheerp::Server(sched, 1987));
		//http::plugin_server_ptr server(new pion::http::plugin_server(static_cast<scheduler &>(sched), 1987));
		// TODO: wrap this stuff with 'if path exists', because they yield fatal errors
		//plugin::add_plugin_directory("/usr/lib/pion/plugins"); // default location package in Ubuntu 15.10
		plugin::add_plugin_directory("/usr/local/share/pion/plugins/"); // default location manual building 5.0.6 from source
		server->load_service("/", "FileService.so");
		server->set_service_option("/", "directory", ".");
		server->add_resource("/cheerp_call", boost::bind(&requestHandler, _1, _2));
		server->start();
		process::wait_for_shutdown();

	} catch (std::exception& e) {
		PION_LOG_FATAL(main_log, pion::diagnostic_information(e));
	}

	return 0;
}

namespace pion {    // begin namespace pion
namespace http {    // begin namespace http
const std::string   types::HEADER_CONNECTION("Connection");
const std::string   types::HEADER_CONTENT_LENGTH("Content-Length");
const std::string   types::HEADER_NAME_VALUE_DELIMITER(": ");
const std::string   types::HEADER_SET_COOKIE("Set-Cookie");
const std::string   types::HEADER_TRANSFER_ENCODING("Transfer-Encoding");
const std::string   types::REQUEST_METHOD_HEAD("HEAD");
const std::string   types::RESPONSE_MESSAGE_BAD_REQUEST("Bad Request");
const std::string   types::RESPONSE_MESSAGE_OK("OK");
const std::string   types::STRING_CRLF("\x0D\x0A");
const std::string   types::STRING_EMPTY;
const std::string   types::STRING_HTTP_VERSION("HTTP/");
}
}
