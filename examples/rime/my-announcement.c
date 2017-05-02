/**
 * \file
 *         Example code that uses the annuncement module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "net/rime/rime.h"

#include "net/rime/announcement.h"

#include <stdio.h>

struct neighbor {
  /* The ->next pointer is needed since we are placing these on a
     Contiki list. */
  struct neighbor *next;

  /* The ->addr field holds the Rime address of the neighbor. */
  linkaddr_t addr;
};

/* This #define defines the maximum amount of neighbors we can remember. */
#define MAX_NEIGHBORS 16

/* This MEMB() definition defines a memory pool from which we allocate
   neighbor entries. */
MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS);

/* The neighbors_list is a Contiki list that holds the neighbors we
   have seen thus far. */
LIST(neighbors_list);

/*---------------------------------------------------------------------------*/
PROCESS(example_announcement_process, "Example announcement process");
AUTOSTART_PROCESSES(&example_announcement_process);
/*---------------------------------------------------------------------------*/
static void
received_announcement(struct announcement *a, const linkaddr_t *from,
		      uint16_t id, uint16_t value)
{
  struct neighbor *n;

  /* We set our own announced value to one plus that of our neighbor. */
  announcement_set_value(a, value + 1);

  printf("Got announcement from %d.%d, id %d, value %d, our new value is %d\n",
	 from->u8[0], from->u8[1], id, value, value + 1);

  /* Check if we already know this neighbor. */
  for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {

    /* We break out of the loop if the address of the neighbor matches
       the address of the neighbor from which we received this
       broadcast message. */
    if(linkaddr_cmp(&n->addr, from)) {
      printf("Neighbor found %d.%d\n", from->u8[0], from->u8[1]);
      break;
    }
  }

  /* If n is NULL, this neighbor was not found in our list, and we
     allocate a new struct neighbor from the neighbors_memb memory
     pool. */
  if(n == NULL) {
    n = memb_alloc(&neighbors_memb);

    /* If we could not allocate a new neighbor entry, we give up. We
       could have reused an old neighbor entry, but we do not do this
       for now. */
    if(n == NULL) {
      return;
    }

    /* Initialize the fields. */
    linkaddr_copy(&n->addr, from);

    /* Place the neighbor on the neighbor list. */
    list_add(neighbors_list, n);

    printf("Neighbor added %d.%d\n", from->u8[0], from->u8[1]);
  }
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
