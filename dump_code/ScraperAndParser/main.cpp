/*
 * main.cpp
 *
 *  Created on: Jun 16, 2014
 *      Author: alexei
 */

#include "util/utils.hpp"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include </home/alexei/jsoncpp-src-0.5.0/include/json/json.h>

const char* G_NEWS_PATH = "/home/alexei/Desktop/thesis/Corpus/RoMedia2014/news";
const char* G_COMM_PATH = "/home/alexei/Desktop/thesis/Corpus/RoMedia2014/news_comments";

const char* RES_DUMP = "/home/alexei/Desktop/thesis/Corpus/RoMedia2014/dump";

const char* OUT_PATH = "ro_media.content";
const char* IDX_PATH = "ro_media.index";

static int comment_id = 0;

class comment{

public:
	comment(){
		id = comment_id++;
		std::cerr << id << "\n";
	}

	int id;
	std::vector< std::string > author;
	std::vector< std::string > text_tokens;
	std::vector< std::string > links;
	struct tm time;

	~comment(){

	}

	friend std::ostream& operator<<(std::ostream& out, const comment& comm );

};

static char dbuffer[MAX_PATH_SIZE];


std::ostream& operator <<(std::ostream& out, const comment & comm ){

	out << comm.id << " " << comm.author.size() << " " << comm.text_tokens.size() << " " << comm.links.size() << "\n";

	//strftime(dbuffer, MAX_PATH_SIZE, "%a %b %d %H:%M:%S %Y", &comm.time);
	//out << dbuffer;

	out << "\n" << comm.author;
	out << "\n" << comm.text_tokens;
	out << "\n" << comm.links;

	out << "\n";

	return out;
}



class data{

public:
	data(){

	}

	~data(){

	}

	int id = 0;
	std::vector< std::string > title_tokens;
	std::vector< std::string > text_tokens;
	std::vector< std::string > links;

	struct tm time;

	friend std::ostream& operator<<(std::ostream& out, const data& blob );
};

std::ostream& operator <<(std::ostream& out, const data& blob ) {

	out << blob.id << "\n";
	out << blob.title_tokens.size() << " " << blob.text_tokens.size() << " " << blob.links.size() << "\n";

	//strftime(dbuffer, MAX_PATH_SIZE, "%a %b %d %H:%M:%S %Y", &blob.time);
	//out << dbuffer;

	out << "\n" << blob.title_tokens;
	out << "\n" << blob.text_tokens;
	out << "\n" << blob.links;
	out << "\n";

	return out;
}


#define LINE_SIZE 50000
char gline[LINE_SIZE];

const char* title_tag = "Title";
int title_sz = 2;

const char* text_tag = "Text";
int text_sz = 2;

const char* auth_tag = "Author";
int auth_sz = 1;


class Parser{

public:

	Parser(){
	}

	int parse_article_id(const char* file)
	{
		int i, ans = 0, digit = 1;

		for (i = 0; file[i]; ++i);
		for (; file[i] != '.'; --i);
		--i;

		while( file[i] != '/' ){
			ans = ans + digit * (file[i] - '0');
			digit *= 10; --i;
		}

		return ans;
	}

	inline void skip_to_char( const char ch ){
		while( *it && *it != ch ) ++it;
		++it;
	}

	void parse_date(struct tm& time, FILE* f)
	{
		while( *it )
		{
			skip_to_char('\"');

			if( *it != 'D' ){
				skip_to_char(',');
			} else {

				while( *it && *it != ':' ) ++it;
				it += 2;

				//ready to parse time;
				char* tmp_it = it;
				while( *it != '\"' ){
					if( *it == 'E' && *(it+1) == 'E' ){
						*it = ' '; ++it;
						*it = ' '; ++it;
						*it = ' '; ++it;
						*it = ' '; ++it;
						break;
					}
					++it;
				}

				it = tmp_it;
				it = strptime(it, "%a %b %d %H:%M:%S %Y\"", &time);
				//std::cerr << "got here " << *it << "\n";
				return;
			}
		}
	}

	std::string token;

	void save_token( std::vector<std::string>& tokens ){

		if( !token.size() ){
			return;
		}

		////std::cerr << token << "\n";
		tokens.push_back(token);
		token.clear();
	}

	void hunt_token( const char tok_1, const char tok_2 ){

		int stack_count = 0;

		do{
			if( *it == tok_1 ){
				++stack_count;
			}
			else {
				if( *it == tok_2 ) --stack_count;
			}

			++it;
		} while( *it && stack_count );

	}

