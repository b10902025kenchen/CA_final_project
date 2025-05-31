do:
	g++ -std=c++17 -O3 main.cpp -o main

000:
	./main 

001:
	./main -m 3

010:
	./main -s

011:
	./main -s -m 5

100:
	./main -t

101:
	./main -t -m 3

110:
	./main -t -s

111:
	./main -t -s -m 3

clean:
	rm -f ./main
