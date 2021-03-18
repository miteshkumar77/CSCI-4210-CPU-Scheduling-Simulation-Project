limited: $(TARGETLIM)
	g++  -I./src -Wall -Werror -D DISPLAY_MAX_T=1000  -c -o src/main.o src/main.cpp
	g++  -I./src -Wall -Werror -D DISPLAY_MAX_T=1000  -c -o src/Process.o src/Process.cpp
	g++  -I./src -Wall -Werror -D DISPLAY_MAX_T=1000  -c -o src/RoundRobin.o src/RoundRobin.cpp
	g++  -I./src -Wall -Werror -D DISPLAY_MAX_T=1000  -c -o src/ShortestRemainingTime.o src/ShortestRemainingTime.cpp
	g++  -I./src -Wall -Werror -D DISPLAY_MAX_T=1000  -c -o src/SeqGenerator.o src/SeqGenerator.cpp
	g++  ./src/main.o ./src/Process.o ./src/RoundRobin.o ./src/ShortestRemainingTime.o ./src/SeqGenerator.o -o limited.out

full: $(TARGETFULL)
	g++  -I./src -Wall -Werror -c -o src/main.o src/main.cpp
	g++  -I./src -Wall -Werror -c -o src/Process.o src/Process.cpp
	g++  -I./src -Wall -Werror -c -o src/RoundRobin.o src/RoundRobin.cpp
	g++  -I./src -Wall -Werror -c -o src/ShortestRemainingTime.o src/ShortestRemainingTime.cpp
	g++  -I./src -Wall -Werror -c -o src/SeqGenerator.o src/SeqGenerator.cpp
	g++  ./src/main.o ./src/Process.o ./src/RoundRobin.o ./src/ShortestRemainingTime.o ./src/SeqGenerator.o -o full.out


debug: $(TARGETDEBUG)
	g++  -I./src -Wall -Werror -g -c -o src/main.o src/main.cpp
	g++  -I./src -Wall -Werror -g -c -o src/Process.o src/Process.cpp
	g++  -I./src -Wall -Werror -g -c -o src/RoundRobin.o src/RoundRobin.cpp
	g++  -I./src -Wall -Werror -g -c -o src/ShortestRemainingTime.o src/ShortestRemainingTime.cpp
	g++  -I./src -Wall -Werror -g -c -o src/SeqGenerator.o src/SeqGenerator.cpp
	g++  ./src/main.o ./src/Process.o ./src/RoundRobin.o ./src/ShortestRemainingTime.o ./src/SeqGenerator.o -o debug.out
		
clean:
	rm -f  ./src/main.o ./src/Process.o ./src/RoundRobin.o ./src/SeqGenerator.o \
	./src/main.d ./src/Process.d ./src/RoundRobin.d ./src/SeqGenerator.d ./src/ShortestRemainingTime.d \
	./src/ShortestRemainingTime.o
