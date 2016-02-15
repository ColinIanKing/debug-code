CFLAGS  += -O2 -Wall -Werror 
LDFLAGS += -lm

hz-test: hz-test.o
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm hz-test hz-test.o
