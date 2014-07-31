#!/usr/bin/python
# -*- coding:utf-8 -*-

import MySQLdb

def sql_connect(db_name):
    db = MySQLdb.connect(host="localhost", # your host, usually localhost
                         user="root", # your username
                         passwd="root", # your password
                         db=db_name, # name of the data base
                         charset='utf8')
    return db.cursor()

def sql_query(cursor, query):
        cursor.execute(query)
        return cursor.fetchall()
