#include "idlib/precompiled.h"

idCVar net_ip("net_ip", "localhost", 0, "local IP address");

namespace ba = boost::asio;

/*
========================
NET_ErrorString
========================
*/
const char* NET_ErrorString(int code) {
	#ifdef _WIN32
	switch (code) {
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSAECONNABORTED";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	default: return "NO ERROR";
	}
	#else
	return "";
	#endif
}

/*
========================
Sys_CompareNetAdrBase

Compares without the port.
========================
*/
bool Sys_CompareNetAdrBase(const netadr_t a, const netadr_t b) {
	return std::make_tuple(a.type, a.address, a.port) == std::make_tuple(b.type, b.address, b.port);
}

/*
================================================================================================

	idUDP

================================================================================================
*/

/*
========================
idUDP::idUDP
========================
*/
idUDP::idUDP() {
	packetsRead = 0;
	bytesRead = 0;
	packetsWritten = 0;
	bytesWritten = 0;
}

/*
========================
idUDP::~idUDP
========================
*/
idUDP::~idUDP() {
	Close();
}

/*
========================
idUDP::InitForPort
========================
*/
bool idUDP::InitForPort(int portNumber) {
	const std::string net_interface = net_ip.GetString();

	if (portNumber != PORT_ANY) {
		idLib::Printf("Opening IP socket: %s:%i\n", net_interface.c_str(), portNumber);
	}

	boost::system::error_code ec;
	
	socket = std::make_unique<ba::ip::udp::socket>(io_context);
	
	socket->open(ba::ip::udp::v4(), ec);
	if (ec) {
		idLib::Printf("NET: Error socket opening: %d %s", ec.value(), NET_ErrorString(ec.value()));
		return false;
	}
	
	ba::ip::udp::endpoint ep;

	if(net_interface.empty() || net_interface == "localhost")
		ep = ba::ip::udp::endpoint(ba::ip::address_v4::any(), portNumber == PORT_ANY ? 0 : portNumber);
	else
		ep = ba::ip::udp::endpoint(boost::asio::ip::address::from_string(net_interface), portNumber == PORT_ANY ? 0 : portNumber);

	socket->bind(ep, ec);
	if (ec) {
		idLib::Printf("NET: Error socket bind to port: %d %d %s\n", ep.port(), ec.value(), NET_ErrorString(ec.value()));
		
		return false;
	}

	idLib::Printf("NET: Socket bound to port: %d\n", socket->local_endpoint().port());

	socket->non_blocking(true, ec);
	if (ec) {
		// if (ec.value() == WSAEWOULDBLOCK)
		// 	return false;

		idLib::Warning("NET: socket set to non-blocking FAIL: %d %s\n", ec.value(), NET_ErrorString(ec.value()));
		return false;
	}

	/*socket->set_option(boost::asio::socket_base::broadcast(true), ec);

	if (ec) {
		if (ec.value() == WSAEWOULDBLOCK)
			return false;

		idLib::Warning("NET: socket set to broadcast FAIL: %d %s\n", ec.value(), NET_ErrorString(ec.value()));
		return false;
	}
	else {
		idLib::Printf("NET: socket set to broadcast OK\n");
	}*/

	bound_to.address = ep.address();
	bound_to.port = ep.port();

	return true;
}

/*
========================
idUDP::Close
========================
*/
void idUDP::Close() {
	if (socket) {
		socket->close();
	}
	socket = nullptr;
}

/*
========================
idUDP::GetPacket
========================
*/
bool idUDP::GetPacket(netadr_t& from, void* data, int& size, int maxSize) {
	while (1) {
		boost::asio::ip::udp::endpoint sender_endpoint;
		boost::asio::socket_base::message_flags flags(0);
		boost::system::error_code ec;
		size = socket->receive_from(boost::asio::buffer(data, maxSize), sender_endpoint, flags, ec);

		if (ec) {
			#ifdef _WIN32
			if (ec.value() == WSAEWOULDBLOCK) {
				return false;
			}
			#endif
			
			idLib::Warning("NET: socket receive warning %d %s\n", ec.value(), NET_ErrorString(ec.value()));

			return false;
		}
		else {
			//idLib::Printf("NET: socket received %d bytes\n", size);
		}

		from.address = sender_endpoint.address();
		from.port = sender_endpoint.port();
		from.type = netadrtype_t::NA_IP;

		packetsRead++;
		bytesRead += size;

		break;
	}

	return true;
}

/*
========================
idUDP::SendPacket
========================
*/
void idUDP::SendPacket(const netadr_t to, const void* data, int size) {
	if (to.type == netadrtype_t::NA_BAD) {
		idLib::Warning("NET: idUDP::SendPacket: bad address type NA_BAD - ignored");
		return;
	}

	packetsWritten++;
	bytesWritten += size;

	boost::asio::ip::udp::endpoint destination(to.address, to.port);
	boost::system::error_code ec;
	size_t len = socket->send_to(boost::asio::buffer(data, size), destination, 0, ec);

	idassert(size == len);

	if (ec) {
		// if (ec.value() == WSAEADDRNOTAVAIL)
		// 	return;

		idLib::Printf("NET: UDP sendto error - packet dropped: %s\n", NET_ErrorString(ec.value()));

		idassert(0);
	}
	else {
		idLib::Printf("NET: socket sent %d bytes\n", len);
	}
}