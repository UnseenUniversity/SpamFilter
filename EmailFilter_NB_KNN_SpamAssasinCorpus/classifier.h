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

		os << "Stats: \n,";
		for_each(i, classes){
			os << classes[i]->name << ",";
		}
		os << "pos_perc," << "neg_perc\n";

		for_each(i, classes){

			int positives = confusion_matrix[i][i];
			int negatives = 0;
			int total;

			os << classes[i]->name << ",";
			for_each(j, classes){
				os << confusion_matrix[i][j] << ",";

				if( abs(i-j) > 1 ){
					negatives += confusion_matrix[i][j];
				} else {
					positives += confusion_matrix[i][j];
				}
			}

//			negatives -= positives;
			total = negatives + positives;
			os << (positives * 100 / total) << "," << (negatives * 100 / total) << "\n";
		}

		os << "\n";
		os.flush();
	}

	void update_matrix( object& obj, int best_fit ){

		if( obj.assig != NULL ){
			confusion_matrix[ obj.assig->id ][ best_fit ]++;
			return;
		}

		std::cout << "Test result!\n";
		for(auto token = obj.features.begin(); token != obj.features.end(); ++token ){
			std::cout << token->first << " : ";
		}
		std::cout << "\nbest fit " << classes[ best_fit ]->name << "!!\n";
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

	void train( std::vector<int>& sample_perc ){

		for_each(i, classes){
			classes[i]->randomize_object_set();
			classes[i]->reset_features();
		}
		def_class::vocabulary.clear();

		for_each(i, classes){

			std::vector<object*> &objects = classes[i]->objects;

			int set_size = objects.size();
			int sample_size = (100-sample_perc[i]) * set_size / 100;

			_for(j, sample_size){
				for( auto k = objects[j]->features.begin();
						  k != objects[j]->features.end(); ++k ){
					classes[i]->train_feature( k->first );
				}
			}
		}
	}

	void classify( std::vector<object*>& obj){
		for_each(i, obj){
			multinomial_boolean_prob( *obj[i] );
		}
	}

	void classify( std::vector<int>& sample_perc )
	{
		train(sample_perc);
		for_each(i, classes){

			std::vector<object*> &objects = classes[i]->objects;

			int set_size = objects.size();
			int sample_begin = set_size - (sample_perc[i]) * set_size / 100;

			for( int i = sample_begin; i < set_size; ++i ){
				this->multinomial_boolean_classic(*objects[i]);
			}
		}
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

		update_matrix(obj, best_fit);
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

		update_matrix(obj, best_fit);
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

	void classify( std::vector<object*>& objects )
	{
		std::vector<int> sample_perc( classes.size(), 0);
		for_each(i, objects){
			knn(sample_perc, *objects[i]);
		}
	}

	void classify( std::vector<int>& sample_perc )
	{
		for_each(i, classes){
			classes[i]->randomize_object_set();
			classes[i]->reset_features();
		}

		for_each(i, classes){
			std::cerr << "Extracting tests from class " << i << "\n";

			std::vector<object*> &objects = classes[i]->objects;

			int set_size = objects.size();
			int sample_begin = set_size - (sample_perc[i]) * set_size / 100;
			for( int j = sample_begin; j < set_size; j += 50 ){
					knn( sample_perc, *objects[j]);
			}
		}
	}


	void knn( std::vector<int>& sample_perc, object& obj )
	{
		auto comp = []( const std::pair<int,int> a,
						const std::pair<int,int> b ){
							return a.first > b.first;
					  };

		std::priority_queue< std::pair<int,int>,
							 std::vector< std::pair<int,int> >,
							 decltype( comp ) > results(comp);

		for_each( i, classes ){

			std::vector<object*> &objects = classes[i]->objects;
			int set_size = objects.size();
			int sample_size = (100-sample_perc[i]) * set_size / 100;

			_for(j, sample_size){

				int dist = 0, common = 0;
				for( auto k = objects[j]->features.begin();
						  k != objects[j]->features.end();
						  ++k ){

					auto exists = obj.features.find(k->first);
					if( exists != obj.features.end() ){
						//double d1 = (double)exists->second / obj.features.size();
						//double d2 =
//						std::cerr << d1 << " " << d2 << "\n";
						//common += (d1 - d2) * (d1 - d2) ;
						common ++;
					} else{
//						common += 1;//(double)k->second / objects[j]->features.size();
					}
				}

				dist = objects[j]->features.size() + obj.features.size() - 2 * common;
				//dist = common;

				if( !dist ){
					update_matrix(obj, i);
					return;
				}

				results.push( std::make_pair(dist,i) );
				if( (int) results.size() > K ){
					results.pop();
				}
			}
		}

		std::vector<double> class_score( classes.size(), 0.0f );
		std::vector<int> class_count( classes.size(), 0.0f );

		double dist_max = 0, dist_min = 0;

		while( !results.empty() ){
			std::pair<int,int> res = results.top();
			results.pop();
			class_score[ res.second ] += res.first * res.first;
			class_count[ res.second ] += 1;

			if( !dist_max || res.first > dist_max ){
				dist_max = res.first;
			}

			if( !dist_min || res.first < dist_min ){
				dist_min = res.first;
			}
		}

		double best_score = 0;
		int best_fit = 0;

//		if( dist_max == dist_min ){ // equal voting
//			for_each( i, classes ){
//				if( !i || class_count[i] > best_score ){
//					best_score = class_count[i];
//					best_fit = i;
//				}
//			}
//		}
//		else{
//			double numitor = dist_max - dist_min;
//			for_each(i, class_count){
//
//				double score = (double) (class_count[i] * dist_max - class_score[i]) / numitor;
//
//				if( !i || score > best_score ){
//					best_score = class_count[i];
//					best_fit = i;
//				}
//			}
//		}

		for_each( i, classes ){

			if( !class_score[i] ){
				best_fit = i;
				break;
			}

			if( !i || score_weighted(class_score[i]) < best_score ){
				best_score = score_weighted(class_score[i]);
				best_fit = i;
			}
		}

		update_matrix(obj, best_fit);
	}

	double score_weighted( int dist ){
		return 1/dist;
	}

	double score_bool(int dist){
		return 1;
	}
};




#endif /* CLASSIFIER_H_ */
