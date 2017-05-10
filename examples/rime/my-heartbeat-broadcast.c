/**
 * \file
 *         Testing the broadcast layer in Rime
 *           modified from example-broadcast.c
 * \author
 *         Ben Hadley 
 */

#include "contiki.h"
#include "net/rime/rime.h"

#include <stdio.h>

#define FIXED_LEADER_ID 1

/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '0x%02x'\n",
         from->u8[0], from->u8[1], ((uint8_t *)packetbuf_dataptr())[0]);
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    if (linkaddr_node_addr.u8[0] == FIXED_LEADER_ID) {
      etimer_set(&et, CLOCK_SECOND * 4);
    } else {
      etimer_set(&et, CLOCK_SECOND * 8);
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if (linkaddr_node_addr.u8[0] == FIXED_LEADER_ID) {
      packetbuf_copyfrom(&linkaddr_node_addr.u8[0], 1);
      /* packetbuf_copyfrom("Alive", 6); */
      broadcast_send(&broadcast);
      printf("broadcast message sent\n");
    } else {
      printf("Follower awake\n");
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
