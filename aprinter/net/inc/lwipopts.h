/*
 * Copyright (c) 2015 Ambroz Bizjak
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// TBD: These should come from generate.py.
#define APRINTER_NUM_IP_REASS_PKTS 1
#define APRINTER_NUM_TCP_CONN 2
#define APRINTER_NUM_TCP_LISTEN 1
#define APRINTER_MEM_ALIGNMENT 4

// Simple options, mostly enable/disable.
#define NO_SYS 1
#define MEM_ALIGNMENT APRINTER_MEM_ALIGNMENT
#define ETH_PAD_SIZE 2
#define ARP_QUEUEING 0
#define IP_FRAG 0
#define LWIP_NETIF_HOSTNAME 1
#define LWIP_RAW 0
#define LWIP_DHCP 1
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0
#define LWIP_STATS 0
#define LWIP_DHCP_CHECK_LINK_UP 1
#define LWIP_DISABLE_TCP_SANITY_CHECKS 1

// Some limits.
#define ARP_TABLE_SIZE 10
#define MEMP_NUM_UDP_PCB 1 // one PCB for DHCP
#define IP_REASS_MAX_PBUFS APRINTER_NUM_IP_REASS_PKTS
#define MEMP_NUM_REASSDATA APRINTER_NUM_IP_REASS_PKTS

// TCP limits.
#define MEMP_NUM_TCP_PCB APRINTER_NUM_TCP_CONN
#define MEMP_NUM_TCP_PCB_LISTEN APRINTER_NUM_TCP_LISTEN
#define MEMP_NUM_FRAG_PBUF 1
#define TCP_MSS 1460
#define TCP_SND_BUF 2920
#define TCP_SND_QUEUELEN 10
#define MEMP_NUM_TCP_SEG (APRINTER_NUM_TCP_CONN * TCP_SND_QUEUELEN)

// Number of pbufs in PBUF pool.
// These are allocated via pbuf_alloc(..., PBUF_ROM or PBUF_REF) and
// reference external data. They are used:
// - In the TCP TX path (tcp_write), they reference application data
//   that is passed to tcp_write() without TCP_WRITE_FLAG_COPY.
//   Note that lots of small pbufs might get allocated if we call tcp_output
//   many times with small bits of data. So currently we reserve something
//   and hope for the best.
// - In the fragmentation of IP packets, they reference parts of the
//   original full packet. Since we don't need and disable fragmentation,
//   we don't reserve anything for this.
#define MEMP_NUM_PBUF (APRINTER_NUM_TCP_CONN * (4 * TCP_SND_QUEUELEN))

// Number of pbufs in PBUF_POOL pool.
// These are allocated via pbuf_alloc(..., PBUF_POOL) and are used only
// in the RX path. They come with their own payload space.
// Note that:
// - The RX code and nothing else allocates pbufs from PBUF_POOL.
// - The RX code immediately inputs allocated pbufs into the stack,
//   it does not queue them.
// - The application code never refuses received pbufs in the tcp_recv callback.
// - The stack may internally buffer up to IP_REASS_MAX_PBUFS received pbufs
//   for IP reassembly.
// Based on this knowledge, the value below should be sufficient, we should
// never run out of pbufs in PBUF_POOL for receiving packets.
#define PBUF_POOL_SIZE (1 + APRINTER_NUM_IP_REASS_PKTS)

// Memory size for the general allocator.
// Importantly, this is used for pbuf_alloc(..., PBUF_RAM). This includes:
// - Outgoing and incoming UDP packets (e.g. used in DHCP).
// - Outgoing TCP ACK and RST packets.
// - Headers for outgoing TCP segments generated in tcp_write() when used
//   without TCP_WRITE_FLAG_COPY.
#define MEM_SIZE (1024 + APRINTER_NUM_TCP_CONN * (256 + TCP_SND_QUEUELEN * 128))