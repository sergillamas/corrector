# -*- coding: utf-8 -*-

from tweepy.streaming import StreamListener
from tweepy import OAuthHandler
from tweepy import Stream
import sys

#credentials to access twitter API 
access_token = "WRITE HERE"
access_token_secret = "WRITE HERE"
consumer_key = "WRITE HERE"
consumer_secret = "WRITE HERE"

#============================================================
# stream listener
#============================================================
class StdOutListener(StreamListener):
    def __init__(self):
      self.processed_tweets  = 0
      self.tweets_to_process = 0
    
    def on_data(self, data):
        print data
        self.processed_tweets += 1
        stop = (self.processed_tweets >= self.tweets_to_process)
        if self.tweets_to_process > 0 and stop:
          return False # close stream
        else:
          return True

    def on_error(self, status):
        print "ERROR: StdOutListener exited with status " + status

#============================================================

# Usage
def Usage():
  print "\n  USAGE:"
  print "  python random_tweets.py [num-tweets]"
  print "    If num-tweets is not provided, the program will run until killed\n"
  sys.exit(0)

most_common_words_es = ["de", "la", "que", "el", "en", "y", "a", "los", "se", "del", "las", "un", "por", "con", "no", "una", "su", "para", "es", "al", "lo", "como", "mas", "o", "pero", "sus", "le", "ha", "me", "si", "sin", "sobre", "este", "ya", "entre", "cuando", "todo", "esta", "ser", "son", "dos", "muy", "desde", "mi"]

#============================================================
# main program
#============================================================

if __name__ == '__main__':

    #connection to twitter api
    listener = StdOutListener()
    auth = OAuthHandler(consumer_key, consumer_secret)
    auth.set_access_token(access_token, access_token_secret)
    stream = Stream(auth, listener)
    
    if len(sys.argv) >= 2:
      if sys.argv[1] == "-h" or sys.argv[1] == "-help" or sys.argv[1] == "--help":
        Usage()
      num_tweets = 0
      if len(sys.argv) == 2:
        num_tweets = int(sys.argv[1])

    # set num tweets to process
    if num_tweets != 0: 
      listener.tweets_to_process = num_tweets

    # stream tweets based on common words for spanish
    stream.filter(languages = ["es"], track = most_common_words_es)
    
    # end of program