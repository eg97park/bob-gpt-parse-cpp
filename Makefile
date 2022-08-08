all: gpt-parse

gpt-parse: main.o
	$(LINK.cc) $^ -o $@

clean:
	rm -f gpt-parse *.o