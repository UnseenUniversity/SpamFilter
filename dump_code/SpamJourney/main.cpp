/*
 * main.cpp
 *
 *  Created on: Jun 18, 2014
 *      Author: alexei
 */

#include "items.h"
#include "feature.h"

const char* PLAYGROUND_PATH = "/home/alexei/Desktop/thesis/Corpus/RoMedia2014";
const char* DUMP_PATH = "/home/alexei/Desktop/thesis/Corpus/RoMedia2014/dump";

static std::vector< article* > g_article_db;
static std::vector< comment* > g_comment_db;

static fp_db article_fp_db;
static fp_db comment_fp_db;

class miner{

public:
	std::fstream fs;
	int no_comments = 0;

	std::vector<article*> article_db;
	std::vector<comment*> comment_db;

	char path[MAX_PATH_SIZE];
	int start, end;

	miner( const char* path, int start, int end )
		: start(start), end(end)
	{
		memset(this->path,0,sizeof(this->path));
		memcpy(this->path, path, strlen(path));
	}

	void run(){
		char buffer[MAX_PATH_SIZE];
		for (int i = start; i < end; ++i){
			sprintf(buffer, "%s/%d", path, i);
			this->operator ()(buffer);
		}
	}

	bool operator() (const char* path)
	{
		article* blob = new article();
		article_db.push_back(blob);

		fs.open(path, std::fstream::in);
		fs >> *blob;
		fs.close();

		if (!blob->comments.size()){
			++no_comments;
		} else {
			for_each(i, blob->comments){
				comment_db.push_back(blob->comments[i]);
			}
		}

		return true;
	}

};

bool article_comp_comment_count( article* first, article* second ){
	return first->comments.size() > second->comments.size();
}

static void compute_cluster( article* art, fp_db& db )
{
	std::cerr << "compute cluster\n";

	std::cerr << "for article " << art->id << " with feat count " << art->features.size() << "\n";
	std::cerr << art->title << "\n\n";

	std::fstream fs;
	char buffer[MAX_PATH_SIZE];
	sprintf(buffer, "%s/stats/%d", PLAYGROUND_PATH, art->id);

	fs.open(buffer, std::fstream::out);

	std::unordered_map<std::string, int>& features = art->features;

	std::set<item*> visible_universe;

	for (auto it = features.begin(); it != features.end(); ++it ) {

		hash _hash = feature::compute_hash(it->first);

		std::list< std::pair<int,item*> > visible_planets = db[_hash.value];

		fs << it->first << "\n";
		for (auto planet  = visible_planets.begin();
			  planet != visible_planets.end(); ++planet ){

			fs << " : (" << planet->first << ", " << planet->second->id << ")";
			visible_universe.insert(planet->second);
		}
		fs << "\n";
	}

	fs.close();

	sprintf(buffer, "%s/cluster/%d", PLAYGROUND_PATH, art->id);

	fs.open(buffer, std::fstream::out);

	fs << art->title << "\n";
	fs << "Visbile universe: \n";

	for (auto it = visible_universe.begin(); it != visible_universe.end(); ++it ) {
		fs << (*it)->id << "\n";
		fs << ((article*) *it)->title << "\n";
	}

	fs.close();
}

static int count_articles = 23001;
static int count_comments = 31904;
static int count_threads = 4;

static int g_no_comments = 0;
static int thread_chunk = count_articles / count_threads + 1;

void get_data()
{
	std::vector<miner*> miners;
	for (int i = 1; i < count_articles; i += thread_chunk) {
		miners.push_back( new miner(DUMP_PATH, i, std::min(i+thread_chunk, count_articles+1)) );
	}

	/**
	 * Fork-Join Paradigm
	 */

	std::vector<std::thread> threads;
	for (int i = 0; i < count_threads; ++i) {
		threads.push_back( std::thread(&miner::run, std::ref(*miners[i])));
	}

	for (auto& th : threads) {
		th.join();
	}

	for_each (i, miners){

		g_article_db.insert(g_article_db.end(),
				    miners[i]->article_db.begin(),
				    miners[i]->article_db.end() );

		g_comment_db.insert(g_comment_db.end(),
				    miners[i]->comment_db.begin(),
				    miners[i]->comment_db.end() );

		g_no_comments += miners[i]->no_comments;

		delete miners[i];
	}
}

void process_article( int start, int finish, fp_db* db )
{
	for (int i = start; i < finish; ++i) {
		g_article_db[i]->inject_features(*db);
	}

	std::cerr << "interval " << start << ", " << finish << " processed\n";
}

void process_features()
{
	std::vector<std::thread> threads;
	std::vector< fp_db > dbs(count_threads);

	for (int i = 1; i < count_articles; i += thread_chunk) {
		int start = i;
		int end   = std::min(i + thread_chunk, count_articles);

//		std::cerr << start << " " << end << "\n";
		threads.push_back( std::thread(process_article, start, end, &dbs[i/thread_chunk]));
	}

	for_each (i, threads) {

		threads[i].join();

		for (auto it = dbs[i].begin(); it != dbs[i].end(); ++it)
		{
//			std::cerr << article_fp_db[it->first].size() << " <\n";

			article_fp_db[it->first].insert(
					article_fp_db[it->first].end(),
					it->second.begin(),
					it->second.end()
			);

//			std::cerr << article_fp_db[it->first].size() << " <<\n";
		}

//		std::cerr << article_fp_db.size() << "\n";
	}

	std::cerr << "id " << g_article_db[0]->id << "\n";
	std::cerr << "comm count " << g_article_db[0]->comments.size() << "\n";
	std::cerr << "title " << g_article_db[0]->title << "\n";

	compute_cluster(g_article_db[0], article_fp_db);
}

int main(int argc, char* argv[])
{
	get_data();

	std::cerr << "Article count: " << g_article_db.size() << "\n";
	std::cerr << "Comment count: " << g_comment_db.size() << "\n";
	std::cerr << "Articles with no comments: " << g_no_comments << "\n";


	std::cerr << "id " << g_article_db[2376]->id << "\n";
	std::cerr << "comm count " << g_article_db[2376]->comments.size() << "\n";
	std::cerr << "title " << g_article_db[2376]->title << "\n";
	//std::cerr << "comm " << g_article_db[2376]->comments << "\n";

	return 0;

	std::sort( g_article_db.begin(), g_article_db.end(),
		   article_comp_comment_count );

	std::cerr << "Sorted!\n";
	std::cerr << "id " << g_article_db[0]->id << "\n";
	std::cerr << "comm count " << g_article_db[0]->comments.size() << "\n";
	std::cerr << "title " << g_article_db[0]->title << "\n";
	std::cerr << "text " << g_article_db[0]->text << "\n";

	return 0;
	process_features();

/*
	scan_dir(DUMP_PATH, romedia_miner);

	std::cerr << "Article count: " << article_db.size() << "\n";
	std::cerr << "Comment count: " << comment_db.size() << "\n";
	std::cerr << "Articles with no comments: " << romedia_miner.no_comments << "\n";

	std::sort( article_db.begin(), article_db.end(),
		   article_comp_comment_count );

	std::cerr << "Sorted!\n";
	std::cerr << article_db[0]->comments.size() << "\n";
*/

	return 0;
}



