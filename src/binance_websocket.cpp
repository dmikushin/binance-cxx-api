/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#include "binance_websocket.h"
#include "binance_logger.h"

#include <atomic>

using namespace binance;
using namespace std;

static atomic<int> lws_service_cancelled(0);

struct lws_context *binance::Websocket::context = NULL;

struct lws_protocols protocol =
{
	.name = "binance-websocket-api",
	.callback = binance::Websocket::event_cb,
	.per_session_data_size = 0,
	.rx_buffer_size = 65536,
};

map<struct lws*, CB> binance::Websocket::handles;

int binance::Websocket::event_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	char errbuf[256];

	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED :
		lws_callback_on_writable(wsi);
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE :
		{
			// Handle incomming messages here.
			try
			{
				string str_result = string((char*)in);
				Json::Reader reader;
				Json::Value json_result;	
				reader.parse(str_result , json_result);

				if (handles.find(wsi) != handles.end())
					handles[wsi](json_result);
			}
			catch (exception &e)
			{
		 		Logger::write_log("<binance::Websocket::event_cb> Error parsing incoming message : %s\n", e.what());
		 		return 1;
			}
		}   	
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE :
		break;

	case LWS_CALLBACK_CLOSED :
		{
			if (handles.find(wsi) != handles.end())
				handles.erase(wsi);
	 	}
		goto cancel;
	case LWS_CALLBACK_GET_THREAD_ID:
		{
			return pthread_self();
		}
		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR :
		{
			if (handles.find(wsi) != handles.end())
				handles.erase(wsi);
	 		Logger::write_log("<binance::Websocket::event_cb> LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
	 	}
		goto cancel;
	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS :
		return 1;
	case LWS_CALLBACK_LOCK_POLL :
	case LWS_CALLBACK_ADD_POLL_FD :
	case LWS_CALLBACK_DEL_POLL_FD :
	case LWS_CALLBACK_UNLOCK_POLL :
	case LWS_CALLBACK_WSI_CREATE :
	case LWS_CALLBACK_WSI_DESTROY :
	case LWS_CALLBACK_CHANGE_MODE_POLL_FD :
	case LWS_CALLBACK_PROTOCOL_INIT :
	case LWS_CALLBACK_PROTOCOL_DESTROY :
		break;
	default :
		{
	 		Logger::write_log("<binance::Websocket::event_cb> unhandled callback reason = %d\n", reason);
		}
		goto cancel;
	}

	return 0;

cancel :

	atomic_store(&lws_service_cancelled, 1);
	return -1;
}

void binance::Websocket::init()
{
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = &protocol;
	info.gid = -1;
	info.uid = -1;

	// This option is needed here to imply LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT
	// option, which must be set on newer versions of OpenSSL.
	info.options = LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;

	context = lws_create_context(&info);
}

enum lws_client_connect_ssl_connection_flags
{
	LCCSCF_USE_SSL                          = (1 << 0),
	LCCSCF_ALLOW_SELFSIGNED                 = (1 << 1),
	LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK  = (1 << 2),
	LCCSCF_ALLOW_EXPIRED                    = (1 << 3),

	LCCSCF_PIPELINE                         = (1 << 16),
};

// Register call backs
void binance::Websocket::connect_endpoint(CB cb, const char* path)
{
	char ws_path[1024];
	strcpy(ws_path, path);
	
	// Connect if we are not connected to the server.
	struct lws_client_connect_info ccinfo = { 0 };
	ccinfo.context 	= context;
	ccinfo.address 	= BINANCE_WS_HOST;
	ccinfo.port 	= BINANCE_WS_PORT;
	ccinfo.path 	= ws_path;
	ccinfo.host 	= lws_canonical_hostname(context);
	ccinfo.origin 	= "origin";
	ccinfo.protocol = protocol.name;
	ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

	struct lws* conn = lws_client_connect_via_info(&ccinfo);
	handles[conn] = cb;
}

// Entering event loop
void binance::Websocket::enter_event_loop()
{
	while (1)
	{	
		lws_service(context, 500);

		if (lws_service_cancelled)
			break;
	}

	atomic_store(&lws_service_cancelled, 0);

	lws_context_destroy(context);
}

