#ifndef HTTP_SERVER2_CONNECTION_HPP
#define HTTP_SERVER2_CONNECTION_HPP

#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "backend_selector.h"
#include "boost/system/error_code.hpp"
#include "boost/chrono.hpp"
#include "timer.h"


namespace http {
namespace server2 {

/// Represents a single connection from a client.
class connection
  : public boost::enable_shared_from_this<connection>,
    private boost::noncopyable
{
public:
  /// Construct a connection with the given io_service.
  explicit connection(boost::asio::io_service& io_service,
		  backend_selector* s);

  /// Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& socket();

  
  // backend_socket
  boost::asio::ip::tcp::socket& backend_socket();

  /// Start the first asynchronous operation for the connection.
  void start();
private:

  void handle_server_write_succ(const boost::system::error_code& err,
		  size_t len);


  /// Handle completion of a read operation.
  void handle_read(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /// Handle completion of a write operation.
  void handle_write(const boost::system::error_code& e);

 void handle_resolve(const boost::system::error_code& err, 
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

 void handle_connect(const boost::system::error_code& err,
		 boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		 const bool first_time);
	
 void start_write_to_backend();

 /// client -> proxy所用的socket
  boost::asio::ip::tcp::socket socket_;
  // backend -> proxy backend到proxy所用的socket
	boost::asio::ip::tcp::socket backend_socket_;

  /// The handler used to process the incoming request. 直接从backend中读取
  // request_handler& request_handler_;

  /// Buffer for incoming data. client -> proxy的buffer
  boost::array<char, 8192> buffer_;
  // backend -> proxy所使用的buffer
  boost::array<char, 8192> backend_buffer_;

  /// The incoming request.
  request request_;

  /// The parser for the incoming request.
  request_parser request_parser_;

  /// The reply to be sent back to the client.
  reply reply_;

  // backend选择器
  backend_selector* backend_selector_;
	// tcp resolver将ip转换成endpoint对象
  boost::asio::ip::tcp::resolver resolver_;

	// 计时器
	timer timer_;	
};

typedef boost::shared_ptr<connection> connection_ptr;

} // namespace server2
} // namespace http

#endif // HTTP_SERVER2_CONNECTION_HPP
