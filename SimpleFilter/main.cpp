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
        fs.open ("knn-1-weighted.results", std::fstream::out);

        fs << "ham documents: " << ham.objects.size() << "\n";
        fs << "spam documents: " << spam.objects.size() << "\n";

//        naive_bayes classifier_naive(classes);
////        classifier_naive.print_stats(std::cout);
//        std::cerr << "Ready to rumble!\n";
//        for( int p1 = 10; p1 < 100; p1 += 10 )
//        	for( int p2 = 10; p2 < 100; p2 += 10 ){
//
//        		classifier_naive.reset_stats();
//        		fs << "sample percentage:\n" << p1 << "\t" << p2 << "\n";
//
//        		std::vector<int> sample_perc = { p1, p2 };
//				classifier_naive.classify(sample_perc);
//				classifier_naive.print_stats(fs);
//        	}

        knn_classifier knn(classes, 1);

		for( int p1 = 90; p1 < 100; p1 += 10 )
			for( int p2 = 70; p2 < 100; p2 += 10 ){

				knn.reset_stats();
				fs << "sample percentage:\n" << p1 << "\t" << p2 << "\n";

				std::vector<int> sample_perc = { p1, p2 };
				knn.classify(sample_perc);
				knn.print_stats(std::cout);

				std::cout << "test done! " << p1 << " " << p2 << "\n";
			}

        fs.close();


//        std::vector<int> sample_perc = { 70, 30 };
//        classifier_naive.classify(sample_perc);
//        classifier_naive.print_stats(std::cout);
}

void sent_filter(){

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
