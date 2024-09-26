#!/usr/bin/python3

import cgi
import cgitb
import sys

cgitb.enable(display=0, logdir="/Users/naomisterk/codam/webbieservie/www/cgi-logs") # Enable exception handler, if error occurs detailed report will be saved to the directory cgi-logs

path = "/Users/naomisterk/codam/webbieservie/www/img/" + sys.argv[1]

with open(path, "rb") as imgFile:
    binaryData = imgFile.read()

length = len(binaryData)

# response = (
# 	"HTTP/1.1 200 OK\r\n"
# 	"Content-Type: image/jpeg\r\n"
# 	"Content-Length: " + str(length) + "\r\n\r\n"
# )

# print(response)
# print(binaryData)

imgFile.close
print("HTTP/1.1 200 OK")
print("Content-Type: image/jpeg")
print("Content-Length: " + str(length) + "\r\n\r\n")
print(binaryData)
