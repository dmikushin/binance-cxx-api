/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#include "binance_websocket.h"
#include "binance_logger.h"

#include <atomic>
#include <libwebsockets.h>
#include <unordered_map>
#include <csignal>
#include <cassert>

using namespace binance;
using namespace std;

static struct lws_context *context;
static unordered_map<lws*, CB> handles;
static lws_sorted_usec_list_t _sul;
static atomic<int> lws_service_cancelled(0);
void connect_client(lws_sorted_usec_list_t *sul);

/*
 * This "contains" the endpoint connection proprty and has
 * the connection bound to it
 */
struct endpoint_connection {
	lws_sorted_usec_list_t	sul; /* schedule connection retry */
	struct lws		*wsi;	     /* related wsi if any */
	uint16_t		retry_count; /* count of consequetive retries */
	lws* conn;
	CB callback_jason_func;
	char* ws_path;
} endpoint_prop;

/*
 * The retry and backoff policy we want to use for our client connections
 */
const uint32_t backoff_ms[] = { 1000, 1000*2, 1000*3, 1000*4, 1000*5};

/*
 * This struct sets the policy for delays between retries,
 * and for how long a connection may be 'idle'
 * before it first tries to ping / pong on it to confirm it's up,
 * or drops the connection if still idle.
 */
const lws_retry_bo_t retry = {
	.retry_ms_table			= backoff_ms,
	.retry_ms_table_count		= LWS_ARRAY_SIZE(backoff_ms),
	.conceal_count			= LWS_ARRAY_SIZE(backoff_ms),
	.secs_since_valid_ping		= 30, /* force PINGs after secs idle */
	.secs_since_valid_hangup	= 60, /* hangup after secs idle */
	.jitter_percent			= 15,
	/*
	 * jitter_percent controls how much additional random delay is
     * added to the actual interval to be used, defult 30
	 */
};

/*
 * If we don't enable permessage-deflate ws extension, during times when there
 * are many ws messages per second the server coalesces them inside a smaller
 * number of larger ssl records, for >100 mps typically >2048 records.
 *
 * This is a problem, because the coalesced record cannot be send nor decrypted
 * until the last part of the record is received, meaning additional latency
 * for the earlier members of the coalesced record that have just been sitting
 * there waiting for the last one to go out and be decrypted.
 *
 * permessage-deflate reduces the data size before the tls layer, for >100mps
 * reducing the colesced records to ~1.2KB.
 */
const struct lws_extension extensions[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate"
		"; client_no_context_takeover"
		"; client_max_window_bits"
	},
	{ NULL, NULL, NULL /* terminator */ }
};

