/*
 * classifier.h
 *
 *  Created on: May 18, 2014
 *      Author: alexei
 */

#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#include "util/utils.hpp"
#include "classes.hpp"


class default_classifier{

public:

	default_classifier(std::vector<def_class*>& classes)
		: classes(classes),
		  confusion_matrix( classes.size(), std::vector<int>(classes.size(), 0))
	{

	}

	virtual void classify( std::vector<int>& sample_perc ) = 0;


	std::vector<def_class*>& classes;
	std::vector< std::vector<int> > confusion_matrix;

	void reset_stats(){

			for_each(i, classes){
				for_each(j, classes){
					confusion_matrix[i][j] = 0;
				}
			}

		}

	void print_stats(std::ostream& os){

		os << "Stats: \n";

		os << "\t";
		for_each(i, classes){
			os << classes[i]->name << "\t";
		}
		os << "pos_perc\t" << "neg_perc\t";

		os << "\n";

		for_each(i, classes){

			int positives = confusion_matrix[i][i];
			int negatives = 0;
			int total;

			os << classes[i]->name << "\t";
			for_each(j, classes){
				os << confusion_matrix[i][j] << "\t";
				negatives += confusion_matrix[i][j];
			}

			negatives -= positives;
			total = negatives + positives;

			os << (positives * 100 / total) << "\t\t" << (negatives * 100 / total) << "\n";

			os << "\n";
		}

		os << "\n";
	}
};

class naive_bayes : public default_classifier{

public:

	naive_bayes(std::vector<def_class*>& classes)
		: default_classifier(classes)
	{

	}

	void k_fold_classification( int k ){

		int obj_count = 0;
		for_each(i, classes){
			classes[i]->randomize_object_set();
			obj_count += classes[i]->objects.size();
		}

		// ...code
	}

	void classify( std::vector<int>& sample_perc )
	{
		for_each(i, classes){
			classes[i]->randomize_object_set();
			classes[i]->reset_features();
		}
		def_class::vocabulary.clear();

//		std::cerr << "reset done!\n";

		for_each(i, classes){

			std::vector<object*> &objects = classes[i]->objects;

			int set_size = objects.size();
			int sample_size = (100-sample_perc[i]) * set_size / 100;

			_for(j, sample_size){
				for( auto k = objects[j]->features.begin();
						  k != objects[j]->features.end(); ++k )
				{
					classes[i]->train_feature( k->first );
				}
			}
		}

//		std::cerr << "train done!\n";
		for_each(i, classes){

			std::vector<object*> &objects = classes[i]->objects;

			int set_size = objects.size();
			int sample_begin = set_size - (sample_perc[i]) * set_size / 100;

			for( int i = sample_begin; i < set_size; ++i ){
				this->multinomial_boolean_classic(*objects[i]);
			}
		}

//		std::cerr << "test done!\n";
	}

	void multinomial_boolean_classic( object &obj ){

		int best_fit = 0;
		double best_score = 0.;

		double voc_size = def_class::vocabulary.size();

		for_each(i, classes){

			def_class& curr_class = *classes[i];

			double score = 0.0f;
			double numitor = voc_size + curr_class.features.size();

			for(auto token = obj.features.begin(); token != obj.features.end(); ++token )
			{
				double frecv = curr_class.features[token->first];
				double prob = log( (frecv + 1 ) / numitor );
				score += pow(prob, frecv);
			}

			if( !i || score > best_score ){
				best_score = score;
				best_fit = i;
			}
		}

		confusion_matrix[ obj.assig->id ][ best_fit ]++;
	}

	void multinomial_boolean_prob( object &obj ){

		int best_fit = 0;
		double best_score = 0.;

		double voc_size = def_class::vocabulary.size();

		for_each(i, classes){

			def_class& curr_class = *classes[i];

			double score = 0.0f;
			double numitor = voc_size + curr_class.features.size();

			for(auto token = obj.features.begin(); token != obj.features.end(); ++token )
			{
				double frecv = curr_class.features[token->first];
				score += log( (frecv + 1 ) / numitor );
			}

			if( !i || score > best_score ){
				best_score = score;
				best_fit = i;
			}
		}

		confusion_matrix[ obj.assig->id ][ best_fit ]++;
	}


};

class knn_classifier : public default_classifier{

public:

	int K;

	knn_classifier(std::vector<def_class*>& classes, int K)
			: default_classifier(classes),
			  K(K)
	{

	}

	void classify( std::vector<int>& sample_perc )
	{
		for_each(i, classes){
			classes[i]->randomize_object_set();
			classes[i]->reset_features();
		}

		for_each(i, classes){

			std::vector<object*> &objects = classes[i]->objects;

			int set_size = objects.size();
			int sample_begin = set_size - (sample_perc[i]) * set_size / 100;

			for( int j = sample_begin; j < set_size; ++j ){
				knn( sample_perc, *objects[j]);
			}

		}
	}


	void knn( std::vector<int>& sample_perc, object& obj )
	{
		auto comp = []( const std::pair<int,int> a,
						const std::pair<int,int> b ){
							return a.first < b.first;
					  };

		std::priority_queue< std::pair<int,int>,
							 std::vector< std::pair<int,int> >,
							 decltype( comp ) > results(comp);

		for_each( i, classes ){

			std::vector<object*> &objects = classes[i]->objects;
			int set_size = objects.size();
			int sample_size = (100-sample_perc[i]) * set_size / 100;

			_for(j, sample_size){

				int dist = 0;
				int common = 0;

				for( auto k = objects[j]->features.begin();
						  k != objects[j]->features.end();
						  ++k ){
					common += obj.features[k->first];
				}

				dist = objects[j]->features.size() + obj.features.size() - 2 * common;

				if( !dist ){
					confusion_matrix[ obj.assig->id ][ i ]++;
					return;
				}

				results.push( std::make_pair(dist,i) );

				if( (int) results.size() > K ){
					results.pop();
				}
			}
		}

		std::vector<double> class_score( classes.size(), 0.0f );

		while( !results.empty() ){
			std::pair<int,int> res = results.top();
			results.pop();
			class_score[ res.second ] += score_weighted( res.first );
		}

		double best_score = 0;
		int best_fit = 0;

		for_each( i, classes ){
			if( !i || class_score[i] < best_score ){
				best_score = class_score[i];
				best_fit = i;
			}
		}

		confusion_matrix[ obj.assig->id ][ best_fit ]++;
	}

	double score_weighted( int dist ){
		return 1/dist;
	}

	double score_bool(int dist){
		return -1;
	}


};




#endif /* CLASSIFIER_H_ */
