#!/usr/bin/python
# -*- coding:utf-8 -*-

import json
from os import walk
from pprint import *

import codecs
import re
import sys
from nltk.tokenize import RegexpTokenizer
import datetime
import time
from threading import Thread
import threading
from time import sleep

import nltk
from nltk.collocations import *
import operator
import simplejson

import hashlib
from sets import Set
    
import dateutil.parser as dateparser

import signal

'''
def signal_handler(signal, frame):
        print('You pressed Ctrl+C!')
        sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)
'''

def update_db(key,db):
    if key in db:
        db[key] += 1
    else:
        db[key] = 1
        
from pymongo import MongoClient
mongo_client = MongoClient();
db = mongo_client.thesis

def retrieve_collection(collection):
    global db
    return db[collection].find()

def dump_blob(collection, blob):
    global db
    db[collection].update({"_id":blob["_id"]}, blob, upsert=True)

def get_blob_by_id(collection, _id_):
    global db
    return db[collection].find_one({"_id" : _id_})

def get_blob_by_key(collection, key, value):
    global db
    return db[collection].find_one({key : value})

def update_blob(collection,blob):
    global db
    return db[collection].update( {"_id" : blob["_id"]}, 
                                  {"$set" : blob }, 
                                  upsert=True)

def update_array(collection, _id, field, values):
    global db
    return db[collection].update( {"_id" : _id}, 
                                  {"$addToSet" : { field : { "$each" : values } } } )

def incr_field(collection, _id, field, amount):
    global db    
    return db[collection].update( {"_id" : _id},
                                  {"$inc" : { field : amount } },
                                  upsert = True )

def update_set(collection, _id, field, old_array, new_array):
    
    global db
    
    array = {}
    for item in old_array:
        array[ item["_id"] ] = item["count"]
    
    for user in new_array:
        update_db(user,array)
        
    conv = [ { k : v } for k,v in array.items() ]
    update_array(collection, user_id, field, array)

###### IO handler classes #######

def dump_object(obj, filename):
    with codecs.open(filename, 'w', 'utf-8') as stream:
        simplejson.dump(obj, stream)

def load_object(filename):
    with codecs.open(filename, 'r', 'utf-8') as stream:
        obj = simplejson.load(stream)
    return obj

#################################

from nltk.stem.snowball import SnowballStemmer
stemmer = SnowballStemmer("romanian")

pattern = re.compile('[^A-Za-z.!?;:\-]+')
diacritics_table = { ord(u'ş') : u's',
                     ord(u'ș') : u's',
                     ord(u'ţ') : u't',
                     ord(u'ț') : u't',
                     ord(u'î') : u'i',
                     ord(u'â') : u'a',
                     ord(u'ă') : u'a',
                     ord(u'\n') : u' ',
                     ord(u'\t') : u' ' }

antena_diacritics = { u'\xc4\x83' : u'a',
                      u'\xc4\x82' : u'a',
                      u'\xc3\x82' : u'a', 
                      u'\xc3\xae' : u'i',
                      u'\xc3\x8e' : u'i',
                      u'\xc3\xa2' : u'a',
                      u'\xc5\xa3' : u't',
                      u'\xc8\x9b' : u't',
                      u'\xc5\x9f' : u's',
                      u'\xc8\x99' : u's',
                      u'\xe2\x80\x9d' : u'\"' }


def clean_token(tok):
    global pattern
    
    for key in antena_diacritics.keys():
        tok = tok.replace( key, antena_diacritics[key] )

    return pattern.sub('',tok).lower()
    
def tokenize_clean_string(string):
    global pattern, diacritics_table
    #tokens = [ clean_token(token) for token in string.split() ]
    #return tokens
    
    
    #contents.decode('utf-16').encode('utf-8')
