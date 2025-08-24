#include <../libmsx/service.h>
#include <uv.h>

struct core_command_service {
  service_t *base;
  uv_timer_t timer;
};

int main() {
  service_init()
}