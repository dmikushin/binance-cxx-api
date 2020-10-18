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

using namespace binance;
using namespace std;

static struct lws_context *context;
static atomic<int> lws_service_cancelled(0);
void connect_client(lws_sorted_usec_list_t *sul);

/*
 * This "contains" the endpoint connection proprty and has
 * the connection bound to it
 */
struct endpoint_connection {
	lws_sorted_usec_list_t	_sul; /* schedule connection retry */
	struct lws		*wsi;	     /* related wsi if any */
	uint16_t		retry_count; /* count of consequetive retries */
	lws* conn;
	CB json_cb;
	char* ws_path;
};

static std::unordered_map<int,int> concurrent;
static std::unordered_map<int, endpoint_connection> endpoints_prop;
static pthread_mutex_t lock_concurrent; /* serialize access */

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
	int m;

	switch (reason)
	{
		case LWS_CALLBACK_CLIENT_ESTABLISHED :
			for (std::pair<int, int> n : concurrent) {
				if (endpoints_prop[n.second].wsi == wsi) {
					lws_callback_on_writable(wsi);
					m = n.second;
					endpoints_prop[m].wsi = wsi;
					lwsl_user("%s: connection established with success concurrent:%d ws_path::%s\n",
							 __func__, n.second, endpoints_prop[n.second].ws_path);
					break;
				}
			}
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

				for (std::pair<int, int> n : concurrent) {
					if (endpoints_prop[n.second].wsi == wsi) {
						m = n.second;
						endpoints_prop[n.second].json_cb(json_result);
						endpoints_prop[m].retry_count = 0;
						lwsl_user("%s: incomming messages from %s\n",
							__func__ , endpoints_prop[n.second].ws_path);
						break;
					}
				}
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
			lwsl_err("CALLBACK_CLOSED: %s\n",
					 in ? (char *)in : "");
			for (std::pair<int, int> n : concurrent) {
				if (endpoints_prop[n.second].wsi == wsi) {
					m = n.second;
					goto do_retry;
				}
			}
			break;

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
			for (std::pair<int, int> n : concurrent) {
				if (endpoints_prop[n.second].wsi == wsi) {
					atomic_store(&lws_service_cancelled, 1);
					endpoints_prop.erase(n.second);
					concurrent.erase(n.second);
					lwsl_err("CLIENT_CONNECTION_ERROR Unknown WIS: %s\n",
							 in ? (char *)in : "(null)");
					return -1;
				}
			}
			break;

		case LWS_CALLBACK_CLIENT_CLOSED:
			lwsl_err("CLIENT_CALLBACK_CLIENT_CLOSED: %s\n",
					 in ? (char *)in : "");
			for (std::pair<int, int> n : concurrent) {
				if (endpoints_prop[n.second].wsi == wsi) {
					m = n.second;
					goto do_retry;
				}
			}
			break;

		default :
			// Make compiler happy regarding unhandled enums.
			break;
	}

	return 0;

do_retry:
	try{
		if (lws_retry_sul_schedule_retry_wsi(endpoints_prop[m].wsi, &endpoints_prop[m]._sul, connect_client,
											 &endpoints_prop[m].retry_count))
		{
			if(endpoints_prop[m].retry_count > (LWS_ARRAY_SIZE(backoff_ms))){
				endpoints_prop.erase(m);
				concurrent.erase(m);
				atomic_store(&lws_service_cancelled, 1);
				return -1;
			}
			{
				lwsl_err("%s: connection attempts exhausted,we will keep retrying count:%d ws_path:%s\n",
						 __func__, endpoints_prop[m].retry_count, endpoints_prop[m].ws_path);
				atomic_store(&lws_service_cancelled, 0);
				lws_sul_schedule(lws_get_context(endpoints_prop[m].wsi), 0, &endpoints_prop[m]._sul, connect_client, 10 * LWS_US_PER_SEC);
				return 0;
			}
		}
		lwsl_user("%s: connection attempts success, retrying count:%d ws_path:%s\n",
				 __func__, endpoints_prop[m].retry_count, endpoints_prop[m].ws_path);
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
	for (std::pair<int, int> n : concurrent) {
		if (&endpoints_prop[n.second]._sul == sul) {
			lwsl_user("%s: success ws_path::%s\n",
					 __func__, endpoints_prop[n.second].ws_path);
			struct lws_client_connect_info ccinfo;

			memset(&ccinfo, 0, sizeof(ccinfo));

			ccinfo.context = context;
			ccinfo.port = BINANCE_WS_PORT;
			ccinfo.address = BINANCE_WS_HOST;
			ccinfo.path = endpoints_prop[n.second].ws_path;
			ccinfo.host = lws_canonical_hostname(context);
			ccinfo.origin = "origin";
			ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK | LCCSCF_PIPELINE;
			ccinfo.protocol = protocols[0].name;
			ccinfo.local_protocol_name = protocols[0].name;
			ccinfo.retry_and_idle_policy = &retry;
			ccinfo.userdata = &endpoints_prop[n.second];
			/*
			 * We store the new wsi here early in the connection process,
			 * this gives the callback a way to identify which wsi faced the error
			 * even before the new wsi is returned and even if ultimately no wsi is returned.
			 */
			ccinfo.pwsi = &endpoints_prop[n.second].wsi;

			endpoints_prop[n.second].conn = lws_client_connect_via_info(&ccinfo);
			if (!endpoints_prop[n.second].conn)
			{
				/*
				 * Failed... schedule a retry... we can't use the _retry_wsi()
				 * convenience wrapper api here because no valid wsi at this
				 * point.
				 */
				if (lws_retry_sul_schedule(context, 0, &endpoints_prop[n.second]._sul, &retry,
										   connect_client, &endpoints_prop[n.second].retry_count))
				{
					lwsl_err("%s: connection attempts exhausted\n", __func__);
					endpoints_prop.erase(n.second);
					concurrent.erase(n.second);
					atomic_store(&lws_service_cancelled, 1);
					return;
				}
			}
			break;
		}
	}

}

void binance::Websocket::init()
{
	pthread_mutex_init(&lock_concurrent, NULL);
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
	pthread_mutex_lock(&lock_concurrent);
	if(concurrent.size() > 1024){
		lwsl_err("%s: maximum of 1024 connect_endpoints reached,\n",
			__func__);
		pthread_mutex_unlock(&lock_concurrent);
		return;
	}
	int n = concurrent.size();
	concurrent.emplace(std::pair<int,int>(n,n));
	endpoints_prop[n].ws_path = const_cast<char *>(path);
	endpoints_prop[n].json_cb = cb;
	connect_client(&endpoints_prop[n]._sul);

	if (!lws_service_cancelled)
	{
		/* schedule the first client connection attempt to happen immediately */
		lws_sul_schedule(context, 0, &endpoints_prop[n]._sul, connect_client, 1);
		lwsl_user("%s: concurrent:%d ws_path::%s\n",
				 __func__, n, endpoints_prop[n].ws_path);
	}
	pthread_mutex_unlock(&lock_concurrent);
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

	concurrent.clear();
	atomic_store(&lws_service_cancelled, 1);

	lws_context_destroy(context);
	
	pthread_mutex_destroy(&lock_concurrent);
}
