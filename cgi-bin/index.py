# Script to return the index page displaying the list of files in the directory

import os

# Read arguments to find the directory



print("Status: 200 OK")

print("Content-Type: text/html")

print("<html><head><title>Index</title></head><body>")
print("<h1>Index</h1>")
print("<ul>")
for file in os.listdir():
	print(f'<li><a href="{file}">{file}</a></li>')
print("</ul>")
print("</body></html>")
