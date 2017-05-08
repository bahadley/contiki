/**
 * \file
 *         Example code that uses the announcement module
 * \author
 *         Ben Hadley 
 *           based on example-announcement.c
 */

#include "contiki.h"
#include "lib/memb.h"
#include "net/rime/rime.h"

#include "net/rime/announcement.h"

#include <stdio.h>


#define FIXED_LEADER_ID 1
#define NO_LEADER_ID 0
#define LEADER_TIMEOUT 30 * CLOCK_SECOND

struct current_leader {
  uint16_t leader;
  struct ctimer ctimer;
};

MEMB(leader_mem, struct current_leader, 1);
static struct current_leader *ldr;

/*---------------------------------------------------------------------------*/
PROCESS(heartbeat_announcement_process, "Heartbeat announcement process");
AUTOSTART_PROCESSES(&heartbeat_announcement_process);
/*---------------------------------------------------------------------------*/

static void
clear_leader(void *n)
{
  struct announcement *a = n;
  ldr->leader = NO_LEADER_ID;
  /*announcement_set_value(a, ldr->leader);*/
}

static void
received_announcement(struct announcement *a, const linkaddr_t *from,
		      uint16_t id, uint16_t value)
{
  printf("Got announcement from %d.%d, id %d, value %d\n",
	 from->u8[0], from->u8[1], id, value);

  if (linkaddr_node_addr.u8[0] == FIXED_LEADER_ID) {
    return;
  }

  if (value == FIXED_LEADER_ID) {
    ldr->leader = value;
    ctimer_set(&ldr->ctimer, LEADER_TIMEOUT, clear_leader, a);
  }

  /*announcement_set_value(a, ldr->leader);*/
}

static struct announcement heartbeat_announcement;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(heartbeat_announcement_process, ev, data)
{
  PROCESS_EXITHANDLER(announcement_remove(&heartbeat_announcement);)
    
  PROCESS_BEGIN();

  /* Register an announcement with ID 128. We provide the
     'received_announcement' function pointer so that this function
     will be called when a announcements from neighbors are heard. */
  announcement_register(&heartbeat_announcement,
			128,
			received_announcement);

  /* Set the lowest eight bytes of the Rime address as the value. */
  if (linkaddr_node_addr.u8[0] == FIXED_LEADER_ID) {
    announcement_set_value(&heartbeat_announcement, FIXED_LEADER_ID);
  }
  else {
    /*announcement_set_value(&heartbeat_announcement, NO_LEADER_ID);*/

    memb_init(&leader_mem);
    ldr = memb_alloc(&leader_mem);
    if(ldr != NULL) {
      ldr->leader = NO_LEADER_ID;
    }
  }

  while(1) {
    static struct etimer et;

    /* Listen for announcements every ten seconds. */
    etimer_set(&et, CLOCK_SECOND * 10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if (linkaddr_node_addr.u8[0] == FIXED_LEADER_ID) {
      announcement_bump(&heartbeat_announcement);
    } else {
      announcement_listen(1);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
