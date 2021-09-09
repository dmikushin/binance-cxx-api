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
#include <assert.h>
//#include <cassert>
//#include <iostream>

#define ALWAYS_INLINE inline __attribute__((__always_inline__))

using namespace binance;
using namespace std;

static struct lws_context *context;
static atomic<int> lws_service_cancelled(0);
static void connect_client(lws_sorted_usec_list_t *sul);
static int force_create_ccinfo(const int &idx);
/*
 * This "contains" the endpoint connection property and has
 * the connection bound to it
 */
struct endpoint_connection {
  lws_sorted_usec_list_t _sul; /* schedule connection retry */
  struct lws *wsi;         /* related wsi if any */
  uint16_t retry_count; /* count of consecutive retries */
  lws *conn;
  CB json_cb;
  char *ws_path;
};

static std::unordered_map<int, endpoint_connection> endpoints_prop;
static pthread_mutex_t lock_concurrent; /* serialize access */

/*
 * The retry and backoff policy we want to use for our client connections
 */
static const uint32_t backoff_ms[] = {1000 * 4, 1000 * 5, 1000 * 6, 1000 * 7, 1000 * 8, 1000 * 9};

/*
 * This struct sets the policy for delays between retries,
 * and for how long a connection may be 'idle'
 * before it first tries to ping / pong on it to confirm it's up,
 * or drops the connection if still idle.
 */
static const lws_retry_bo_t retry = {
    .retry_ms_table            = backoff_ms,
    .retry_ms_table_count        = LWS_ARRAY_SIZE(backoff_ms),
    .conceal_count            = LWS_ARRAY_SIZE(backoff_ms),
    .secs_since_valid_ping        = 60, /* force PINGs after secs idle */
    .secs_since_valid_hangup    = 60, /* hangup after secs idle */
    .jitter_percent            = 30,
    /*
     * jitter_percent controls how much additional random delay is
     * added to the actual interval to be used, default 30
     */
};

