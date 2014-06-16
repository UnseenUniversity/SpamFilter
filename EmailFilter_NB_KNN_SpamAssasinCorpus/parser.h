/*
 * parser.h
 *
 *  Created on: May 18, 2014
 *      Author: alexei
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "util/utils.hpp"
#include "classes.hpp"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

class parser{


public:
        parser(){
            pagesize = getpagesize();
        }

        struct stat buf;
        int fd = -1, pagesize;
        char* data = NULL;

        int it = -1, file_size = -1;

        inline bool check_newline(){
                return data[it] == '\n';
        }

        inline bool check_newline(int it){
                return data[it] == '\n';
        }

        virtual bool operator() (const char* path) = 0;

        void init_ignore_list( const char* file ){
			std::fstream fs (file, std::fstream::in);
			std::string token;
			while( fs >> token ){
					ignore_list.insert(token);
			}
			fs.close();
        }

        std::unordered_set<std::string> ignore_list;

        void setup_file( const char* path )
        {
        	data = NULL;
			fd = open(path, O_RDONLY);
			_assert( fd >= 0, "fail to open file %s", path );

			fstat(fd, &buf);
			file_size = buf.st_size;
			it = 0; // place iterator at start
			data = (char*) mmap( (caddr_t*) 0, file_size + 2, PROT_READ, MAP_SHARED, fd, (off_t) 0 );
			_assert( *data != -1, "fail to mmap" );
        }

        void release_file(){
            _assert( munmap( data, file_size + 2 ) != -1, "fail un unmap\n");
            close(fd);
        }

        def_class* current_class = NULL;
        void bind_class(def_class* current_class){
        	this->current_class = current_class;
        }

        virtual ~parser(){

        }
};

static std::vector<std::string> special_tokens = {
		"This is a multipart MIME message",
		"Content-",
		"--==_",
		"-----",
		"http://",
		"From:"
};

static const int SCRAPE_BEGIN = 0;
static const int SCRAPE_END = 4;

static const int HTTP_TOK = 4;
static const int FROM_TOK = 5;

class mail_parser : public parser{

public:
        mail_parser() : parser(){
        }


        inline bool check_token( int tok_id )
        {
    		int sz = it + special_tokens[tok_id].size();
    		if( it + sz >= file_size ){
    			return false;
    		}

    		if( !strncmp(data+it, special_tokens[tok_id].c_str(), sz) ){
    			it += sz;
    			return true;
    		}

    		return false;
        }

        inline bool check_scrape_token(){

        	_forf(i, SCRAPE_BEGIN, SCRAPE_END){
        		if( check_token(i) ){
        			return true;
        		}
        	}
        	return false;
        }

        inline void scan_for_email()
        {
			int counter = 0;
			while( it < file_size )
			{
				if(counter == MAIL_SIZE){
					return;
				}

				switch( data[it] )
				{
						case ' ':
						{
							counter = 0; ++it;
							break;
						}

						case '@':
						{
							while( check_newline() &&
								   it < file_size  &&
								   counter < (MAIL_SIZE-1) ){
								 email[counter++] = data[it++];
							}
							email[counter] = 0;

							std::string email_str(email);
							current_class->inject(email_str);

//                              fprintf(stderr,"%s\n", email);
							return;
						}

						case '\n':
							return;

						default:
								email[counter++] = data[it++];
				}
			}
        }

        void parse_header(){

                while( it < file_size ){

                        if( check_newline() ){
                                if( check_newline(it+1) ){
                                        ++it; return;
                                }
                        }
                        else if( check_token(FROM_TOK) ){
                                scan_for_email();
                        }

                        ++it;
                }
        }

        bool is_relevant_char(){
                return isalpha(data[it]) ||
                       isdigit(data[it]) ||
                       data[it] == '\''  ||
                       data[it] == '-';
        }

        void get_token(){

                int len = 0;
                while( it < file_size && len < (TOKEN_SIZE-1) && is_relevant_char() ){
                        token[len++] = tolower(data[it++]);
                }

                if( is_relevant_char() ){

                        do{
                                ++it;
                        } while( it < file_size && is_relevant_char() );

                        len = 1;
                } // skip long, uninteresting words
                else{
                        while( len >= 1 &&
                        	   (token[len-1] == '\'' || token[len-1] == '-' ) )
                        {
                            --len;
                        }

                }

                if( len <= 1 )
                     return;

                token[len] = 0;
                std::string token_str(token);
                if( ignore_list.find(token_str) != ignore_list.end() ){
                	return;
                }
                current_class->inject(token_str);

//              fprintf(stdout,"token %s\n", token);
        }

        inline bool is_url_char(){
                return data[it] >= '!' &&
                	   data[it] <= '~';
        }


        void get_url(){

			int len = 0;

			while( it < file_size && len < (URL_SIZE-1) && is_url_char() ){
					url[len++] = data[it++];
			}

			while( it < file_size && is_url_char() ){
					++it;
			} // skip extra-long urls
			url[len] = 0;

			std::string url_str(url);
			current_class->inject(url_str);
//			fprintf(stdout,"url: %s\n", url);
        }

        void munch_to_newline()
        {
        	while( it < file_size && data[it] != '\n' && data[it] != 0 )
        		++it;
        	++it;
        }

        void parse_body(){

			while( it < file_size && data[it] )
			{
				if( isalpha(data[it]) )
				{
					if( check_token(HTTP_TOK) ){
							get_url();
					}
					else if( check_scrape_token() ){
							munch_to_newline();
					}
					else {
						get_token();
					}
				}
				else {
					++it;
				}
			}
        }



        bool operator() (const char* path)
        {
        	_assert(current_class != NULL, "Forgot to setup class\n");
        	current_class->inject_new_object();

			setup_file(path);

			parse_header();
			parse_body();

			release_file();
			return true;
        }


        static const int MAIL_SIZE = 128;
        char email[MAIL_SIZE];

        static const int URL_SIZE = 128;
        char url[URL_SIZE];

        static const int TOKEN_SIZE = 10;
        char token[TOKEN_SIZE];
};

class kelog_parser : public parser{

public:

	std::vector<def_class*>& classes;
	kelog_parser( std::vector<def_class*>& classes ) :
		 parser(), classes(classes)
	{

	}

	void skip_to_tok( char tok ){
		while( data[it++] != tok );
	}

	void skip_line(){
		skip_to_tok('\n');
	}

	void parse_header(){
		skip_to_tok('\t');
		skip_to_tok('\t');
	}

	static const int TOK_SIZE = 128;
	char token[TOK_SIZE];
	std::vector< std::string > tok_str;

	void parse_body( char delim ){

		int len = 0;
		tok_str.clear();

		while( it < file_size && data[it] != delim ){

			if( data[it] == ' ' ){

				if( len ){
					token[len] = 0;
					std::string token_str(token);
					//if( ignore_list.find(token_str) == ignore_list.end() ){
						tok_str.push_back( token_str );
					//}
				}

				len = 0;
			}
			else {
				token[len++] = data[it];
			}

			++it;
		}

		if( len ){
			token[len] = 0;
			std::string token_str(token);
			//if( ignore_list.find(token_str) == ignore_list.end() ){
				tok_str.push_back( token_str );
			//}
		}

//		for_each( i, tok_str ){
//			std::cerr << tok_str[i] << " ";
//		}
//		std::cerr << "\n";
	}

	void parse_class(){

		++it;
		if( tok_str.size() > 0 )
		{
			int _class_id = data[it] - '0';
			_assert( _class_id >= 0 && _class_id <= 4, "Invalid class id %d |%c|", it, (char)data[it]);

			classes[ _class_id ]->inject_new_object();
			for_each( i, tok_str ){
				classes[ _class_id ]->inject(tok_str[i]);
			}

			tok_str.clear();
		}

		it += 2;
	}

	void parse_class_obj(std::vector<object*>& objects){

		++it;

		if( tok_str.size() > 0 )
		{
			objects.push_back( new object(NULL) );

			for_each( i, tok_str ){
				objects[ objects.size() - 1 ]->inject(tok_str[i]);
			}

			tok_str.clear();
		}

	}

	bool operator() (const char* path)
	{
		setup_file(path);

		skip_line();
		while( it < file_size ){
			parse_header();
			parse_body('\t');
			parse_class();
		}

		release_file();
		return true;
	}

	bool operator() (const char* path, std::vector<object*>& objs){

		setup_file(path);

		skip_line();
		while( it < file_size ){
			parse_header();
			parse_body('\n');
			parse_class_obj(objs);
		}

		release_file();
		return true;
	}

};



#endif /* PARSER_H_ */
