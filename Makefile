topic-miami-monitor-example: topic-miami-monitor-example.o topic-miami-monitor-lib.o
	$(CC) -o $@ topic-miami-monitor-example.o topic-miami-monitor-lib.o

topic-miami-monitor-lib.o: topic-miami-monitor-lib.c topic-miami-monitor-lib.h

topic-miami-monitor-example.o: topic-miami-monitor-example.c topic-miami-monitor-lib.h

