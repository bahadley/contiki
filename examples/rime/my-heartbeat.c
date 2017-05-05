#include "contiki.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "net/rime/polite.h"

#include <stdio.h>

#define CHANNEL 136

#define FIXED_LEADER_ID 1
#define NO_LEADER_ID 0
#define LEADER_TIMEOUT 60 * CLOCK_SECOND

struct leader {
  struct leader *next;
  uint8_t leader;
  uint8_t resend;
  struct ctimer ctimer;
};

#define MAX_LEADERS 1
LIST(leader_table);
MEMB(leader_mem, struct leader, MAX_LEADERS);
/*---------------------------------------------------------------------------*/
PROCESS(example_heartbeat, "heartbeat example");
AUTOSTART_PROCESSES(&example_heartbeat);
/*---------------------------------------------------------------------------*/

static void
clear_leader(void *n)
{
  struct leader *l = n;
  l->leader = 0;
  l->resend = 3;
}

static void
recv(struct polite_conn *c)
{
  uint8_t hrt_bt = ((uint8_t *)packetbuf_dataptr())[0];

  printf("message received '0x%02x'\n", hrt_bt);

  if (hrt_bt == FIXED_LEADER_ID) {
    struct leader *e = list_head(leader_table);
    if(e != NULL && e->leader == NO_LEADER_ID) {
      e->leader = hrt_bt; 
      e->resend = 3;
      ctimer_set(&e->ctimer, LEADER_TIMEOUT, clear_leader, e);
    }
  }
}

static void
sent(struct polite_conn *c)
{
  printf("sent\n");
}

static void
dropped(struct polite_conn *c)
{
  printf("dropped\n");
}

static const struct polite_callbacks leader_callbacks = { sent };
static const struct polite_callbacks follower_callbacks = { recv, sent, dropped };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_heartbeat, ev, data)
{
  static struct polite_conn c;

  PROCESS_EXITHANDLER(polite_close(&c));
  PROCESS_BEGIN();


  if (linkaddr_node_addr.u8[0] == FIXED_LEADER_ID) {
    polite_open(&c, CHANNEL, &leader_callbacks);
  }
  else {
    polite_open(&c, CHANNEL, &follower_callbacks);

    memb_init(&leader_mem);
    list_init(leader_table);
    struct leader *e = memb_alloc(&leader_mem);
    if(e != NULL) {
      e->leader = NO_LEADER_ID; 
      e->resend = 0;
      list_add(leader_table, e);
    }
  }

  while(1) {
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if (linkaddr_node_addr.u8[0] == FIXED_LEADER_ID) {
      uint8_t ldr = FIXED_LEADER_ID;
      packetbuf_copyfrom(&ldr, sizeof(ldr));
      polite_send(&c, CLOCK_SECOND * 4, 4);
    }
    else {
      struct leader *l = list_head(leader_table);
      if(l != NULL && l->resend > 0) {
        packetbuf_copyfrom(&l->leader, sizeof(l->leader));
        l->resend -= 1;
        polite_send(&c, CLOCK_SECOND * 4, 4);
      }
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
