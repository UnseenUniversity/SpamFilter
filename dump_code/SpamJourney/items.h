/*
 * items.h
 *
 *  Created on: Jun 18, 2014
 *      Author: alexei
 */

#ifndef ITEMS_H_
#define ITEMS_H_

#include "util/utils.hpp"
#include "feature.h"

class item{

public:
	item() {

	}

	int id = 0;
	struct tm date;
	std::vector< std::string > text;
	std::vector< std::string > links;

	std::unordered_map< std::string, int > features;

	void inject_features( std::vector<std::string>& payload, fp_db& db ) {

		//feature::feature_miner(features, text);
		feature::feature_miner(features, payload);

		//std::cerr << payload.size() << " feat " << features.size() << "\n";

		feature::inject(features, this, db);

	}



	~item(){

	}

	friend std::istream& operator>>(std::istream& in, item* item);

};

class article;
class comment;

class article : public item {

public:
	article() {

	}

	std::vector< std::string > title;

	std::vector< comment* > comments;
	std::vector< article* > article_cluster;

	~article(){

	}

	inline void inject_features(fp_db& db){
		((item*) this)->inject_features(title, db);
	}

	bool operator<(article &other){
		return other.comments.size() - comments.size();
	}

	friend std::istream& operator>>(std::istream& in, article& article);
	friend std::ostream& operator<<(std::ostream& out, article& article);
};

class comment : public item {

public:
	comment() {

	}

	std::vector< std::string > author;

	std::vector< comment* > comments_cluster;
	article* post_origin = NULL;

	~comment() {

	}

	inline void inject_features(fp_db& db){
		((item*) this)->inject_features(author, db);
	}

	friend std::istream& operator>>(std::istream& in, comment& comment);
	friend std::ostream& operator<<(std::ostream& out, comment& comment);
};



#endif /* ITEMS_H_ */