	void parse_tag(const char* tag, const int sz,
		       std::vector<std::string>& tokens,
		       std::vector<std::string>& links)
	{
		while( *it )
		{
			skip_to_char('\"');

			int i;
			for( i = 0; i < sz && tag[i] == *it; ++i, ++it );

			//std::cerr << i << "\n";
			//std::cerr << *it << "\n";

			skip_to_char(':');

			if( i == sz ){

				++it;
				//std::cerr << *it << "!\n";
				//std::cerr << "CONTENT!:\n";

				token.clear();
				while( *it != '\"'){

					//std::cerr << "tempt " << *it << "\n";

					if( (*it < ']' || *it > '`')
					&&  (*it < '%' || *it > '\'')
					&&  (*it < '*' || *it > '-' )
					&&  (*it < '/' || *it > '?')
					&&  (*it < '{' || *it > '~')
					&&  (*it != '[')
					)
					{
						switch( *it ){

							case '\\':
							{
								if( *(it+1) == 'u' ){
									it += 4;
								}
								++it;

								save_token(tokens);
								break;
							}

							case '(':
							case ')':
							case ' ':
							case '.':
							{
								save_token(tokens);
								break;
							}

							default:
							{
								if( *it == 'h' && *(it+1) == 't' && *(it+2) == 't' && *(it+3) == 'p' ){

									std::string link;

									while(*it >= '!' && *it <= '~' && *it != ' ' && *it != '.' ){
										link += *it;
										++it;
									}

									//std::cerr << "LINK: [" << link << "]\n";
									links.push_back(link);

									break;
								}

								token += *it;
							}
						}
					} else {
						save_token(tokens);
						//std::cerr << "forbidden char " << *it << "\n" ;//<< " " << *(it + 2) << "\n";
					}

					++it;
				}

				save_token(tokens);
				//std::cerr << "End of an awesome story\n";

				it += 2;
				return;

			} else {

				///std::cerr << "fdsjakl " << *it << "\n";

				if( *it == '\"'){

//					std::cerr << "ghilimea\n";
					++it;
					while( *it ){
						skip_to_char('"');
						if( *(it-2) != '\\' && *it ){
							//std::cerr << *(it-6)<< *(it-5) << *(it-4) << *(it-3) << *(it-2) << " ~~ " << *it << "\n";
							break;
						}
						++it;
						//std::cerr << (int) *it;
					}

				} else if( *it == '['){
					hunt_token('[',']'); // json array
				}

				++it;
				//std::cerr << *it << " " << *(it+1) << "\n";
			}
		}
	}

	char *it;

	bool process_article(FILE* f, data &blob){

		if( !fgets(gline, sizeof(gline), f) ){
			return false;
		}

		it = gline + 1; // skip {
		//parse_date(blob.time, f);
		parse_tag(title_tag, title_sz, blob.title_tokens, blob.links );
		parse_tag(text_tag, text_sz, blob.text_tokens, blob.links );

		fclose(f);

		return true;
	}

	bool process_comment(FILE* f, comment &new_comm){

		if( !fgets(gline, sizeof(gline), f) ){
			return false;
		}

		it = gline + 1;
		//parse_date(new_comm.time, f);
		parse_tag(auth_tag, auth_sz, new_comm.author, new_comm.links );
		parse_tag(text_tag, text_sz, new_comm.text_tokens, new_comm.links );

		fclose(f);
		return false;
	}

	bool operator()(const char* path)
	{
		data blob;

		int comment_count = 1;
		std::string gpath(G_COMM_PATH);
		FILE* commp = NULL;
		char tpath[MAX_PATH_SIZE];

		FILE* f = safe_fopen(path, "r");
		blob.id = parse_article_id(path);
//		std::cerr << path << "\n" << blob.id << "\n";

		sprintf(tpath, "%s/%d", RES_DUMP, blob.id);
		std::fstream fout (tpath, std::fstream::out);

		if (!process_article(f, blob) ){
			return false;
		}
		fout << blob;

		std::vector< comment > comments;

		while(true)
		{
			sprintf(tpath, "%s/%d_comment_%d.json", G_COMM_PATH, blob.id, comment_count);
			commp = fopen( tpath, "r" );

			if( commp == NULL ){
				break;
			}

			//process comment
			comment new_comm;
			process_comment(commp, new_comm);

			++comment_count;
			comments.push_back(new_comm);
		}

		fout << comment_count-1 << "\n" << comments;
		//std::cerr << "num_comments " << comment_count << "\n";

		fout.close();
		return true;
	}

	~Parser(){
	}

};




int main(int argc, char* argv[])
{

//	  std::string input_json = "{\n \"mynestedvalue\" : {\n   \"value2\" : \"A string\" \n },\n \"myinteger\" : 42, \n \"mystring\" : \"John\" \n}\n";
//
//	  std::cout << input_json << std::endl;
//
//	  Json::Value values;
//	  Json::Reader reader;
//	  reader.parse( input_json, values);
//
//	  // second argument to get(a,b) is a default value that will be assigned in case of a parsing error
//	  std::string mystring_value = values.get("mystring","defaultstring").asString();
//	  std::cout << "Mystring: " << mystring_value << std::endl;
//	  int myinteger_value = values.get("myinteger", Json::Value(0) ).asInt();
//	  std::cout << "Myinteger: " << myinteger_value << std::endl;
//
//	  Json::Value mynested_value = values.get("mynestedvalue", Json::Value(0) );
//	  std::string value2_string = mynested_value.get("value2","default").asString();
//
//	  std::cout << "value2: " << value2_string << std::endl;
//
//
//	std::string var("5 f:)");//Ăn 1 ărk Âzoie aâl Îch î Ș ș Ț ț");

	Parser parser;
	scan_dir(G_NEWS_PATH, parser);

//	std::string date("Sun Jan 19 00:05:17 EET 2014\" ");
//	struct tm tm;
//
//	char *p = strptime(date.c_str(), "%a %b %d %H:%M:%S EET %Y", &tm);
//
//	std::cerr << *p;
//
//	printf("year: %d; month: %d; day: %d;\n",
//	        tm.tm_year, tm.tm_mon, tm.tm_mday);
//	printf("hour: %d; minute: %d; second: %d\n",
//	        tm.tm_hour, tm.tm_min, tm.tm_sec);
//	printf("week day: %d; year day: %d\n", tm.tm_wday, tm.tm_yday);



//	FILE *cp, *ip;
//	cp = safe_fopen(OUT_PATH, "w");
//	ip = safe_fopen(IDX_PATH, "w");
//
//
//
//
//	safe_fclose(cp, OUT_PATH);
//	safe_fclose(ip, IDX_PATH);


	return 0;
}



