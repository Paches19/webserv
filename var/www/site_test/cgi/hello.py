#!/usr/bin/python3

import cgi, cgitb 
form = cgi.FieldStorage() 

first_name = form.getvalue('fname')
last_name  = form.getvalue('lname')


if first_name is None:
    first_name = "you haven't wriiten text"
if last_name is None:
    last_name = "you haven't wriiten text"
print ("<html><body><p>your name is : %s, %s</p></body></html>"%(first_name, last_name));
