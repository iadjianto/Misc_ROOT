# -*- coding: utf-8 -*-
"""
Created on Thu Aug  4 11:42:02 2022

@author: isaia

Taken from a challenge posed to make a CSV reading function without any packages

line input:" coursera,rocks "
line output: ['START', 'coursera','rocks', 'END']

line input:" 'a',b,'c,d' "
line output: ['START', 'a',b,'c,d', 'END']

line input:" ,education "
line output: ['START', 'education','END']

All inputs may have single quotes, and are separated in order of priority:
    1. Quotes around a single element
    2. Commas in the absence of rules by quotes (such as the element (..., b,...) in second example, which has no quotes)

"""

#in_put = "coursera,rocks"
in_put = "'a',b,'c,d'"
#in_put = ",education"

def line_reader(in_put: tuple):

    line = in_put.split(',') # Initial split with commas, rejoin ones with quotes later
    
    
    start = ["START"]
    end = ["END"]
    
    line = [element for element in line if element] # Toss aside empty elements as in last example
    #line = [element[1:-1] for element in line if element[0] and element [-1] == "\'"]
    
    start_dex = 0
    end_dex = -1
    for i in range(len(line)):# Handles cases where quotes determine the contents of each element
    	if line[i][0] == "\'" and line[i][-1] == "\'":# If an element already has quotes on the start and end
    		line[i] = line[i][1:-1]# element is everything between quotes
    	elif line[i][0] == "\'" and line[i][-1] != "\'":# If an element only starts with a quote
    		line[i] = line[i][1:]# start recording how many elements to join together
    		start_dex = i
    	elif line[i][0] != "\'" and line[i][-1] == "\'":# If an element only ends with quotes
    		line[i] = line[i][:-1]
    		end_dex = i+1
    		line[start_dex:end_dex] = [','.join(line[start_dex:end_dex])]# joins all elements recorded since the open quotes till end quote
    
    
    line =start + line+ end# adds start and end string

    return line

print(line_reader(in_put))