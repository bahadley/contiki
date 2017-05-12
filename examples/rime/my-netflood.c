#include "contiki.h"
#include "net/rime/rime.h"

#include <stdio.h>

#define FIXED_LEADER_ID 1
#define NO_LEADER_ID 0
#define LEADER_TIMEOUT 20 * CLOCK_SECOND

struct current_leader {
  uint8_t leader;
  struct ctimer ctimer;
};

static struct current_leader ldr;

/*---------------------------------------------------------------------------*/
PROCESS(netflood_process, "heartbeat-netflood");
AUTOSTART_PROCESSES(&netflood_process);
/*---------------------------------------------------------------------------*/

static void
clear_leader(void *ptr)
{
  printf("ALERT: no leader\n");
  ldr.leader = NO_LEADER_ID;
}

int 
recv(struct netflood_conn *c, const linkaddr_t *from,
     const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  printf("from: %d.%d, originator: %d.%d, seq: %d, hops: %d\n", 
    from->u8[0], from->u8[1], originator->u8[0], originator->u8[1], seqno, hops);

  ldr.leader = originator->u8[0];
  ctimer_set(&ldr.ctimer, LEADER_TIMEOUT, clear_leader, NULL);

  return 1;
}

void
sent(struct netflood_conn *c)
{
  printf("sent\n");
}

void
dropped(struct netflood_conn *c)
{
  printf("dropped\n");
}

static const struct netflood_callbacks callbacks = { recv, sent, dropped };
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(netflood_process, ev, data)
{
  static struct netflood_conn c;

  PROCESS_EXITHANDLER(netflood_close(&c));
  PROCESS_BEGIN();

  netflood_open(&c, CLOCK_SECOND * 4, 136, &callbacks);

  static struct etimer et;
  static uint8_t seqno = 0;

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if (linkaddr_node_addr.u8[0] == FIXED_LEADER_ID) {
      seqno += 1;
      netflood_send(&c, seqno);
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
