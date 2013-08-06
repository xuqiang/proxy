#ifndef _TEXT_H_
#define _TEXT_H_

#include <vector>
#include <string>


class StringSplitter {
public:
	/** @param big the string to be split
		@param splitter the delimiter
	*/
	StringSplitter( const char * big , const char * splitter )
		: _big( big ) , _splitter( splitter ) {
	}

	/** @return true if more to be taken via next() */
	bool more() const { return _big[0] != 0; }

	/** get next split string fragment */
	std::string next();

	void split( std::vector<std::string>& l );

	std::vector<std::string> split();
	
	static std::vector<std::string> split( const std::string& big , const std::string& splitter );
	
	static std::string join( std::vector<std::string>& l , const std::string& split );

private:
	const char * _big;
	const char * _splitter;
};

#endif // #ifndef


