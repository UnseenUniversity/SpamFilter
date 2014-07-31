/*
 * feature_selector.h
 *
 *  Created on: Jun 18, 2014
 *      Author: alexei
 */

#ifndef FEATURE_SELECTOR_H_
#define FEATURE_SELECTOR_H_

#include "util/utils.hpp"

union hash{
	long long value;
	struct{
		int h1;
		int h2;
	};
};

class item;

// fingerprint database
typedef std::unordered_map<long long, std::list<std::pair<int,item*>>> fp_db;


namespace feature {

	enum mode{
		OSB,		// Orthogonal Sparse Bigram
		SBPH,		// Sparse Binary Polynomial Hashing
		UNIGRAM
	};

	const mode current_mode = UNIGRAM;

	inline void unigram_miner(std::unordered_map< std::string, int >& features,
				  std::vector<std::string>& tokens)
	{
		for (auto it = tokens.begin(); it != tokens.end(); ++it ) {
			features[*it]++;
		}
	}

	static void feature_miner(std::unordered_map< std::string, int >& features,
			   	  std::vector<std::string>& tokens) {

		switch(current_mode) {

			case UNIGRAM:
			{
				unigram_miner(features,tokens);
				break;
			}

			case OSB:
			{
				break;
			}

			case SBPH:
			{
				break;
			}
		}

	}


	static const int def_salt = 73;
	static const int MOD1 = 100007;
	static const int MOD2 = 100021;

	static hash compute_hash(const std::string& token){

		hash hash;
		_for(i, token.size()){
			hash.h1 = (hash.h1 * def_salt + token[i]) % MOD1;
			hash.h2 = (hash.h2 * def_salt + token[i]) % MOD2;
		}

		return hash;
	}

	static void inject( std::unordered_map<std::string, int>& features, item* data_pointer, fp_db& db )
	{
		for (auto it = features.begin(); it != features.end(); ++it ) {

			hash _hash = compute_hash(it->first);

			//std::cerr << _hash.value << "\n";
			//std::cerr << db[_hash.value].size() << "\n";

			db[_hash.value].push_back(
				std::make_pair<int,item*>((int)it->second,
							  (item*)data_pointer) );
		}
	}
}





#endif /* FEATURE_SELECTOR_H_ */
