#ifndef _UTIL_H_
#define _UTIL_H_

struct request{
    int seat_id;
    int user_id;
    int customer_priority;
    char* resource;
};

// Struct so we can pass in the args to pthread_create
struct args_t {
  int connfd;
  struct request* req;
};

void parse_request(int, struct request*);
void process_request(void* args);

#endif
