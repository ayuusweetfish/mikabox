#include "v3d.h"
#include "main.h"
#include "common.h"
#include "prop_tag.h"
#include "printf/printf.h"

void v3d_init()
{
  set_clock_rate(5, 250 * 1000 * 1000);
  if (!enable_qpu()) {
    printf("Cannot enable QPUs, what can go wrong?\n");
    return;
  }
  printf("QPUs enabled\n");
}
