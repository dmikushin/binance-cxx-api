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
static int force_create_ccinfo(const std::string &path);
/*
 * This "contains" the endpoint connection property and has
 * the connection bound to it
 */
struct endpoint_connection {
  struct lws *wsi;         /* related wsi if any */
  uint16_t retry_count; /* count of consecutive retries */
  lws *conn;
  CB json_cb;
  std::string ws_path;
  atomic<bool> close_conn;
};

static std::unordered_map<std::string, endpoint_connection> endpoints_prop;
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
    .secs_since_valid_ping        = 60*3, /* force PINGs after secs idle */
    .secs_since_valid_hangup    = 60*10, /* hangup after secs idle */
    .jitter_percent            = 30,
    /*
     * jitter_percent controls how much additional random delay is
     * added to the actual interval to be used, default 30
     */
};

static int event_cb(lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

struct lws_protocols protocols[] =
    {
        {
            .name = "binance-websocket-api",
            .callback = event_cb,
            .per_session_data_size = sizeof(struct endpoint_connection),
            .rx_buffer_size = 128 * 1024,
        },

        {nullptr, nullptr, 0, 0} /* end */
    };

static int event_cb(lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {

  auto *current_data = static_cast< endpoint_connection *>(user);

  switch (reason) {

  case LWS_CALLBACK_WSI_CREATE:
    if(!lws_service_cancelled){
      const std::string ws_path = current_data->ws_path;
      if (!ws_path.empty() && endpoints_prop.find(ws_path) != endpoints_prop.end()) {
        if(!endpoints_prop.at(ws_path).close_conn.load()){
          pthread_mutex_lock(&lock_concurrent);
          endpoints_prop.at(ws_path).wsi = wsi;
          lwsl_user("%s: create wsi for current_data#:%s ws_path::%s\n",
                    __func__, ws_path.c_str(), endpoints_prop.at(ws_path).ws_path.c_str());
          pthread_mutex_unlock(&lock_concurrent);
        }
      }
    }
    break;

  case LWS_CALLBACK_CLIENT_ESTABLISHED:
    if(!lws_service_cancelled){
      const std::string ws_path = current_data->ws_path;
      if (!ws_path.empty() && endpoints_prop.find(ws_path) != endpoints_prop.end()) {
        if(!endpoints_prop.at(ws_path).close_conn.load()){
          pthread_mutex_lock(&lock_concurrent);
          lws_callback_on_writable(wsi);
          endpoints_prop.at(ws_path).wsi = wsi;
          lwsl_user("%s: connection established with success current_data#:%s ws_path::%s\n",
                    __func__, ws_path.c_str(), endpoints_prop.at(ws_path).ws_path.c_str());
          pthread_mutex_unlock(&lock_concurrent);
        }
      }

    }
    break;

  case LWS_CALLBACK_CLIENT_RECEIVE:
    if(!lws_service_cancelled){
      const std::string ws_path = current_data->ws_path;
      if (!ws_path.empty() && endpoints_prop.find(ws_path) != endpoints_prop.end()) {
        if(!endpoints_prop.at(ws_path).close_conn.load()){
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
          endpoints_prop.at(ws_path).json_cb(json_result);
          endpoints_prop.at(ws_path).retry_count = 0;
          json_result.clear();
          pthread_mutex_unlock(&lock_concurrent);
        }
        break;
      }
      break;
    }
    break;

  case LWS_CALLBACK_CLOSED :
  case LWS_CALLBACK_CLIENT_WRITEABLE:
    break;


  case LWS_CALLBACK_CLIENT_CLOSED:
  case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
  case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
  case LWS_CALLBACK_WSI_DESTROY:
    if (in != nullptr){
      lwsl_err("case reason:%d  ERROR: %s\n", reason,
               in ? (char *) in : "(null)");
    }
    lws_set_opaque_user_data(wsi, nullptr);
    if(!lws_service_cancelled){
      const std::string ws_path = current_data->ws_path;
      if (!ws_path.empty() && endpoints_prop.find(ws_path) != endpoints_prop.end()) {
        if (endpoints_prop.at(ws_path).close_conn.load()) {
          pthread_mutex_lock(&lock_concurrent);
          endpoints_prop.at(ws_path).wsi = nullptr;
          endpoints_prop.at(ws_path).ws_path.clear();
          endpoints_prop.erase(ws_path);
          lwsl_err("reason:%d  deleted: %s : %s\n", reason,
                   in ? (char *) in : "(null)", ws_path.c_str());
          pthread_mutex_unlock(&lock_concurrent);
        } else {
          endpoints_prop.at(ws_path).retry_count++;
          endpoints_prop.at(ws_path).wsi = nullptr;
          auto n = force_create_ccinfo(ws_path);
          if (n || endpoints_prop.at(ws_path).retry_count > (LWS_ARRAY_SIZE(backoff_ms))) {
            pthread_mutex_lock(&lock_concurrent);
            endpoints_prop.at(ws_path).wsi = nullptr;
            endpoints_prop.at(ws_path).ws_path.clear();
            endpoints_prop.erase(ws_path);
            lws_cancel_service(lws_get_context(wsi));
            lws_context_destroy(lws_get_context(wsi));
            atomic_store(&lws_service_cancelled, 1);
            pthread_mutex_unlock(&lock_concurrent);
            return -1;
          }
        }
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
  default :
    // Make compiler happy regarding unhandled enums.
    break;
  }

  if(lws_service_cancelled){
    return -1;
  }
  return lws_callback_http_dummy(wsi, reason, user, in, len);
}


static void sigint_handler(int sig) {
  Logger::write_log("<binance::Websocket::sigint_handler> Interactive attention signal : %d\n", sig);
  atomic_store(&lws_service_cancelled, 1);
}

/* do not use pthread_mutex_lock on force_create_ccinfo*/
static int force_create_ccinfo(const std::string &path) {
  struct lws_client_connect_info ccinfo{};
  memset(&ccinfo, 0, sizeof(ccinfo));
  ccinfo.context = context;
  ccinfo.port = BINANCE_WS_PORT;
  ccinfo.address = BINANCE_WS_HOST;
  ccinfo.path = endpoints_prop.at(path).ws_path.c_str();
  ccinfo.host = lws_canonical_hostname(context);
  ccinfo.origin = "origin";
  ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED |
      LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK |
      LCCSCF_PIPELINE | LCCSCF_PRIORITIZE_READS |
      LCCSCF_WAKE_SUSPEND__VALIDITY;
  ccinfo.protocol = protocols[0].name;
  ccinfo.local_protocol_name = protocols[0].name;
  ccinfo.retry_and_idle_policy = &retry;
  ccinfo.userdata = &endpoints_prop.at(path);
  ccinfo.pwsi = &endpoints_prop.at(path).wsi;
  endpoints_prop.at(path).conn = lws_client_connect_via_info(&ccinfo);
  if (!endpoints_prop.at(path).conn) {
    lwsl_err("%s: Failed :%s\n",
             __func__, endpoints_prop.at(path).ws_path.c_str());
    /*try cancel the service*/
    atomic_store(&lws_service_cancelled, 1);
    return 1;
  }
  lwsl_user("%s: success, original_path#:%s retry_count[%d] ws_path::%s\n",
            __func__, path.c_str(), endpoints_prop.at(path).retry_count, endpoints_prop.at(path).ws_path.c_str());
  return 0;
}

/* do not use pthread_mutex_lock on force_delete_ccinfo*/
static void force_delete_ccinfo(const std::string &path) {

  if ( endpoints_prop.find(path) != endpoints_prop.end() ) {
    lwsl_info("%s: found connect_endpoints ws_path::%s\n",
              __func__, endpoints_prop.at(path).ws_path.c_str());
    if(endpoints_prop.at(path).wsi) {
      pthread_mutex_lock(&lock_concurrent);
      atomic_store(&endpoints_prop.at(path).close_conn, true);
      pthread_mutex_unlock(&lock_concurrent);
      /*use Async Kill*/
      lws_set_timeout(endpoints_prop.at(path).wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
    }
  } else {
    lwsl_err("%s: not found connect_endpoints error path::%s\n",
             __func__, path.c_str());
  }
  return;
}

void binance::Websocket::kill_all() {
  pthread_mutex_lock(&lock_concurrent);
  atomic_store(&lws_service_cancelled, 1);
  pthread_mutex_unlock(&lock_concurrent);
}

void binance::Websocket::init() {
  pthread_mutex_init(&lock_concurrent, nullptr);
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
    pthread_mutex_destroy(&lock_concurrent);
    return;
  } else {
    atomic_store(&lws_service_cancelled, 0);
  }
}

// Register call backs
void binance::Websocket::connect_endpoint(CB cb,const std::string &path) {

  if (endpoints_prop.size() > 1024) {
    lwsl_err("%s: maximum of 1024 connect_endpoints reached,\n",
             __func__);
    return;
  }
  if(!lws_service_cancelled){
    pthread_mutex_lock(&lock_concurrent);
    endpoints_prop[path].ws_path = path;
    endpoints_prop[path].json_cb = cb;
    atomic_store(&endpoints_prop[path].close_conn, false);
    pthread_mutex_unlock(&lock_concurrent);
    int n = force_create_ccinfo(path);
    lwsl_user("%s: connecting::%s connect result[%s],\n",
              __func__, path.c_str(), n ? "NotOkay" : "Okay");
  } else {
    lwsl_err("%s: no service running,\n",
             __func__);
    return;
  }
}

// Unregister call backs
void binance::Websocket::disconnect_endpoint(const std::string &path) {

  if (endpoints_prop.empty()) {
    lwsl_err("%s: error connect_endpoints is empty,\n",
             __func__);
    return;
  }
  if(!lws_service_cancelled) {
    force_delete_ccinfo(path);
  } else {
    lwsl_err("%s: no service running,\n",
             __func__);
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
        /* extra check if not null */
        if(context)lws_cancel_service(context);
        break;
      }
    } catch (exception &e) {
      lwsl_err("%s:::%s\n",
               __func__, e.what());
      Logger::write_log("<BinaCPP_websocket::enter_event_loop> Error ! %s", e.what());
      /* extra check if not null */
      if(context)lws_cancel_service(context);
      break;
    }
  } while (n >= 0 && std::chrono::steady_clock::now() < end);

  /* ensure the service is canceled after the hours timeout */
  pthread_mutex_lock(&lock_concurrent);
  atomic_store(&lws_service_cancelled, 1);
  pthread_mutex_unlock(&lock_concurrent);

  if(context)lws_context_destroy(context);
  /* extra check if not null */
  pthread_mutex_destroy(&lock_concurrent);
  endpoints_prop.clear();
}
