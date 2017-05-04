/**
 * \file
 *         Example for using the trickle code in Rime
 *           modified from example-trickle.c
 * \author
 *         Ben Hadley 
 */

#include "contiki.h"
#include "net/rime/trickle.h"

#include "random.h"

#include <stdio.h>

static uint8_t node;

/*---------------------------------------------------------------------------*/
PROCESS(example_trickle_process, "Trickle example");
AUTOSTART_PROCESSES(&example_trickle_process);
/*---------------------------------------------------------------------------*/
static void
trickle_recv(struct trickle_conn *c)
{
  printf("%d.%d: trickle message received '0x%02x'\n",
	 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
	 ((uint8_t *)packetbuf_dataptr())[0]);
}
const static struct trickle_callbacks trickle_call = {trickle_recv};
static struct trickle_conn trickle;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_trickle_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(trickle_close(&trickle);)
  PROCESS_BEGIN();

  trickle_open(&trickle, CLOCK_SECOND, 145, &trickle_call);

  node = 0;
  node |= (1 << (linkaddr_node_addr.u8[0] - 1));

  while(1) {
    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 2 + random_rand() % (CLOCK_SECOND * 2));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom(&node, 1);
    trickle_send(&trickle);
    printf("trickle message sent\n");
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
