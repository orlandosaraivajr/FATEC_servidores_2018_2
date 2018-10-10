#!/usr/bin/python3
# coding: utf-8

import threading
import random
import os


class ThreadClass(threading.Thread):
    def run(self):
#        os.system("sleep " + str(random.randint(1, 10)))
        print("Oi, sou %s  " % (self.getName()))


for i in range(120000):
    t = ThreadClass()
    t.start()
