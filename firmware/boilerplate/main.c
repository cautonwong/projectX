#include <zephyr/kernel.h>

int main() {
  while (1) {
    k_sleep(K_MSEC(100));
  }
}
