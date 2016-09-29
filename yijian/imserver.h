#ifndef IMSERVER_H_
#define IMSERVER_H_

#include "macro.h"
#include "pinglist.h"
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <ctime>
#include <ev.h>

struct Imp_connection {
  ev_io io_;
  time_t last_msg_time_;
};

class Connection {

};

class Server {
};


#endif