#    pprint( blob["Title"].split()[3].replace(u'\xc4\x83', 'a') )
#    pprint( unicodedata.normalize('NFKD', unicode(blob["Title"])))#.encode('ascii', 'ignore') 
    #unicode(blob["Title"]).decode('utf-16').encode('utf-8'))
    #pprint(result["title"])
    
    
    result = pattern.sub(' ', unicode(string.lower())
                                .translate(diacritics_table))
    return result.split()
    
def tokenize_clean_simple(string):
    global diacritics_table
    return unicode(string.lower()).translate(diacritics_table)
    
def monthToNum(month):
    return{ 'ian' : 1, 'Jan' : 1, 'Ian' : 1,
            'feb' : 2, 'Feb' : 2,
            'mar' : 3, 'Mar' : 3,
            'apr' : 4, 'Apr' : 4,
            'mai' : 5, 'Mai' : 5, 'May' : 5,
            'iun' : 6, 'Iun' : 6, 'Jun' : 6,
            'iul' : 7, 'Iul' : 7, 'Jul' : 7,
            'aug' : 8, 'Aug' : 8,
            'sep' : 9, 'Sep' : 9, 
            'oct' : 10, 'Oct' : 10,
            'nov' : 11, 'noi' : 11, 'Nov' : 11, 'Noi' : 11,
            'dec' : 12, 'Dec' : 12
    }[month]

antena_date = re.compile(r"""
    VideoNews.ro
    (          # Match and capture the following:
     (?:       # Start of non-capturing group, used to match a single character
      (?!      # only if it's impossible to match the following:
       \{      # - a literal {
       (?:     # Inner non-capturing group, used for the following alternation:
        VideoNews.ro  # - Either match the word START
       |       # or
        Comenteaza    # - the word END
       )       # End of inner non-capturing group
       \}      # - a literal }
      )        # End of negative lookahead assertion
      .        # Match any single character
     )*        # Repeat as often as possible
    )          # End of capturing group 1
    Comenteaza    # until {END} is matched.""", 
    re.VERBOSE)

####### Facebook ################

import facebook
token = db["fb"].find_one({"_id":1})["token"]
graph = facebook.GraphAPI(token)


#page_id = graph.fql({"getid":"select comments_fbid from link_stat where url = 'http://www.gandul.info/reportaj/lista-tortionarilor-care-traiesc-acest-batranel-cumsecade-este-alexandru-visinescu-unul-dintre-cei-mai-cumpliti-calai-ai-romaniei-11173039'"})
#graph.get_object( str(comm_id) + "/likes?&limit=100")

# 1395073737382244/comments?limit=200&after=ODg=

def scan_likes(entity_id, after=None):
    
    global graph
    
    query = "/likes?limit=200"
    if after != None:
        query += "after=" + after
        
    #pprint(str(entity_id) + query)
    obj = graph.get_object( str(entity_id) + query )
    
    likes = [ item["id"] for item in obj["data"] ]
    if "paging" in obj and "next" in obj["paging"]:
        likes += scan_likes(entity_id, obj["paging"]["after"])
    
    return likes