static int event_cb(lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
  std::atomic<int> idx(-1);
  auto *current_data = static_cast< endpoint_connection *>(user);

  switch (reason) {
  case LWS_CALLBACK_PROTOCOL_INIT:
    lwsl_user("%s: LWS_CALLBACK_PROTOCOL_INIT\n", __func__);
    for (std::pair<int, endpoint_connection> n : endpoints_prop) {
      if(endpoints_prop[n.first].ws_path != NULL){
        idx = n.first;
        force_create_ccinfo(idx.load());
      }
    }
    break;

  case LWS_CALLBACK_CLIENT_ESTABLISHED :
    for (std::pair<int, endpoint_connection> n : endpoints_prop) {
      if (endpoints_prop[n.first].ws_path != NULL && endpoints_prop[n.first].wsi == wsi && strcmp(current_data->ws_path, endpoints_prop[n.first].ws_path) == 0) {
        pthread_mutex_lock(&lock_concurrent);
        idx = n.first;
        lws_callback_on_writable(wsi);
        endpoints_prop[idx.load()].wsi = wsi;
        lwsl_user("%s: connection established with success endpoint#:%d ws_path::%s\n",
                  __func__, idx.load(), endpoints_prop[idx.load()].ws_path);
        pthread_mutex_unlock(&lock_concurrent);
        break;
      }
    }
    break;

		case LWS_CALLBACK_CLIENT_RECEIVE :
		{
			// Handle incoming messages here.
			try
			{
      for (std::pair<int, endpoint_connection> n : endpoints_prop) {
        if (endpoints_prop[n.first].ws_path != NULL && endpoints_prop[n.first].wsi == wsi && strcmp(current_data->ws_path, endpoints_prop[n.first].ws_path) == 0) {
          pthread_mutex_lock(&lock_concurrent);
          string str_result = string(reinterpret_cast<const char *>(in), len);
          Json::Value json_result;
          JSONCPP_STRING err;
          Json::CharReaderBuilder builder;
          const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
          if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
                             &err)) {
            lwsl_user("%s: LWS_CALLBACK_CLIENT_RECEIVE Error Json:%s\n",
                      __func__, err.c_str());
            json_result.clear();
            pthread_mutex_unlock(&lock_concurrent);
            break;
          }
          idx = n.first;
          endpoints_prop[idx.load()].json_cb(json_result);
          endpoints_prop[idx.load()].retry_count = 0;
          json_result.clear();
          pthread_mutex_unlock(&lock_concurrent);
          break;
        }
      }
    }
    catch (exception &e) {
      pthread_mutex_unlock(&lock_concurrent);
      Logger::write_log("<binance::Websocket::event_cb> Error parsing incoming message : %s\n", e.what());
      lwsl_err("%s: Error parsing incoming message %s\n", __func__, e.what());
      return 1;
    }
  }
    break;

  case LWS_CALLBACK_CLIENT_WRITEABLE : break;

  case LWS_CALLBACK_CLOSED :
    for (std::pair<int, endpoint_connection> n : endpoints_prop) {
      if (endpoints_prop[n.first].ws_path != NULL && endpoints_prop[n.first].wsi == wsi && strcmp(current_data->ws_path, endpoints_prop[n.first].ws_path) == 0) {
        pthread_mutex_lock(&lock_concurrent);
        idx = n.first;
        lwsl_user("CALLBACK_CLOSED: %s\n",
                  in ? (char *) in : "");
        pthread_mutex_unlock(&lock_concurrent);
        goto do_retry;
      }
    }
    break;

  case LWS_CALLBACK_GET_THREAD_ID: {
#ifdef __APPLE__
    // On OS X pthread_threadid_np() is used, as pthread_self() returns a structure.
    // Note the _np suffix suggests that it is an extension to POSIX.
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
#else
    auto tid = pthread_self();
#endif
    return (int) (uint64_t) tid;
  }
    break;

  case LWS_CALLBACK_CLIENT_CONNECTION_ERROR :
    for (std::pair<int, endpoint_connection> n : endpoints_prop) {
      if (endpoints_prop[n.first].ws_path != NULL && endpoints_prop[n.first].wsi == wsi && strcmp(current_data->ws_path, endpoints_prop[n.first].ws_path) == 0) {
        pthread_mutex_lock(&lock_concurrent);
        idx = n.first;
        endpoints_prop[idx.load()].wsi = NULL;
        endpoints_prop[idx.load()].retry_count++;
        if (force_create_ccinfo(idx.load()) || endpoints_prop[idx.load()].retry_count > 2 * (LWS_ARRAY_SIZE(backoff_ms))) {
          lwsl_err("CLIENT_CONNECTION_ERROR: %s : %s\n",
                   in ? (char *) in : "(null)", current_data->ws_path);
          endpoints_prop[idx.load()].wsi = NULL;
          endpoints_prop[idx.load()].json_cb = NULL;
          endpoints_prop[idx.load()].retry_count = 0;
          endpoints_prop[idx.load()].conn = NULL;
          lws_cancel_service(lws_get_context(wsi));
          lws_context_destroy(lws_get_context(wsi));
          atomic_store(&lws_service_cancelled, 1);
          pthread_mutex_unlock(&lock_concurrent);
          return -1;
        }
        pthread_mutex_unlock(&lock_concurrent);
        break;
      }
    }
    break;

  case LWS_CALLBACK_CLIENT_CLOSED:
    for (std::pair<int, endpoint_connection> n : endpoints_prop) {
      if (endpoints_prop[n.first].ws_path != NULL && endpoints_prop[n.first].wsi == wsi && strcmp(current_data->ws_path, endpoints_prop[n.first].ws_path) == 0) {
        pthread_mutex_lock(&lock_concurrent);
        idx = n.first;
        lwsl_user("CLIENT_CALLBACK_CLIENT_CLOSED: %s\n",
                  in ? (char *) in : "");
        pthread_mutex_unlock(&lock_concurrent);
        goto do_retry;
      }
    }
    break;

  default :
    // Make compiler happy regarding unhandled enums.
    break;
  }

  return lws_callback_http_dummy(wsi, reason, user, in, len);

  do_retry:
  try {
    if (lws_retry_sul_schedule_retry_wsi(endpoints_prop[idx.load()].wsi, &endpoints_prop[idx.load()]._sul, connect_client,
                                         &endpoints_prop[idx.load()].retry_count)) {
      if (endpoints_prop[idx.load()].retry_count > 2 * (LWS_ARRAY_SIZE(backoff_ms))) {
        pthread_mutex_lock(&lock_concurrent);
        lwsl_err("%s: connection attempts exhausted, retry_count[%d], ws_path:%s\n",
                 __func__, endpoints_prop[idx.load()].retry_count, endpoints_prop[idx.load()].ws_path);
        endpoints_prop[idx.load()].wsi = NULL;
        endpoints_prop[idx.load()].json_cb = NULL;
        endpoints_prop[idx.load()].retry_count = 0;
        endpoints_prop[idx.load()].conn = NULL;
        lws_cancel_service(lws_get_context(wsi));
        lws_context_destroy(lws_get_context(wsi));
        atomic_store(&lws_service_cancelled, 1);
        pthread_mutex_unlock(&lock_concurrent);
        return -1;
      }
      {
        pthread_mutex_lock(&lock_concurrent);
        endpoints_prop[idx.load()].wsi = NULL;
        auto n = force_create_ccinfo(idx.load());
        if (n) {
          endpoints_prop[idx.load()].wsi = NULL;
          endpoints_prop[idx.load()].json_cb = NULL;
          endpoints_prop[idx.load()].retry_count = 0;
          endpoints_prop[idx.load()].conn = NULL;
          lws_cancel_service(lws_get_context(wsi));
          lws_context_destroy(lws_get_context(wsi));
          atomic_store(&lws_service_cancelled, 1);
          pthread_mutex_unlock(&lock_concurrent);
          return -1;
        }
        pthread_mutex_unlock(&lock_concurrent);
      }
    }
    lwsl_user("%s: connection attempts success, after [%d] retry, ws_path:%s\n",
              __func__, endpoints_prop[idx.load()].retry_count, endpoints_prop[idx.load()].ws_path);
  } catch (exception &e) {
    Logger::write_log("<binance::Websocket::event_cb> Error do_retry message : %s\n", e.what());
    lwsl_err("%s: Error in do_retry: %s\n", __func__, e.what());
    atomic_store(&lws_service_cancelled, 1);
    return -1;
  }

  return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const lws_protocols protocols[] =
    {
        {
            .name = "binance-websocket-api",
            .callback = event_cb,
            .per_session_data_size = sizeof(struct endpoint_connection),
            .rx_buffer_size = 128 * 1024,
        },

        {NULL, NULL, 0, 0} /* end */
    };

