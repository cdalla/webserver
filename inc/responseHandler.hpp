#ifndef RESPONSEHANDLER_HPP
# define RESPONSEHANDLER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include "structs.hpp"

class Client;

class responseHandler {

    private:


		Client*			_ptr;
		std::string		_file;
		std::string 	_content_type;
		std::string 	_body;
		std::string 	_root;
		std::list<std::string> _cgi_ext;
		std::string		_upload_dir;
		std::string 	_response;
		char**			_env;
		std::list<std::string>		_index;
		bool 			_autoindex;
		
		

		std::string 	_getStatusMessage(int error);
		void			_determineType( std::string path );
		void			_createErrorPage( int error );
		void 			_handleError( int error );
		void 			_handlePage( std::string path );
		void 			_handleDirectory( std::string path );
		void 			_handleCGI( std::string path );
		void 			_createEnv( void );
		void 			_createResponse( void );
		void 			_locationHandler( std::string path );
		void 			_handleDirRequest( std::string path );
	public:

        responseHandler( Client *ptr );
        ~responseHandler( void );

        std::string	get( void );
		

};

#endif
