build: 
	g++ -std=c++0x clk.cpp -o clock.out
	g++ -std=c++0x schedulerSRTF.cpp -o schSRTF.out
	g++ -std=c++0x schedulerRR.cpp -o schRR.out
	g++ -std=c++0x FilesGenerator.cpp -o Files.out
	g++ -std=c++0x processGenerator.cpp -o main.out
	g++ -std=c++0x process.cpp -o process.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./main.out
