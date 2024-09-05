#include "rank.h"
#include <thread>
#include "corotask/common/logging.h"
#include "corotask/common/traits.h"

using namespace ct::model;

rank& rank::instance() {
   static rank r;
   return r;
}

bool rank::init(const int argc, char** argv) {
   if (argc == 1) {
      return init_root(argc, argv);
   }
   return init_worker(argc, argv);
}

bool rank::fini() {
   _coro_pool.shutdown();
   return true;
}

void rank::await(void* coro) {
   const auto& prom = tasks::promise_base_generic::from_address(coro);
   while (!prom.is_done.test());
}

bool rank::init_root(int argc, char** argv) {
   _coro_pool.add_workers(std::thread::hardware_concurrency());
   return true;
}

bool rank::init_worker(int argc, char** argv) {
   _id = QString::fromStdString(argv[1]).toInt();
   _workers = QString::fromStdString(argv[2]).toInt();
   const auto root_server = QString::fromStdString(argv[3]);

   _threads = std::thread::hardware_concurrency();

   // append and try to connect
   const auto& root_client = _clients.emplace_back(new network::client);
   if (!root_client->connect(root_server)) {
      CT_FATAL("Failed to connect to root-server");
      return false;
   }

   // setup server for other clients to connect to
   _server = uptr<network::server>(new network::server);
   if (!_server->start(QString("0.0.0.0:%1").arg(PORT))) {
      CT_FATAL("Failed to start server");
      return false;
   }

   // connect to all other workers
   const auto urls = root_client->get_workers();
   for (int i = 0; i < urls.size(); i++) {
      const auto& client = _clients.emplace_back(new network::client);
      if (!client->connect(urls.at(i))) {
         CT_FATAL("Failed to connect to worker");
         return false;
      }
   }

   // move server to its own thread

   // move all clients to its own threads

   return true;
}
