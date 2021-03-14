limited: $(TARGETLIM)
	clang++  -I./src -Wall -Werror -g -D DISPLAY_MAX_T=1000  -c -o src/main.o src/main.cpp
	clang++  -I./src -Wall -Werror -g -D DISPLAY_MAX_T=1000  -c -o src/Process.o src/Process.cpp
	clang++  -I./src -Wall -Werror -g -D DISPLAY_MAX_T=1000  -c -o src/RoundRobin.o src/RoundRobin.cpp
	clang++  -I./src -Wall -Werror -g -D DISPLAY_MAX_T=1000  -c -o src/SeqGenerator.o src/SeqGenerator.cpp
	clang++  ./src/main.o ./src/Process.o ./src/RoundRobin.o ./src/SeqGenerator.o -o limited.out

full: $(TARGETFULL)
	clang++  -I./src -Wall -Werror -g -c -o src/main.o src/main.cpp
	clang++  -I./src -Wall -Werror -g -c -o src/Process.o src/Process.cpp
	clang++  -I./src -Wall -Werror -g -c -o src/RoundRobin.o src/RoundRobin.cpp
	clang++  -I./src -Wall -Werror -g -c -o src/SeqGenerator.o src/SeqGenerator.cpp
	clang++  ./src/main.o ./src/Process.o ./src/RoundRobin.o ./src/SeqGenerator.o -o full.out
		
clean:
	rm -f  ./src/main.o ./src/Process.o ./src/RoundRobin.o ./src/SeqGenerator.o ./src/main.d ./src/Process.d ./src/RoundRobin.d ./src/SeqGenerator.d