#!/usr/bin/python
# -*- coding:utf-8 -*-

import sys
from threading import Thread
import threading


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
