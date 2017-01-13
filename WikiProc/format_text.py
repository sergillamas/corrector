#!/usr/bin/env python
# -*- coding: utf-8 -*-

import codecs
import sys
import re

if len(sys.argv) < 2:
  print "   Usage: python format_text.py input_file"
  sys.exit(0)

out_file = codecs.open("out_file.txt", 'wb', "utf-8-sig");

with codecs.open(sys.argv[1], 'rb', "utf-8-sig") as f:
  for line in f:
    if not line.startswith("<doc") and not line.startswith("</doc") and not line == "\n" and not len(line.split()) == 1:
      # lowercase
      line = line.lower()
      
      # remove
      line = re.sub(ur'[^a-zA-Z!¡?¿.,()\'$€&":;\[\]%\-_ çñáéíóúàèìòùäëïöüâêîôû]', '', line)
       
      # separate characters
      chars = u",.:;?\"¿¡!()&%$€\[\]"
      for c in chars:
        if c in line:
          line = line.replace(c, " " + c + " ")
      
      # write to output (remove extra whitespaces)
      out_file.write(' '.join(line.split()) + '\n')
        
out_file.close

