ThisSocks : *.cpp *.h
	g++ *.cpp -o $@ -O3

clean :
	rm -f ThisSocks *.o
