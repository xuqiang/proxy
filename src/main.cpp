#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "server.hpp"
#include "config.h"

int main(int argc, char* argv[])
{
  try
  {
	  // 读取配置文件
	http::server2::config cfg("./conf/proxy.conf");
	int ret = cfg.parse();
	if(ret) {
		return 1;
	}
	

	// Initialise the server.
    http::server2::server s(cfg);

    // Run the server until stopped.
    s.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