ALWAYS_INLINE static void sigint_handler(int sig) {
  Logger::write_log("<binance::Websocket::sigint_handler> Interactive attention signal : %d\n", sig);
  atomic_store(&lws_service_cancelled, 1);
}

/*
 * Scheduled sul callback that starts the connection attempt
 */
ALWAYS_INLINE static void connect_client(lws_sorted_usec_list_t *sul) {
  for (std::pair<int, endpoint_connection> n : endpoints_prop) {
    if (&endpoints_prop[n.first]._sul == sul) {
      pthread_mutex_lock(&lock_concurrent);
      std::atomic<int> idx(n.first);
      lwsl_user("%s: success ws_path::%s\n",
                __func__, endpoints_prop[idx.load()].ws_path);
      struct lws_client_connect_info ccinfo{};

      memset(&ccinfo, 0, sizeof(ccinfo));

      ccinfo.context = context;
      ccinfo.port = BINANCE_WS_PORT;
      ccinfo.address = BINANCE_WS_HOST;
      ccinfo.path = endpoints_prop[idx.load()].ws_path;
      ccinfo.host = lws_canonical_hostname(context);
      ccinfo.origin = "origin";
      ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED |
          LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK |
          LCCSCF_PIPELINE | LCCSCF_PRIORITIZE_READS |
          LCCSCF_WAKE_SUSPEND__VALIDITY;
      ccinfo.protocol = protocols[0].name;
      ccinfo.local_protocol_name = protocols[0].name;
      ccinfo.retry_and_idle_policy = &retry;
      ccinfo.userdata = &endpoints_prop[idx.load()];
      /*
       * We store the new wsi here early in the connection process,
       * this gives the callback a way to identify which wsi faced the error
       * even before the new wsi is returned and even if ultimately no wsi is returned.
       */
      ccinfo.pwsi = &endpoints_prop[idx.load()].wsi;

      endpoints_prop[idx.load()].conn = lws_client_connect_via_info(&ccinfo);
      pthread_mutex_unlock(&lock_concurrent);
      if (!endpoints_prop[idx.load()].conn) {
        /*
         * Failed... schedule a retry... we can't use the _retry_wsi()
         * convenience wrapper api here because no valid wsi at this
         * point.
         */
        if (lws_retry_sul_schedule(context, 0, &endpoints_prop[idx.load()]._sul, &retry,
                                   connect_client, &endpoints_prop[idx.load()].retry_count)) {
          pthread_mutex_lock(&lock_concurrent);
          lwsl_err("%s: Failed schedule a retry, we can't use the _retry_wsi():%s\n",
                   __func__, endpoints_prop[idx.load()].ws_path);
          atomic_store(&lws_service_cancelled, 1);
          pthread_mutex_unlock(&lock_concurrent);
          return;
        }
      }
      break;
    }
  }
}

