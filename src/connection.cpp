#include <iostream>
#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"
#include "backend_selector.h"
#include <boost/lexical_cast.hpp>
#include "glog/logging.h"
#include "timer.h"

namespace http {
namespace server2 {

connection::connection(boost::asio::io_service& io_service,
		backend_selector* s)
  : socket_(io_service),
	backend_selector_(s),
	resolver_(io_service),
	backend_socket_(io_service)
{
}

// client到proxy的socket
boost::asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

boost::asio::ip::tcp::socket& connection::backend_socket() {
	return backend_socket_;
}

void connection::start()
{
	timer_.start();
	// client读取数据
  socket_.async_read_some(boost::asio::buffer(buffer_),
      boost::bind(&connection::handle_read, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

// 从client读取数据
void connection::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
	LOG(INFO) << "handle_read";
  if (!e)
  {
    boost::tribool result;
    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred);
	// 如果读取完成
    if (result)
    {
		// 选择backend
		backend b;
		int ret = backend_selector_->pick(request_, b);
		LOG(INFO) << "requst_=" << request_.uri << 
			" backend=" << b.address << ":" << b.port << 
			" takes=" << timer_.ms();

		if(ret) {
			reply_ = reply::stock_reply(reply::bad_request);
			boost::asio::async_write(socket_, 
					reply_.to_buffers(),
					boost::bind(&connection::handle_write, shared_from_this(),
						boost::asio::placeholders::error));
		}
	
		// 发起请求
		boost::asio::ip::tcp::resolver::query query(b.address, 
												boost::lexical_cast<std::string> (b.port));
		resolver_.async_resolve(query, 
				boost::bind(&connection::handle_resolve, 
							shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::iterator));	
    }
    else if (!result)
    {
		// 解析错误
      reply_ = reply::stock_reply(reply::bad_request);
      boost::asio::async_write(socket_, reply_.to_buffers(),
          boost::bind(&connection::handle_write, shared_from_this(),
            boost::asio::placeholders::error));
    }
    else
    {
		// 读取未完成
      socket_.async_read_some(boost::asio::buffer(buffer_),
          boost::bind(&connection::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket.
}

// 尝试连接backend serv
void connection::handle_connect(const boost::system::error_code& err,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		const bool first_time) {
	LOG(INFO) << "handle_connect";
	if (!err && !first_time) {
		LOG(INFO) << "handle reslove takes " << timer_.ms() << "ms";
		start_write_to_backend();
	}
	else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator()) {
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		backend_socket_.async_connect(endpoint, 
				boost::bind(&connection::handle_connect,
						shared_from_this(),
						boost::asio::placeholders::error,
						++endpoint_iterator, false));
	}
	else {
		// 错误情况
		reply_ = reply::stock_reply(reply::internal_server_error);
		boost::asio::async_write(socket_, reply_.to_buffers(),
				boost::bind(&connection::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
	}
}


// 如果转换成endpoint成功
void connection::handle_resolve(const boost::system::error_code& err,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
	LOG(INFO) << "handle_resolve";
	if(!err) {
		// 得到第一个endpoint，第一次尝试连接改backend
		const bool first_time = true;
		handle_connect(boost::system::error_code(), endpoint_iterator, first_time);
	}
	else {
		// 解析错误，写完之后，shutdown
		reply_ = reply::stock_reply(reply::internal_server_error);
		boost::asio::async_write(socket_, reply_.to_buffers(),
				boost::bind(&connection::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
	}
}

// 向backend serv写数据
void connection::start_write_to_backend() {
	LOG(INFO) << "start_write_to_backend";
	// 向backend写数据
	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(buffer_.data(),
							buffer_.size()));
	LOG(INFO) << "write backend date="  << buffer_.data();
	boost::asio::async_write(backend_socket_, 
							buffers,
							boost::bind(&connection::handle_server_write_succ, 
										shared_from_this(),
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred));
}


// 向backend写入成功，读取数据
void connection::handle_server_write_succ(const boost::system::error_code& err,
										size_t len) {
	LOG(INFO) << "write data to backend takes " << timer_.ms() << "ms";
	const std::string http_end = "\r\n\r\n";
	if(!err) {
		std::string received(backend_buffer_.data());
		if(received.find(http_end) != std::string::npos) {
			LOG(INFO) << "read from backend=" << backend_buffer_.data();
			// 数据写入client
			std::vector<boost::asio::const_buffer> buffers;
			buffers.push_back(boost::asio::buffer(backend_buffer_.data(),
													backend_buffer_.size()));
			boost::asio::async_write(socket_, 
									buffers,
									boost::bind(&connection::handle_write,
												shared_from_this(),
												boost::asio::placeholders::error));		
		}
		else
		{
			// 读取未完成
			backend_socket_.async_read_some(boost::asio::buffer(backend_buffer_),
					boost::bind(&connection::handle_server_write_succ, 
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
		}
	}
	else {
		reply_ = reply::stock_reply(reply::bad_request);
		boost::asio::async_write(backend_socket_, reply_.to_buffers(),
				boost::bind(&connection::handle_write, shared_from_this(),
					boost::asio::placeholders::error));
	}
}

// 写完成之后，调用该函数清理io_service
void connection::handle_write(const boost::system::error_code& e)
{
  if (!e)
  {
	LOG(INFO) << "write to client total takes " << timer_.ms() << "ms";
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    backend_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	
	timer_.reset();

	// 清楚缓存中的数据 
	buffer_.assign(0);
	backend_buffer_.assign(0);
  }

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
  // 类将会注销
}

} // namespace server2
} // namespace http
