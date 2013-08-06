#include "server.hpp"
#include <boost/bind.hpp>
#include "backend_selector.h"
#include "glog/logging.h"

namespace http {
namespace server2 {


server::server(const http::server2::config& c)
  : io_service_pool_(c.thread_num_),
    signals_(io_service_pool_.get_io_service()),
    acceptor_(io_service_pool_.get_io_service()),
    new_connection_()
{
	// 初始化glog
	FLAGS_log_dir = "./logs/";
	FLAGS_logbufsecs = 1; 
	google::InitGoogleLogging("proxy");

	backend_selector_.init(c);
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  //  服务器停止signal,调用handle_stop 
	signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
  signals_.async_wait(boost::bind(&server::handle_stop, this));

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  // 绑定端口，开始listen
  boost::asio::ip::tcp::resolver resolver(acceptor_.get_io_service());
  boost::asio::ip::tcp::resolver::query query(c.address_, c.port_);
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  start_accept();
}


void server::run()
{
  io_service_pool_.run();
}

void server::start_accept()
{
  new_connection_.reset(new connection(
        io_service_pool_.get_io_service(), 
		&backend_selector_));
  acceptor_.async_accept(new_connection_->socket(),
      boost::bind(&server::handle_accept, this,
        boost::asio::placeholders::error));
}

// 请求处理函数
void server::handle_accept(const boost::system::error_code& e)
{
  if (!e)
  {
    new_connection_->start();
  }

  start_accept();
}

// 停止server
void server::handle_stop()
{
  io_service_pool_.stop();
}

} // namespace server2
} // namespace http
