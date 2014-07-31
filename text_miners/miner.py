#!/usr/bin/python
# -*- coding:utf-8 -*-

import pprint
from pprint import *
import re

import nltk
from nltk.tokenize import RegexpTokenizer
from nltk.collocations import *

dia_table1 = { ord(u'ş') : u's',
               ord(u'ș') : u's',
               ord(u'ţ') : u't',
               ord(u'ț') : u't',
               ord(u'î') : u'i',
               ord(u'â') : u'a',
               ord(u'ă') : u'a',
               ord(u'\n') : u' ',
               ord(u'\t') : u' ' }

dia_table2 = { u'\xc4\x83' : u'a',
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
    
    
dia_table3 = { u"\xe2" : u"a",
               u"\xe1" : u"a",
               u"\xe9" : u"e",
               u"\xed" : u"i",
               u"\xee" : u"i",
               u"\xf3" : u"o",
               u"\xfa" : u"u",
               u"\u0103" : u"a",
               u"\u0163" : u"t",
               u"\u021b" : u"t",
               u"\u015f" : u"s",
               u"\u0219" : u"s" }
              
import sys

def strip_diacritics_1(tok):
    global dia_table1
    
#    pprint(tok)
#    pprint(ord(tok[len(tok)-1]))
    
    result = unicode(tok).translate(dia_table1)
    return result

def strip_diacritics_2(tok):
    
    for key in dia_table2.keys():
        tok = tok.replace( key, dia_table2[key] )

    return pattern.sub('',tok)
    
def strip_diacritics_3(tok):
    
    global dia_table3
    
    for key in dia_table3:
        tok = tok.replace(key, dia_table3[key])
    
    return tok
    