def scan_comments(entity_id, page_id, users, after=None):
    
    global graph
    
    query = "/comments?limit=200"    
    if after != None:
        query += "after=" + after
        
    pprint(str(entity_id) + query)
    obj = graph.get_object(str(entity_id) + query)
    posts = []
    
    for post in obj['data']:
        
        #pprint(post)
        
        if "from" in post:
            user_id = post['from']['id']
        else:
            user_id = None
            
        post_id = post['id']
        content = post['message']
        
        date = dateparser.parse( post["created_time"] )
        
        if user_id is not None and user_id not in users:
            user_blob = get_blob_by_id("fb_users", user_id)
            
            if user_blob is None:            
                user_blob = { "_id" : user_id, 
                              "name" : post["from"]["name"]
                              }
                              
                dump_blob("fb_users", user_blob)
        
    
        users_clone = set(users)
        
        #pprint("front")
        replies = scan_comments(post_id, page_id, users_clone)
        #pprint("back")
        
        likes = scan_likes(post_id)
        
        ################## Update post ########################
        
        
        post_blob = {
        "_id"     : post_id,
        "author"  : user_id,
        "date"    : date,
        "content" : content, 
        "likes"   : likes,
        "replies" : replies
        }
        dump_blob("fb_posts", post_blob)
        #pprint("dump blob fb_posts " + str(post_blob))

        posts.append(post_id)        
        ################# Update user field ###################
        
        users |= users_clone
        users |= set(likes)
        
        if user_id is not None:
            users.add(user_id)  
        
            #pprint("users " + str(users) + "| clone " + str(users_clone))
            #pprint("likes " + str(likes))
            
            update_array("fb_users", user_id, "pages", [ page_id ])
        
            for user in list(users):
                if user != user_id:
                    incr_field("fb_conv", user_id, user, 1)
            
            for user in likes:
                if user != user_id:
                    incr_field("fb_likes", user_id, user, 1)
        
         #######################################################
         
    if "paging" in obj and "next" in obj["paging"]:
        pprint("should query " + obj["paging"]["after"])
        posts += scan_comments( entity_id, page_id, reply_to, users, after=obj["paging"]["after"] )

    return posts

def fb_scanner(url):
    
    time.sleep(6)
    
    global graph
    url = ("\'" + url + "\'").encode('ascii','ignore')

    page_id = graph.fql({"getid":"select comments_fbid from link_stat where url = " + url})[0]['fql_result_set'][0]["comments_fbid"]
    
    if page_id == None:
        return (None, None)
    
    users = set()
    return (page_id, scan_comments(page_id, page_id, users))
    
    
def fb_append_author():
    global db
    post_cursor = db.fb_posts.find( { "author" : { "$exists" : False } } )
    
    i = 0
    for post in post_cursor:
        
        pprint(post["_id"])
        
        try:
            obj = graph.get_object(post["_id"] + "/?field=from")
        
            author = None
            if "from" in obj:
                author = obj["from"]["id"]
        
        except facebook.GraphAPIError:
            author = None
            pprint("!!")
            
        
                    
        db.fb_posts.update( { "_id" : post["_id"]}, 
                            { "$set" : {"author" : author} } )
                            
        i += 1
        if i == 10:
            pprint("Z!");
            i = 0;
            time.sleep(6);
            
##################################

def chunkify(lst,n):
    return [ lst[i::n] for i in xrange(n) ]

def th_map(items, _map_, no_threads = 1):
    
    ranges  = chunkify(items, no_threads)
    threads = [ Thread(target = _map_, args = (_range_,) ) 
                for _range_ in ranges ];
    
    for th in threads:
        th.start()
    
    for th in threads:
        th.join()
        
##################################

def get_parser(host):
    
    orig_host = host
    host = host[-4:]
    
    if host not in parser_db:
        pprint("unrecognized host " + orig_host)
        return None
        
    return parser_db[host]


import unicodedata

def parse_content(blob):
    result = {}
    
    result["title"] = tokenize_clean_string(blob["Title"])
    result["text"] = tokenize_clean_string(blob["Text"])

    return result

def parse_date_gandul(date):
    
    pos1 = date.find("RSS") + 3
    pos2 = date[pos1:].find("Publicitate")
    [day, month, year, hour, minute] = re.split(r"[ :]", date[pos1:(pos1+pos2)])[1:-1]
    month = monthToNum(month)
    return datetime.datetime(int(year), month, int(day), int(hour), int(minute))

def parse_date_antena(date):
    
    date = antena_date.search(date).group(0)
    [day,month,year,_,hour,minute,_] = re.split(r"[ :]",date)[-7:]
    month = monthToNum(month)    
    return datetime.datetime(int(year), month, int(day), int(hour), int(minute))

def parse_date(date):
    [day1,month,day,hour,minute,seconds,_,year] = re.split(r"[ :]",date)
    month = monthToNum(month)
    return datetime.datetime(int(year), month, int(day), int(hour), int(minute), int(seconds))
    
