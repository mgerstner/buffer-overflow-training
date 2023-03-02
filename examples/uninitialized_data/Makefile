SERVER = random_svc
CLIENT = random_client
PSEUDO = pseudo_random
BINS = $(SERVER) $(CLIENT) $(PSEUDO)
FLAGS = -g -O0
HEADERS = common.h

all: $(BINS)

$(SERVER): random_svc.c $(HEADERS)
	gcc $(FLAGS) $< -o $@

$(CLIENT): random_client.c $(HEADERS)
	gcc $(FLAGS) $< -o $@

$(PSEUDO): pseudo_random.c $(HEADERS)
	gcc $(FLAGS) $< -o $@

clean:
	rm -f $(BINS)
