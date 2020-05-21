#include "counter.h"

#define LOG_TAG "counter"
#include "../../logger.h"

#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    log_error("Wrong number of arguments. Received %d, expected %d", argc - 1,
              2);
    return -1;
  }
  return unitnos_counter_self_main(atoi(argv[1]), atoi(argv[2]));
}
