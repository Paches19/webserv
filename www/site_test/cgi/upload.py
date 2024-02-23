#!/usr/bin/python3
import cgi, os
import cgitb; cgitb.enable()
form = cgi.FieldStorage()
fileitem = form['file']
directory = os.environ["UPLOAD_DIR"]
if fileitem.filename:
   fn = os.path.basename(fileitem.filename)
   open(directory + '/' + fn, 'wb').write(fileitem.file.read())
   message = 'The file "' + fn + '" was uploaded successfully'
else:
   message = 'No file was uploaded'
print ("""\
<html>
<body>
   <p>%s</p>
</body>
</html>
""" % (message,))