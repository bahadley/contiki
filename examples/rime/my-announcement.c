/**
 * \file
 *         Example code that uses the annuncement module
 * \author
 *         Ben Hadley
 */

#include "contiki.h"
#include "net/rime/rime.h"

#include "net/rime/announcement.h"

#include <stdio.h>

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif

/*---------------------------------------------------------------------------*/
PROCESS(example_announcement_process, "Example announcement process");
AUTOSTART_PROCESSES(&example_announcement_process);
/*---------------------------------------------------------------------------*/
static void
received_announcement(struct announcement *a, const linkaddr_t *from,
		      uint16_t id, uint16_t value)
{
  /* We set our own announced value to one plus that of our neighbor. */
  announcement_set_value(a, value + 1);

  printf("Got announcement from %d.%d, id %d, value %d, our new value is %d\n",
	 from->u8[0], from->u8[1], id, value, value + 1);

#if CONTIKI_TARGET_NETSIM
  {
    char buf[8];
    sprintf(buf, "%d", value + 1);
    ether_set_text(buf);
  }
#endif

}
static struct announcement example_announcement;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_announcement_process, ev, data)
{
  PROCESS_EXITHANDLER(announcement_remove(&example_announcement);)
    
  PROCESS_BEGIN();

  /* Register an announcement with ID 128. We provide the
     'received_announcement' function pointer so that this function
     will be called when a announcements from neighbors are heard. */

  announcement_register(&example_announcement,
			128,
			received_announcement);

  /* Set the lowest eight bytes of the Rime address as the value. */
  announcement_set_value(&example_announcement, linkaddr_node_addr.u8[0]);

  while(1) {
    static struct etimer et;

    /* Listen for announcements every ten seconds. */
    etimer_set(&et, CLOCK_SECOND * 10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    announcement_listen(1);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
