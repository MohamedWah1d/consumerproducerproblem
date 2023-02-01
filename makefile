biuld:
	ipcrm -a
	g++ consumer.cpp -o consumer
	g++ producer.cpp -o producer
	gnome-terminal
clean:
	rm -f producer consumer *.o