article_count = db["article"].count()

def api_limit_update():
    global damn_api_limit
    damn_api_limit -= 1
            
def gandul_parser(blob):
    
    return

    api_limit_update()    
    print "gandul parser..."
    (_id,posts) = fb_scanner(blob["url"])
    
    result = parse_content(blob)
    result["date"]  = parse_date_gandul( blob["content"] )
    result["host"]  = blob["host"]
    result["_id"]   = _id
    result["posts"] = posts
    
    dump_blob("article", result)
    

def mediafax_parser(blob):
    
    result = parse_content(blob)
    result["date"]  = parse_date(blob["DateTime_dt"])
    result["host"]  = blob["host"]
    result["_id"]   = blob["_id"]
    result["posts"] = []
    
    dump_blob("article", result)

def realitatea_parser(blob):
    
    result = parse_content(blob)
    result["date"]  = parse_date(blob["DateTime_dt"])
    result["host"]  = blob["host"]
    result["_id"]   = blob["_id"]
    result["posts"] = []
    
#    pprint(result["host"])
    dump_blob("article", result)
    
    pass

var = 0

def antena_parser(blob):

    return
    
    print "antena parser...."

    #(_id,posts) = fb_scanner(blob["url"])
    
    result = parse_content(blob)
    result["date"]  = parse_date_antena( blob["content"] )
    result["host"]  = blob["host"]
    result["_id"]   = blob["_id"]
    result["posts"] = []
    
    dump_blob("article", result)

def zf_parser(blob):
    
    result = parse_content(blob)
    result["date"]  = parse_date(blob["DateTime_dt"])
    result["host"]  = blob["host"]
    result["_id"]   = blob["_id"]
    result["posts"] = []
    
#    pprint(result["host"])
    dump_blob("article", result)
    
    pass

def hotnews_parser(blob):
    
    result = parse_content(blob)
    result["date"]  = parse_date(blob["DateTime_dt"])
    result["host"]  = blob["host"]
    result["_id"]   = blob["_id"]
    result["posts"] = []
    
#    pprint(result["host"])
    dump_blob("article", result)
    
    pass

parser_db = { "info"  : gandul_parser,
              "x.ro"  : mediafax_parser,
              ".net"  : realitatea_parser,
              "3.ro"  : antena_parser,
              "f.ro"  : zf_parser,
              "s.ro"  : hotnews_parser
        };



def parse_articles(files):
    
    path = "./news"

    for _file_ in files:        
        
        article_id = _file_[0]
        filename   = _file_[1]
        
        filepath = path + "/" + filename
        blob = load_object(filepath)
            
        parser = get_parser(blob["host"])
        
        if parser is not None:
            
            blob["_id"] = article_id
#            url_map = { "_id" : article_id, "url" : blob["Source"] } 
#            dump_blob("url_map", url_map)
            parser(blob)

lock = threading.Lock()