ALWAYS_INLINE static int force_create_ccinfo(const int &idx) {
  try {
    struct lws_client_connect_info ccinfo{};
    memset(&ccinfo, 0, sizeof(ccinfo));
    ccinfo.context = context;
    ccinfo.port = BINANCE_WS_PORT;
    ccinfo.address = BINANCE_WS_HOST;
    ccinfo.path = endpoints_prop[idx].ws_path;
    ccinfo.host = lws_canonical_hostname(context);
    ccinfo.origin = "origin";
    ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED |
        LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK |
        LCCSCF_PIPELINE | LCCSCF_PRIORITIZE_READS |
        LCCSCF_WAKE_SUSPEND__VALIDITY;
    ccinfo.protocol = protocols[0].name;
    ccinfo.local_protocol_name = protocols[0].name;
    ccinfo.retry_and_idle_policy = &retry;
    ccinfo.userdata = &endpoints_prop[idx];
    ccinfo.pwsi = &endpoints_prop[idx].wsi;
    endpoints_prop[idx].conn = lws_client_connect_via_info(&ccinfo);
    if (!endpoints_prop[idx].conn) {
      lwsl_err("%s: Failed :%s\n",
               __func__, endpoints_prop[idx].ws_path);
      atomic_store(&lws_service_cancelled, 1);
      return 1;
    }
    lwsl_user("%s: success, retry_count[%d] endpoint#:%d ws_path::%s\n",
              __func__, idx, endpoints_prop[idx].retry_count, endpoints_prop[idx].ws_path);
  } catch (exception &e) {
    lwsl_err("%s:::%s\n",
             __func__, e.what());
    lwsl_err("%s: Failed :%s\n",
             __func__, endpoints_prop[idx].ws_path);
    atomic_store(&lws_service_cancelled, 1);
    return 1;
  }
  return 0;
}

void binance::Websocket::init() {
  pthread_mutex_init(&lock_concurrent, NULL);
  endpoints_prop.clear();

  struct lws_context_creation_info info{};
  signal(SIGINT, sigint_handler);
  memset(&info, 0, sizeof(info));
  // This option is needed here to imply LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT
  // option, which must be set on newer versions of OpenSSL.
  info.options = LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;
  info.port = CONTEXT_PORT_NO_LISTEN;
  info.gid = -1;
  info.uid = -1;
  info.protocols = protocols;
  info.fd_limit_per_thread = 1024;
  info.max_http_header_pool = 1024;

  context = lws_create_context(&info);
  if (!context) {
    lwsl_err("lws init failed\n");
    atomic_store(&lws_service_cancelled, 1);
    return;
  } else {
    atomic_store(&lws_service_cancelled, 0);
  }
}

// Register call backs
void binance::Websocket::connect_endpoint(CB cb, const char *path) {
  try {
    pthread_mutex_lock(&lock_concurrent);
    if (endpoints_prop.size() > 1024) {
      lwsl_err("%s: maximum of 1024 connect_endpoints reached,\n",
               __func__);
      pthread_mutex_unlock(&lock_concurrent);
      return;
    }
    std::atomic<int> n(static_cast<int>(endpoints_prop.size()));
    endpoints_prop[n].ws_path = const_cast<char *>(path);
    endpoints_prop[n].json_cb = cb;
    pthread_mutex_unlock(&lock_concurrent);
    force_create_ccinfo(n);
  } catch (exception &e) {
    lwsl_err("%s:::%s\n",
             __func__, e.what());
    lwsl_err("%s: connect_endpoints error path::%s\n",
             __func__, path);
    return;
  }

  //connect_client(&endpoints_prop[n]._sul);

  /*if (!lws_service_cancelled)
  {
      //schedule the first client connection attempt to happen immediately
      lws_sul_schedule(context, 0, &endpoints_prop[n]._sul, connect_client, 1 * LWS_US_PER_MS);
      lwsl_user("%s: schedule the first client connection for endpoint#:%d ws_path::%s\n",
          __func__, n, endpoints_prop[n].ws_path);
  }*/
}

