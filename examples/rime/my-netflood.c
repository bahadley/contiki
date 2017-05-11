
#include "net/rime/netflood.h"
#include "contiki.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(netflood_process, "");
AUTOSTART_PROCESSES(&netflood_process);
/*---------------------------------------------------------------------------*/
static int 
recv(struct netflood_conn *c, const linkaddr_t *from,
               const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  printf("from: %d.%d, originator: %d.%d, seq: %d, hops: %d\n", 
    from->u8[0], from->u8[1], originator->u8[0], originator->u8[1], seqno, hops);

  return 1;
}
static void
sent(struct netflood_conn *c)
{
  printf("sent\n");
}
static void
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

  static uint8_t seqno = 0;

  while(1) {
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if (linkaddr_node_addr.u8[0] == 1) {
      /*packetbuf_copyfrom("Hej", 4);*/
      seqno += 1;
      netflood_send(&c, seqno);
    } 
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