def parse_comment(files):
    
    global article_count
    counter = 0
     
    path = "./news_comments"
    for _file_ in files:        
            
        counter += 1
        if counter % 100 == 0:
            pprint(counter)
            
        if counter < 34100:
            continue
            
        comm_id  = _file_[0]
        filename = _file_[1]
        
        filepath = path + "/" + filename
        blob = load_object(filepath)
            
        url_map = get_blob_by_key("url_map", "url", blob["ReplyTo"])
        
        if url_map == None:

            with lock:
                article_count += 1
                local_count = article_count
            
            article = { "_id" : local_count,
                        "url" : blob["ReplyTo"],
                        "posts" : [] 
                        }
            
            url_map = { "_id" : local_count,
                        "url" : blob["ReplyTo"] }
            
            pprint(local_count)
            dump_blob("article",article)
            dump_blob("url_map",url_map)
            
        else:
            local_count = url_map["_id"]
        
        result = {
        "_id"     : comm_id,
        "author"  : blob["Author"],
        "date"    : parse_date(blob["DateTime_dt"]),
        "title"   : tokenize_clean_simple(blob["Title"]),
        "content" : tokenize_clean_simple(blob["Text"]),
        "article" : local_count
        }
        
        
        article = get_blob_by_id("article", local_count)
        
        if article == None:
            
            global pattern
 
            title = pattern.sub('/',blob["ReplyTo"].replace(".html",""))

            #pprint(title)

            bad = ["-n-avem-", "-dintr-o-", "-bucuresti-alexandria-", "-s-a-", "-bacau-brasov-", 
                   "-iasi-targu-", "s-au", "-administrativ-teritoriala-", "-petrecut-o-", "-l-a-", 
                   "-nu-l-", "-d-lui-", "-site-ul-", "-futu-ti-", "-ma-tii-", "-sa-ti-", "-m-ar-",
                   "-ce-a-", "-le-am-", "-sa-l-", "-parlamentari-butoane-", "-si-a-",
                   "-nu-i-", "-intr-o-", "-usd-udmr-", "-multi-modal-", "-i-a-", "-mi-a-",
                   "democrat-liberalii-", "-n-v-", "-anti-imigratie-", "-jong-un-", "-nord-coreean-",
                   "-crestin-ortodox-", "-intr-un-", "-anti-romani-", "-l-ati-", "-doua-trei-", 
                   "-n-o-", "-de-a-", "-anti-rosia-", "-smartphone-uri-", "-n-a-", "-ia-ti-", "ia-ti-",
                   "-lege-paravan-", "-ce-si-", "-copy-paste-", "-sa-i-", "-zi-i-",
                   "-jacuzzi-ul-", "-lobby-ului-", "-nu-mi-", "-song-thaek-", "-ne-au-", "-mi-am-",
                   "manca-ti-ar-", "-primit-o-", "-l-au-", "-rusia-ucraina-", "-psd-pc-",
                   "-ong-uri-", "-a-i-", "-nu-si-", "-sebes-turda-", "-sa-si-", "-facut-o-", "-draghia-akli-",
                   "-smartphone-ul-", "-mi-am-", "-mosi-stramosi-", "-miting-alergator-",
                   "sparge-i-", "-steaua-chelsea-", "-ponta-n-", "-sa-l-",
                   "-l-a-", "-spunandu-i-", "-maternitatea-fantoma-", "-l-a-", "-mi-a-",
                   "-mihai-razvan-", "-chitoiu-ghetea-", "-urmariti-mi-",
                   "-kerry-corlatean-", "al-qaida-",
                   "wikileaks-", "peter-", "-l-am-", "-le-a-", "-jong-il-",
                   "-nord-coreeni-", "-i-au-", "-intr-o-", "-si-au-", "-sa-mi-",
                   "-intrebati-l-",
                   "exclusiv-", "plus-", "-implementat-o-",
                   "-basescu-superstar-", "-le-am-", "-care-i-",
                   "-stick-ul-", "-basescu-ponta", "mercedes-benz", "-sud-african-",
                   "-batut-o-",
                   "-antonescu-voiculescu-", 
                   "-steaua-astra",
                   "-te-ai-",
                   "-salaj-alexandria-",
                   "-ra-apps-", "-ce-ti-"
                   "-struto-camila-", "-democrat-liberala", "-n-au-",
                   "-e-urilor","-a-si-", "-i-am-", "-i-au-", "-i-a-",
                   "-medico-legal-", "-printr-un-", "-democrat-liberal-",
                   "-s-o-", "-l-a-", "-l-am-", "-l-as-",
                   "-ne-ar-", "-redactor-sef-",
                   "-bentley-ul-", "-dintr-un-", "-pac-urilor-",
                   "-facebook-ul-", "smartphone-ul-", "-ce-i-",
                   "-nu-l-", "-l-a-", "-s-ar-", "-n-ar-",
                   "-prim-ministru-", "mall-mania-", "-ridicati-va-",
                   "l-demite-", "-nord-coreean-",
                   "-contra-revolutionare-",
                   "sedinta-fulger-",
                   "-like-uri-",
                   "day-to-day-",
                   "-site-urile-", "-pdl-basescu-",
                   "-acuzandu-l-", "-tv-urile-",
                   "-cnadnr-ul", "-tv-uri-",
                   "-mass-media-", "-bruxelles-ul-",
                   "-nord-aeroport", "-tir-uri-",
                   "-euro-", "-si-au-", "-udmr-ul"
                    ]
            
            for key in bad:
                title = title.replace(key,"-")

            #pprint(title)
            
            title = title.split("/")
            
            start = 3
            if len(blob["ReplyTo"].split("/")) == 4:
                start = 2
            
            title = "-".join(title[start:len(title)-1]).replace("-"," ").split()
            title = title[0:len(title)]
            
            title.append("gandul")
            
