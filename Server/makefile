target:
	gcc -std=c++17 ../networkmanager.cpp server.cpp -o Server -lstdc++

run:
	make
	./Server 127.0.0.2 127.0.0.1 7801

clean:
	rm *.o
	rm Server

checks:
	gcc -std=c++17 -fsanitize=address ../networkmanager.cpp server.cpp -o Server -lstdc++