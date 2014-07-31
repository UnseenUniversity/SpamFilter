#!/usr/bin/python
# -*- coding:utf-8 -*-

from pprint import *
from pymongo import MongoClient

mongo_client = MongoClient();
db = mongo_client.thesis

def retrieve_collection(collection):
    global db
    return db[collection].find()

def dump_blob(collection, blob):
    global db
    db[collection].update({"_id":blob["_id"]}, blob, upsert=True)

def dump_blob_db(db, collection, blob):
    db[collection].update({"_id":blob["_id"]}, blob, upsert=True)

def get_blob_by_id(collection, _id_):
    global db
    return db[collection].find_one({"_id" : _id_})

def get_blob_by_id_db(db, collection, _id_):
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

def remove_from_array(collection, _id, field, values):
    global db
    return db[collection].update( {"_id" : _id}, 
                                  {"$pullAll" : { field : values } } )

def incr_field(collection, _id, field, amount):
    global db    
    return db[collection].update( {"_id" : _id},
                                  {"$inc" : { field : amount } },
                                  upsert = True )
                                  
def incr_field_db(db, collection, _id, field, amount):
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
    