#            if "dintr" in title: 
#                index = title.index("dintr")
#                del title[index]
#                del title[index]
            
            #pprint(title)
            
            global db
            
            article = db["article"].find_one( { "title": { "$all": title } } );
            count = db["article"].find( { "title": { "$all": title } } ).count();
            
            i = 0
            while count != 1 and i < len(title)-1:
                
                new_title = title[1:i-1] + title[i+1:len(title)]
                count = db["article"].find( { "title": { "$all": new_title } } ).count();
                article = db["article"].find_one( { "title": { "$all": title } } );
                #pprint(new_title)
                
                i += 1    
            
            if count != 1 or article is None:
                
                #i = 0
                #while i < len(title)-1:
                #    new_title = title[1:i-1] + title[i+1:len(title)]
                #    pprint(new_title)
                #    i += 1
                
                print "bad, mkeay.."
                print local_count
                print comm_id
                print count
                print title
                print blob["ReplyTo"]
                print (article is None)
                
                if count == 0:
                    
                    article = { "_id" : local_count,
                                "url" : blob["ReplyTo"],
                                "posts" : [] 
                                }
                    
                    url_map = { "_id" : local_count,
                                "url" : blob["ReplyTo"] }
                    
                    pprint(str(local_count) + " dummpy" )
                    dump_blob("article",article)
                    dump_blob("url_map",url_map)

                else:
                    tokens = [ "\"" + tok + "\"" for tok in title ]
                    pprint("doc=db.article.findOne({\"title\" : {$all: [" + ",".join(tokens) + "]}})")
                    sys.exit(0)
            
            article["fb_id"] = article["_id"]
            article["_id"]   = local_count
            article["posts"].append(comm_id)
            article["url"]   =  blob["ReplyTo"]

            db["article"].remove({"_id":article["fb_id"]})
            dump_blob("article", article)
            
        else:
            update_array("article", local_count, "posts", [comm_id] )

#        pprint("success")
            
            #pprint(article)            
            #sys.exit(0)
            
            
        
        
 #       update_array("article", local_count, "posts", [comm_id] )
        dump_blob("comment", result)
        

def parse_news():
    
    print "parse_news..."
    
    path = "./news"
    (_,_,files) = walk(path).next()
    
    files = [ (i,files[i]) for i in xrange(len(files)) ]
    th_map(files, parse_articles)

def parse_comments():
    
    print "parse comments..."
    
    path = "./news_comments"
    (_,_,files) = walk(path).next()
    files = [ (i,files[i]) for i in xrange(len(files)) ]
    th_map(files, parse_comment)
    
    
    
    
def scan_comments():
    
    comments = retrieve_collection("comment")
    pprint(comments[0])
    
    
def main():
    scan_comments()
    
    #parse_comments()    
    #fb_append_author()
    #pprint(db.fb_posts.find( { "author" : { "$exists" : False } } ).count())

if __name__ == "__main__":
    main()