ALWAYS_INLINE static void force_delete_ccinfo(const char *path) {

  try {
    std::atomic<bool> path_found(false);
    for (std::pair<int, endpoint_connection> n : endpoints_prop) {
      lwsl_info("%s: loop find path::%s vs path::%s\n",
                __func__, path, endpoints_prop[n.first].ws_path);

      if (endpoints_prop[n.first].ws_path != NULL && strcmp(const_cast<char *>(path), endpoints_prop[n.first].ws_path) == 0) {
        std::atomic<int> idx(n.first);
        lwsl_info("%s: found connect_endpoints ws_path::%s\n",
                  __func__, endpoints_prop[idx].ws_path);
        lws_cancel_service_pt(endpoints_prop[idx].wsi);
        endpoints_prop[idx.load()].ws_path = NULL;
        endpoints_prop[idx.load()].wsi = NULL;
        endpoints_prop[idx.load()].json_cb = NULL;
        endpoints_prop[idx.load()].retry_count = 0;
        endpoints_prop[idx.load()].conn = NULL;
        path_found.store(true);
        break;
      }
    }
    if(!path_found.load()){
      lwsl_err("%s: not found connect_endpoints error path::%s\n",
               __func__, path);
    }

    return;
  } catch (exception &e) {
    lwsl_err("%s:::%s\n",
             __func__, e.what());
    lwsl_err("%s: not found connect_endpoints error path::%s\n",
             __func__, path);
    return;
  }
}

ALWAYS_INLINE static void force_reconnect(const char *path) {

  try {
    std::atomic<bool> path_found(false);
    for (std::pair<int, endpoint_connection> n : endpoints_prop) {
      assert(endpoints_prop[n.first].ws_path);
      lwsl_info("%s: loop find path::%s vs path::%s\n",
                __func__, path, endpoints_prop[n.first].ws_path);

      if (endpoints_prop[n.first].ws_path != NULL && strcmp(const_cast<char *>(path), endpoints_prop[n.first].ws_path) == 0) {
        std::atomic<int> idx(n.first);
        lwsl_info("%s: found ws_path::%s\n",
                  __func__, endpoints_prop[idx.load()].ws_path);
        if (!lws_service_cancelled)
        {
          //schedule the first client connection attempt to happen immediately
          lws_sul_schedule(context, 0, &endpoints_prop[idx.load()]._sul, connect_client, 1 * LWS_US_PER_MS);
          lwsl_user("%s: schedule reconnection for endpoint#:%d ws_path::%s\n",
                    __func__, idx.load(), endpoints_prop[idx.load()].ws_path);
        }
        path_found.store(true);
        break;
      }
    }
    if(!path_found.load()){
      lwsl_err("%s: not found force_reconnect error path::%s\n",
               __func__, path);
    }

    return;
  } catch (exception &e) {
    lwsl_err("%s:::%s\n",
             __func__, e.what());
    lwsl_err("%s: not found force_reconnect error path::%s\n",
             __func__, path);
    return;
  }
}

void binance::Websocket::reconnect_path(const char *path) {
  pthread_mutex_lock(&lock_concurrent);
  force_reconnect(path);
  pthread_mutex_unlock(&lock_concurrent);
}

void binance::Websocket::kill_all() {
  pthread_mutex_lock(&lock_concurrent);
  atomic_store(&lws_service_cancelled, 1);
  pthread_mutex_unlock(&lock_concurrent);
}

// Unregister call backs
void binance::Websocket::disconnect_endpoint(const char *path) {
  try {
    pthread_mutex_lock(&lock_concurrent);
    if (endpoints_prop.empty()) {
      lwsl_err("%s: error connect_endpoints is empty,\n",
               __func__);
      pthread_mutex_unlock(&lock_concurrent);
      return;
    }
    force_delete_ccinfo(path);
    pthread_mutex_unlock(&lock_concurrent);
  } catch (exception &e) {
    lwsl_err("%s:::%s\n",
             __func__, e.what());
    lwsl_err("%s: disconnect_endpoint error path::%s\n",
             __func__, path);
    return;
  }

}

// Entering event loop
void binance::Websocket::enter_event_loop(const std::chrono::hours &hours) {
  auto start = std::chrono::steady_clock::now();
  auto end = start + hours;
  auto n = 0;
  lwsl_user("%s: INIT\n", __func__);
  do {
    try {
      n = lws_service(context, 0);
      if (lws_service_cancelled) {
        lws_cancel_service(context);
        break;
      }
    } catch (exception &e) {
      lwsl_err("%s:::%s\n",
               __func__, e.what());
      Logger::write_log("<BinaCPP_websocket::enter_event_loop> Error ! %s", e.what());
      lws_cancel_service(context);
      break;
    }
  } while (n >= 0 && std::chrono::steady_clock::now() < end);

  endpoints_prop.clear();
  atomic_store(&lws_service_cancelled, 1);
  lws_context_destroy(context);
  pthread_mutex_destroy(&lock_concurrent);
}