int event_cb(lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	struct endpoint_connection *endpoint_prop = (struct endpoint_connection *)user;

	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED :
			lwsl_user("%s: established\n", __func__);
			lws_callback_on_writable(wsi);
			endpoint_prop->wsi = wsi;
			break;

	case LWS_CALLBACK_CLIENT_RECEIVE :
		{
			// Handle incomming messages here.
			try
			{
				string str_result = string(reinterpret_cast<char*>(in), len);
				Json::Reader reader;
				Json::Value json_result;
				reader.parse(str_result , json_result);

				auto iter = handles.find(wsi);
        if (iter != handles.end())
					iter->second(json_result);
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
			goto do_retry;

	case LWS_CALLBACK_GET_THREAD_ID:
		{
#ifdef __APPLE__
			// On OS X pthread_threadid_np() is used, as pthread_self() returns a structure.
			// Note the _np suffix suggests that it is an extension to POSIX.
			uint64_t tid;
			pthread_threadid_np(NULL, &tid);
#else
			auto tid = pthread_self();
#endif
			return (int)(uint64_t)tid;
		}
			break;

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR :
			lwsl_err("CLIENT_CONNECTION_ERROR: %s\n",
					 in ? (char *)in : "(null)");
			if (handles.find(wsi) != handles.end())
				handles.erase(wsi);
			lws_cancel_service(lws_get_context(wsi));
			atomic_store(&lws_service_cancelled, 1);
			return -1;
			break;

		case LWS_CALLBACK_CLIENT_CLOSED:
			lwsl_err("CLIENT_CALLBACK_CLIENT_CLOSED: %s\n",
					 in ? (char *)in : "");
			goto do_retry;

		default :
			// Make compiler happy regarding unhandled enums.
			break;
	}

	return 0;

do_retry:
	try{
		if (lws_retry_sul_schedule_retry_wsi(wsi, &endpoint_prop->sul, connect_client,
											 &endpoint_prop->retry_count))
		{
			if (handles.find(wsi) != handles.end())
				handles.erase(wsi);
			lws_cancel_service(lws_get_context(wsi));
			lwsl_err("%s: connection attempts exhausted\n", __func__);
			atomic_store(&lws_service_cancelled, 1);
			return -1;
		}
	}catch (exception &e)
	{
		Logger::write_log("<binance::Websocket::event_cb> Error do_retry message : %s\n", e.what());
		atomic_store(&lws_service_cancelled, 1);
		return -1;
	}

	return 0;
}

const lws_protocols protocols[] =
	{
		{
			.name = "binance-websocket-api",
			.callback = event_cb,
			.per_session_data_size = 0,
			.rx_buffer_size = 65536,
		},

		{ NULL, NULL, 0, 0 } /* end */
	};

void
sigint_handler(int sig)
{
	Logger::write_log("<binance::Websocket::sigint_handler> Interactive attention signal : %d\n", sig);
	atomic_store(&lws_service_cancelled, 1);
}

/*
 * Scheduled sul callback that starts the connection attempt
 */
void connect_client(lws_sorted_usec_list_t *sul)
{
	struct endpoint_connection *endpoint_prop = lws_container_of(&_sul, struct endpoint_connection, sul);
	struct lws_client_connect_info ccinfo;

	memset(&ccinfo, 0, sizeof(ccinfo));

	ccinfo.context = context;
	ccinfo.port = BINANCE_WS_PORT;
	ccinfo.address = BINANCE_WS_HOST;
	ccinfo.path = endpoint_prop->ws_path;
	ccinfo.host = lws_canonical_hostname(context);
	ccinfo.origin = "origin";
	ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
	ccinfo.protocol = protocols[0].name;
	ccinfo.local_protocol_name = protocols[0].name;
	ccinfo.retry_and_idle_policy = &retry;
	ccinfo.userdata = endpoint_prop;
	/*
	 * We store the new wsi here early in the connection process,
	 * this gives the callback a way to identify which wsi faced the error
	 * even before the new wsi is returned and even if ultimately no wsi is returned.
	 */
	ccinfo.pwsi = &endpoint_prop->wsi;

	endpoint_prop->conn = lws_client_connect_via_info(&ccinfo);
	if (!endpoint_prop->conn)
	{
		/*
		 * Failed... schedule a retry... we can't use the _retry_wsi()
		 * convenience wrapper api here because no valid wsi at this
		 * point.
		 */
		if (lws_retry_sul_schedule(context, 0, sul, &retry,
								   connect_client, &endpoint_prop->retry_count))
		{
			lwsl_err("%s: connection attempts exhausted\n", __func__);
			atomic_store(&lws_service_cancelled, 1);
			assert(endpoint_prop->wsi);
			if ((endpoint_prop->wsi) && handles.find(endpoint_prop->wsi) != handles.end())
				handles.erase(endpoint_prop->wsi);
			return;
		}
		else{
			handles[endpoint_prop->conn] = endpoint_prop->callback_jason_func;
		}
	}else{
		handles[endpoint_prop->conn] = endpoint_prop->callback_jason_func;
	}
}

void binance::Websocket::init()
{
	struct lws_context_creation_info info;
	signal(SIGINT, sigint_handler);
	memset(&info, 0, sizeof(info));
	// This option is needed here to imply LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT
	// option, which must be set on newer versions of OpenSSL.
	info.options = LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;
	info.port = CONTEXT_PORT_NO_LISTEN;
	info.gid = -1;
	info.uid = -1;
	info.protocols = protocols;
	info.fd_limit_per_thread = 0;
	info.extensions = extensions;

	context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		atomic_store(&lws_service_cancelled, 1);
		return;
	} else{
		atomic_store(&lws_service_cancelled, 0);
	}
}

// Register call backs
void binance::Websocket::connect_endpoint(CB cb, const char* path)
{
	struct endpoint_connection *endpoint_prop = lws_container_of(&_sul, struct endpoint_connection, sul);
	endpoint_prop->ws_path = const_cast<char *>(path);
	endpoint_prop->callback_jason_func = cb;
	connect_client(&_sul);
	if (!lws_service_cancelled)
	/* schedule the first client connection attempt to happen immediately */
	lws_sul_schedule(context, 0, &endpoint_prop->sul, connect_client, 1);
}

// Entering event loop
void binance::Websocket::enter_event_loop(std::chrono::hours hours)
{
	auto start = std::chrono::steady_clock::now();
	auto end = start + hours;
	auto n = 0;
	do {
		n = lws_service(context, 1000);
		if (lws_service_cancelled)
		{
			lws_cancel_service(context);
			break;
		}
	} while (n >= 0 && std::chrono::steady_clock::now() < end);

	atomic_store(&lws_service_cancelled, 0);

	lws_context_destroy(context);
}
