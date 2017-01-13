import json
import sys

# Aux functions
def error(msg):
  print "ERROR: " + msg + "\n"
  sys.exit(0)

# Usage
def Usage():
  print "\n  USAGE:"
  print "  python get_tweets_info.py [file-name]"
  print "    Where filename is tweets.txt by default\n"
  sys.exit(0)

# Get filename (file where tweets info is stored)
if len(sys.argv) < 2:
  print "...Assuming tweets.txt is the input file"
  filename = "tweets.txt"
elif len(sys.argv) == 2:
  if sys.argv[1] == "-h" or sys.argv[1] == "-help" or sys.argv[1] == "--help":
    Usage()
  filename = sys.argv[1]
else:
  Usage()

# Process tweets
tweets = []

try:
  for line in open(filename, 'r'):
    if line.strip() != "": #ignore empty lines
      tweets.append(json.loads(line))
except IOError:
  error("couldn't open \"" + filename + "\" file")
  
# Print results
i = 0
errors = 0
for tweet in tweets:
  if 'text' not in tweet:
    errors += 1
    continue
  print "##### Tweet " + str(i) + ":\n" 
  print tweet["text"].encode('utf-8')
  print ""
  i += 1

print "##### Number of errors: " + str(errors)
print "##### END #####" 