/*
 * main.cpp
 *
 *  Created on: May 18, 2014
 *      Author: alexei
 */

#include "util/utils.hpp"

#include "classifier.h"
#include "classes.hpp"
#include "parser.h"


const char* ham_corpus  = "/home/alexei/workspace/SpamFilter/corpus/ham";
const char* spam_corpus = "/home/alexei/workspace/SpamFilter/corpus/spam";
const char* stopwords = "/home/alexei/workspace/SpamFilter/corpus/stopwords.list";

void spam_filter(){

        mail_parser parser;
        parser.init_ignore_list(stopwords);

        def_class ham("ham", 0);
        parser.bind_class(&ham);
        scan_dir<mail_parser> (ham_corpus, parser);

        def_class spam("spam", 1);
        parser.bind_class(&spam);
        scan_dir<mail_parser> (spam_corpus, parser);

        std::vector<def_class*> classes;
        classes.push_back(&ham);
        classes.push_back(&spam);

        std::fstream fs;
        fs.open ("mn_nb_boolean_spam.results", std::fstream::out);

        fs << "ham documents," << ham.objects.size() << "\n";
        fs << "spam documents," << spam.objects.size() << "\n";

        naive_bayes classifier_naive(classes);

        std::cerr << "Ready to rumble!\n";
        for( int p1 = 10; p1 < 100; p1 += 10 )
        	for( int p2 = 10; p2 < 100; p2 += 10 ){

        		classifier_naive.reset_stats();
        		fs << "sample percentage: " << p1 << "," << p2 << "\n";

        		std::vector<int> sample_perc = { p1, p2 };
				classifier_naive.classify(sample_perc);
				classifier_naive.print_stats(fs);
        	}

//        knn_classifier knn(classes, 9);
//
//        knn.reset_stats();
//		std::vector<int> sample_perc = { 1, 1 };
//		knn.classify(sample_perc);
//		knn.print_stats(fs);
//
//        for( int p1 = 10; p1 < 100; p1 += 10 ){
//			knn.reset_stats();
//        	std::vector<int> sample_perc = { p1, p1 };
//			knn.classify(sample_perc);
//			knn.print_stats(fs);
//        }

        fs.close();
}

const char* kellog_train_corpus = "/home/alexei/workspace/SpamFilter/corpus/kellog/train.tsv";
const char* kellog_test_corpus = "/home/alexei/workspace/SpamFilter/corpus/kellog/test.tsv";

void sent_filter(){

	std::vector<def_class*> classes;
	classes.push_back( new def_class("very negative", 0) );
	classes.push_back( new def_class("negative", 1) );
	classes.push_back( new def_class("neutral", 2) );
	classes.push_back( new def_class("positive", 3) );
	classes.push_back( new def_class("very positive", 4) );

	kelog_parser parser(classes);
	parser.init_ignore_list(stopwords);
	parser(kellog_train_corpus);


    std::fstream fs;
    fs.open ("knn-21-kellog.results", std::fstream::out);

    knn_classifier knn(classes, 21);

    for( int p1 = 10; p1 < 100; p1 += 10 ){
    	std::vector<int> sample_perc = { p1, p1, p1, p1, p1 };
		knn.classify(sample_perc);
		knn.print_stats(fs);
    }

//	naive_bayes classifier_naive(classes);
//	for( int p1 = 10; p1 < 100; p1 += 10 ){
//
//		classifier_naive.reset_stats();
//		fs << "sample percentage " << p1 << "\n";
//
//		std::vector<int> sample_perc = { p1, p1, p1, p1, p1 };
//		classifier_naive.classify(sample_perc);
//		classifier_naive.print_stats(fs);
//	}

	fs.close();

//	std::vector<object*> tests;
//	parser(kellog_test_corpus, tests);

//	freopen("dump_bayes", "w", stdout);
//	naive_bayes classifier_naive(classes);
//	std::vector<int> sample_perc = {0,0,0,0,0};
//	classifier_naive.train(sample_perc);
//	classifier_naive.classify(tests);

//	std::cout << tests.size() << "\n";
//	std::cout << tests[0]->features.size() << "\n";
//
//	for_each( i, tests ){
//		std::cout << tests[i]->features.size() << "\n";
//	}

//	freopen("dump_knn_3.results", "w", stdout);
//	knn_classifier knn(classes, 3);
//	knn.classify(tests);

}

void print_usage(){
        fprintf(stderr,"USAGE: ./filter spam || ./filter kellog\n");
}

int main( int argc, char* argv[] )
{
        if( argc < 2 ){
                print_usage();
                return 1;
        }

        if( !strcmp(argv[1], "spam") ){
                spam_filter();
        }
        else if( !strcmp(argv[1], "kellog") ){
                sent_filter();
        }
        else {
                fprintf(stderr,"No command!\n");
                print_usage();
                return 1;
        }

        return 0;
}
