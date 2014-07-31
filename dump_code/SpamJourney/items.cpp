/*
 * items.cpp
 *
 *  Created on: Jun 18, 2014
 *      Author: alexei
 */

#include "items.h"

std::istream& operator>>(std::istream& in, item* item){
	in >> item->id;
	return in;
}

std::istream& operator>>(std::istream& in, article& article){

	in >> ((item*) &article);

	int title_tokens = 0;
	int text_tokens = 0;
	int url_tokens = 0;

	in >> title_tokens;
	in >> text_tokens;
	in >> url_tokens;

	std::string token;

	article.title.reserve(title_tokens);
	article.text.reserve(text_tokens);
	article.links.reserve(url_tokens);

	_forf(i, 0, title_tokens){
		in >> token;
		article.title.push_back(token);
	}

	_forf(i, 0, text_tokens){
		in >> token;
		article.text.push_back(token);
	}

	_forf(i, 0, url_tokens){
		in >> token;
		article.links.push_back(token);
	}

	int num_comments;
	in >> num_comments;

	_forf(i, 0, num_comments){
		comment* reply = new comment();
		reply->post_origin = &article;
		article.comments.push_back(reply);
		in >> *reply;
	}

	return in;
}

std::ostream& operator<<(std::ostream& out, article& article)
{
	out << "Article " << article.id << "\n";
	out << "Title "   << article.title << "\n";
	out << "Text "    << article.text << "\n";
	out << "Links "   << article.links << "\n";

	out << "Comments: \n";
	for_each(i, article.comments){
		out <<  *article.comments[i];
	}

	return out;
}

// ======================================================================

std::istream& operator>>(std::istream& in, comment& comment){

	in >> ((item*) &comment);

	int author_tokens = 0;
	int text_tokens = 0;
	int url_tokens = 0;

	in >> author_tokens;
	in >> text_tokens;
	in >> url_tokens;
	std::string token;

	comment.author.reserve(author_tokens);
	comment.text.reserve(text_tokens);
	comment.links.reserve(url_tokens);

	_forf(i, 0, author_tokens){
		in >> token;
		comment.author.push_back(token);
	}

	_forf(i, 0, text_tokens){
		in >> token;
		comment.text.push_back(token);
	}

	_forf(i, 0, url_tokens){
		in >> token;
		comment.links.push_back(token);
	}

	return in;
}

std::ostream& operator<<(std::ostream& out, comment& comment)
{
	out << "Comment " << comment.id << "\n";
	out << "Author "  << comment.author << "\n";
	out << "Text "    << comment.text << "\n";
	out << "Links "   << comment.links << "\n";
	return out;
}
