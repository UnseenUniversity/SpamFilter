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

        }

        struct stat buf;
        int fd, pagesize;
        char* data;

        int it, file_size;

        virtual bool operator() (const char* path) = 0;
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
        mail_parser(){
                pagesize = getpagesize();
        }

        inline bool check_newline(){
                return data[it] == '\n';
        }

        inline bool check_newline(int it){
                return data[it] == '\n';
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

                while( it < file_size && data[it] ){
                        //fprintf(stdout, "%c", data[it]);

                    	//fprintf(stdout,"%d %c %d\n", it, data[it], file_size);
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


        void setup_file( const char* path )
        {
        	_assert(current_class != NULL, "Forgot to setup class\n");
        	current_class->inject_new_object();

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

        bool operator() (const char* path)
        {
			setup_file(path);

//			std::cout << path << "\n";
			parse_header();
			parse_body();
//			std::cout << path << "\n";

			release_file();
			return true;
        }

        void init_ignore_list( const char* file ){
			std::fstream fs (file, std::fstream::in);
			std::string token;
			while( fs >> token ){
					ignore_list.insert(token);
			}
			fs.close();
        }

        std::unordered_set<std::string> ignore_list;

        def_class* current_class;

        void bind_class(def_class* current_class){
        	this->current_class = current_class;
        }


        static const int MAIL_SIZE = 128;
        char email[MAIL_SIZE];

        static const int URL_SIZE = 128;
        char url[URL_SIZE];

        static const int TOKEN_SIZE = 10;
        char token[TOKEN_SIZE];
};




#endif /* PARSER_H_ */
