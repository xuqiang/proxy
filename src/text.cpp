// text.cpp
#include "text.h"
#include <string.h>
#include <sstream>

#include <boost/smart_ptr/scoped_array.hpp>

using namespace std;


// --- StringSplitter ----

/** get next split string fragment */
string StringSplitter::next() {
	const char * foo = strstr( _big , _splitter );
	if ( foo ) {
		string s( _big , foo - _big );
		_big = foo + strlen( _splitter );
		while ( *_big && strstr( _big , _splitter ) == _big )
			_big++;
		return s;
	}
	
	string s = _big;
	_big += strlen( _big );
	return s;
}


void StringSplitter::split( vector<string>& l ) {
	while ( more() ) {
		l.push_back( next() );
	}
}

vector<string> StringSplitter::split() {
	vector<string> l;
	split( l );
	return l;
}

string StringSplitter::join( vector<string>& l , const string& split ) {
	stringstream ss;
	for ( unsigned i=0; i<l.size(); i++ ) {
		if ( i > 0 )
			ss << split;
		ss << l[i];
	}
	return ss.str();
}

vector<string> StringSplitter::split( const string& big , const string& splitter ) {
	StringSplitter ss( big.c_str() , splitter.c_str() );
	return ss.split();
}



