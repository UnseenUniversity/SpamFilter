/*
 * default_class.hpp
 *
 *  Created on: May 18, 2014
 *      Author: alexei
 */

#ifndef DEFAULT_CLASS_HPP_
#define DEFAULT_CLASS_HPP_

#include "util/utils.hpp"

class def_class;

class object{

public:

	object( def_class* assignment )
		: assig(assignment)
	{

	}

	void inject( std::string& token ){
		features[token]++;
		feature_count += 1;
	}

	int classification = -1;
	def_class* assig;

	std::unordered_map<std::string, int> features;
	int feature_count = 0;

	friend std::ostream& operator<< (std::ostream& os, const object& obj );
};

class def_class{

public:
		std::string name;
		int id;

		def_class(const char* name, int id) :
			name(name), id(id)
		{
		}

		void inject( std::string& token ){
			_assert( current_object != NULL, "invalid object");
			current_object->inject(token);
		}

		inline void train_feature( const std::string& feat ){
			features[feat]++;
			vocabulary.insert(feat);
		}

		inline void reset_features(){
			features.clear();
		}

		void randomize_object_set() {
			srand( time(NULL) );
			_forb(i, objects.size()-1, 1 ){
				int random_item = rand() % (i+1);
				std::swap( objects[i], objects[random_item] );
			}
		}

		void inject_new_object(){

			objects.push_back( new object(this) );
			current_object = objects[ objects.size()-1 ];
		}

		std::vector<object*> objects;
		object* current_object = NULL;

		static std::unordered_set<std::string> vocabulary;
		std::unordered_map<std::string, int> features;
        int feature_count = 0;

    	friend std::ostream& operator<< (std::ostream& os, const def_class& def_class);
};

std::unordered_set<std::string> def_class::vocabulary;

std::ostream& operator<< (std::ostream& os, const object& obj ){

	os << "Object tokens:\n";

	for ( auto it = obj.features.begin(); it != obj.features.end(); ++it )
		os << " " << it->first << ":" << it->second << "\n";

	os << "\n";

	return os;
}

std::ostream& operator<< (std::ostream& os, const def_class& _class)
{
	os << "Class " << _class.name << "\n";

	for( auto it = _class.objects.begin(); it != _class.objects.end(); ++it ){
		os << *it;
	}

	os << "\n\n";
	return os;
}


#endif /* DEFAULT_CLASS_HPP_ */
