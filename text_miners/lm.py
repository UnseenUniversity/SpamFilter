#!/usr/bin/python
# -*- coding:utf-8 -*-

from pprint import *

import codecs, re, sys

from threading import Thread

import hashlib
from sets import Set
    
import dateutil.parser as dateparser

from mongo_orm import *
from sql_orm import *
from miner import *

from thread_manager import *

import numpy as np

def levenshtein(source, target):
    
    if len(source) < len(target):
        return levenshtein(target, source)

    # So now we have len(source) >= len(target).
    if len(target) == 0:
        return len(source)

    # We call tuple() to force strings to be used as sequences
    # ('c', 'a', 't', 's') - numpy uses them as values by default.
    source = np.array(tuple(source))
    target = np.array(tuple(target))

    # We use a dynamic programming algorithm, but with the
    # added optimization that we only need the last two rows
    # of the matrix.
    previous_row = np.arange(target.size + 1)
    for s in source:
        # Insertion (target grows longer than source):
        current_row = previous_row + 1

        # Substitution or matching:
        # Target and source items are aligned, and either
        # are different (cost of 1), or are the same (cost of 0).
        current_row[1:] = np.minimum(
                current_row[1:],
                np.add(previous_row[:-1], target != s))

        # Deletion (target grows shorter than source):
        current_row[1:] = np.minimum(
                current_row[1:],
                current_row[0:-1] + 1)

        previous_row = current_row
    return previous_row[-1]
       
w_cache = {}

def dex_stemmer(sql_client, w):
    
    global w_cache
    
    if w in w_cache:
        return w_cache[w]

    select_q = "select a.formUtf8General "
    from_q   = "from  Lexem a, InflectedForm b, Inflection c, LexemModel d, LexemDefinitionMap e, Definition r "
    where_q  = "where b.formUtf8General = '" + w + "' and r.sourceId = 1 and a.id = d.lexemId and a.id = e.lexemId and r.id = e.definitionId and d.id = b.lexemModelId and c.id = b.inflectionId;"

    query = select_q + from_q + where_q
    
    ans = sql_query(sql_client,query)

    if len(ans) == 0:
        w_cache[w] = w
        return w

    words = [item[0] for item in ans]
    
    stem = min(words, key = lambda x : levenshtein(x, w))
    w_cache[w] = strip_diacritics_3(stem)
    
    return w_cache[w]

def compute_freq_tokens(sql_client, db, tokens):
    
    tokens = [ dex_stemmer(sql_client, w) for w in tokens if w != "-" ]
    
    for w in tokens:
        incr_field_db(db, 'word_stats', w, 'freq', 1)
    
    bigrams  = [ tokens[i:i+2] for i in xrange(len(tokens)-1) ]
    for grams in bigrams:
        
        _id = " ".join(grams)
        first  = grams[0]
        second = grams[1]
        
        blob = get_blob_by_id_db(db, "bigrams", _id)
        
        if blob is None:
            dump_blob_db(db, "bigrams", { "_id"     : _id,
                                          "unigram" : first,
                                          "suffix"  : second,
                                          "count"   : 1 } )
        else:
            incr_field_db(db, "bigrams", _id, "count", 1 )
    
    del bigrams
    
    trigrams = [ tokens[i:i+3] for i in xrange(len(tokens)-2) ]
    for grams in trigrams:
        
        _id = " ".join(grams)
        first  = grams[0]
        second = grams[1]
        suffix = grams[2]
        
        blob = get_blob_by_id_db(db, "trigrams", _id)
        
        if blob is None:
            dump_blob_db(db, "trigrams", { "_id"     : _id,
                                           "bigram"  : first + " " + second,
                                           "suffix"  : suffix,
                                           "count"   : 1 } )
        else:
            incr_field_db(db, "trigrams", _id, "count", 1 )
    


def compute_freq(articles):
    
    pprint("workload " + str(len(articles)))
    
    mongo_client = MongoClient();
    db = mongo_client.thesis
    
    sql_client = sql_connect("DEX")

    for article in articles:
    
        if "title" not in article:
            continue
    
        title = article['title']
        text  = article['text']
        
        unique_terms = set(title) | set(text)

        for w in unique_terms:
            incr_field_db(db, 'word_stats', dex_stemmer(sql_client, w), 'unique_doc', 1)  

        compute_freq_tokens(sql_client, db, title)        
        compute_freq_tokens(sql_client, db, text)


def build_vocabulary():
    

    articles = retrieve_collection("article")
    vocabulary = {}

    print "load articles..."
    entries = [ article for article in articles ]

    print "do work..."
    th_map(entries, compute_freq, no_threads = 4)




def main():

    build_vocabulary()
    
    
    
    
if __name__ == "__main__":
    main()
    
