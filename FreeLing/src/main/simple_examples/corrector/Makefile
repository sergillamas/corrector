
# Makefile to build the simple examples of FreeLing usage.
# If your installation paths are not /usr/local, call make with:
#
# make FREELINGDIR=/my/freeling/dir BOOSTINCLUDEDIR=/my/boost/include BOOSTLIBDIR=/my/boost/lib

# directories where freeling and boost are installed
FREELINGDIR=/usr/local
BOOSTINCLUDEDIR=/usr/local/include
BOOSTLIBDIR=/usr/local/lib

# paths for compilation commands below
PATHS=-L$(FREELINGDIR)/lib -I$(FREELINGDIR)/include -I$(FREELINGDIR)/include/foma -L$(BOOSTLIBDIR) -I$(BOOSTINCLUDEDIR)
LIBS=-lfreeling -lboost_system

all:	corrector word_vec convert_model evaluate tweet_norm

corrector: corrector.cc
	g++ -std=c++0x -o corrector corrector.cc $(PATHS) $(LIBS)

word_vec: word_vec.cc
	g++ -std=c++0x -o word_vec word_vec.cc $(PATHS) $(LIBS)

convert_model: convert_model.cc
	g++ -std=c++0x -o convert_model convert_model.cc $(PATHS) $(LIBS)

evaluate: evaluate.cc
	g++ -std=c++0x -o evaluate evaluate.cc $(PATHS) $(LIBS)

tweet_norm: tweet_norm.cc
	g++ -std=c++0x -o tweet_norm tweet_norm.cc $(PATHS) $(LIBS)

clean:
	rm -f corrector word_vec convert_model evaluate tweet_norm
