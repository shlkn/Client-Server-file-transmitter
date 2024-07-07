target:
	gcc -std=c++17 ../networkmanager.cpp filereader.cpp transferfilemanager.cpp client.cpp -o Client -lstdc++

run:
	make
	./Client somefile.bin 127.0.0.1 127.0.0.2 7800 7801

run2:
	make
	./Client file.txt 127.0.0.1 127.0.0.2 7799 7801

clean:
	rm *.o
	rm Client

checks:
	gcc -std=c++17 -fsanitize=address ../networkmanager.cpp filereader.cpp transferfilemanager.cpp client.cpp -o Client -lstdc++