# Script to return the index page displaying the list of files in the directory

import os

# Read arguments to find the directory
directory = os.environ.get("SCRIPT_FILENAME")

# If the provided directory is a file, get the directory name
if os.path.isfile(directory):
	directory = os.path.dirname(directory)

print("Status: 200 OK")

print("Content-Type: text/html")

print("\r\n\r\n")

print("<html><head><title>Index</title></head><body>")
print("<h1>Index</h1>")
print("<ul>")

# List all the files in the directory
for file in os.listdir(directory):
	print(f'<li><a href="{file}">{file}</a></li>')

print("</ul>")
print("</body></html>")
