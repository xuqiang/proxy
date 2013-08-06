#include "config.h"
#include "text.h"

#include "boost/program_options.hpp"
#include "boost/program_options/parsers.hpp"
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <vector>

namespace http {
namespace server2 {
config::config(const std::string& fn) {
	conf_fname_ = fn;
}

int config::parse() {
	namespace po = boost::program_options;
	po::options_description desc("proxy config");
	po::variables_map vm;
	desc.add_options()
		("backends", po::value<std::string>(), "backends list")
		("thread_num", po::value<int>(), "thread num")
		("address", po::value<std::string>(), "host to run")
		("hash_str", po::value<std::string>(), "host to run")
		("port", po::value<std::string>(), "port num");
	
	std::ifstream ifs(conf_fname_);
	if(!ifs) {
		return 1;
	}
	store(po::parse_config_file(ifs, desc), 
			vm);
	notify(vm);
	
	if(vm.count("backends") && vm.count("thread_num") &&
			vm.count("address") && vm.count("port") &&
			vm.count("hash_str")) {
		thread_num_ = vm["thread_num"].as<int>();
		address_ = vm["address"].as<std::string> ();
		port_ = vm["port"].as<std::string> ();
		hash_str_ = vm["hash_str"].as<std::string> ();

		std::string backends = vm["backends"].as<std::string> ();
		std::vector<std::string> host_and_ports = StringSplitter::split(backends, ",");	
		if(host_and_ports.size() <= 0) {
			return 1;
		}
		for(int i = 0; i < host_and_ports.size(); ++i) {
			std::vector<std::string> parts = StringSplitter::split(host_and_ports[i], ":");
			if(parts.size() != 2) {
				continue;
			}
			backend b;
			try {
				b.port = boost::lexical_cast<int>(parts[1]); 
			}
			catch (std::exception& e) {
				continue;
			}
			b.address = parts[0];
			b.query_str = host_and_ports[i];

			backends_.push_back(b);
		}
		if(backends_.size() <= 0) {
			return 1;
		}
		return 0;
	}
	else {
		return 1;
	}
	return 1;
}

};
};
