#include <queue.h>
#include "skel.h"

struct arp_entry *arp_table;
int arp_table_size;
struct route_table_entry *rtable;
uint32_t rtable_size;

/* Reads routing table from given file into the r_table parameter
 * Returns the routing table size
 */
int read_rtable(char *filename, struct route_table_entry *r_table) {
	FILE *fp;
	char *line;
	char *addr_byte_char;
	char buffer [100];
	uint32_t addr_byte, entries_no;
	uint32_t curr_char;
	uint8_t byte_offset[4] = {0, 8, 16, 24};
  
	fp = fopen(filename,"r");
	if(fp == NULL) {
		perror("Cannot open Routing table file");
		exit(-1);
	}
	entries_no = 0;
	/* Read file line by line */
	while ((line = fgets(buffer, 64, fp))) {
		/* Separate line by dots and spaces */
		addr_byte_char = strtok(buffer, " .");
		curr_char = 0;
		while (addr_byte_char != NULL) {
			addr_byte = atoi(addr_byte_char);
			/* Prefix */
			if (curr_char < 4)
				// << 24 if curr_char % 4 == 0, << 16 if curr_char % 4 == 1 .. 
				r_table[entries_no].prefix += addr_byte << 
												byte_offset[curr_char % 4];
	  		/* Next hop */
	 		else if (curr_char < 8)
				r_table[entries_no].next_hop += addr_byte << 
												byte_offset[curr_char % 4];
			/* Mask */
	  		else if (curr_char < 12)
				r_table[entries_no].mask += addr_byte << 
												byte_offset[curr_char % 4];
			/* Interface */
			else if (curr_char == 12)
				r_table[entries_no].interface = addr_byte;
			curr_char = curr_char + 1;
			addr_byte_char = strtok(NULL, " .");
		}
		/* Line is finished processing */
		entries_no = entries_no + 1;
	}
	fclose(fp);
	return entries_no;
}

/* Get ARP entry from ARP table which matches the given IP */
struct arp_entry *get_arp_entry(uint32_t ip) {
    for(int i = 0; i < arp_table_size; i++) {
    	if(arp_table[i].ip==ip) {
    		return &arp_table[i];
    	}
    }
    return NULL;
}

/* Add given IP and MAC to ARP table */
void add_arp_table(uint32_t ip, uint8_t *mac) {
	arp_table[arp_table_size].ip = ip;
	for(int i = 0; i < 6; i++) 
		arp_table[arp_table_size].mac[i] = mac[i];
	arp_table_size++;
}

/* Compares 2 routing table entries based on prefix and mask */
int comp_rtable_entry(const void *a, const void *b) {
	struct route_table_entry x = *((struct route_table_entry*) a);
	struct route_table_entry y = *((struct route_table_entry*) b);
	if(ntohl(x.prefix) == ntohl(y.prefix)) {
		return ntohl(x.mask) > ntohl(y.mask);
	}
	return ntohl(x.prefix) > ntohl(y.prefix);
}

/* Sort routing table */
void sort_rtable() {
	qsort(rtable, rtable_size, sizeof(*rtable), comp_rtable_entry);
}

/* binary search function for finding routing table entry */
int binarySearch(int left, int right, uint32_t dest_ip) {
	int mid;
	while (left <= right) {
		mid = right + (left - right)/2;
		if(ntohl((rtable[mid].mask &dest_ip)) == ntohl(rtable[mid].prefix)) 
			return mid;
		if(ntohl((rtable[mid].mask &dest_ip)) < ntohl(rtable[mid].prefix)) 
			right = mid - 1;
		if(ntohl((rtable[mid].mask &dest_ip)) > ntohl(rtable[mid].prefix)) 
			left = mid + 1;
	}
	return -1;
}

/* Searches for routing table entry that matches the given IP performing 
 *	a binary search on the routing table (routing table has to be sorted)
 */ 
struct route_table_entry *get_best_route_binary(uint32_t dest_ip) {
	int res = binarySearch(0, rtable_size, dest_ip);
	if(res > 0)
		return &rtable[res];
	return NULL;
}


/* Returns -1, if given IP doesn't belong to router
 * 		   the interface, if ip belongs to router 
 */
int is_router_ip(uint32_t ip) {
	struct sockaddr_in router_ip;
	for(int i = 0; i < ROUTER_NUM_INTERFACES; i++) {
		if(inet_aton(get_interface_ip(i), &router_ip.sin_addr) == 0) {
			fprintf(stderr, "Invalid router IP\n");
			continue;
		}
		if(router_ip.sin_addr.s_addr == ip)
			return i;
	}
	return -1;
}

int main(int argc, char *argv[]) {

	packet m;
	int rc;
	init(argc - 2, argv + 2);
	char *filename = argv[1];

	rtable = malloc(sizeof(struct route_table_entry) * MAX_ROUTER_TABLE);
	rtable_size = read_rtable(filename, rtable);
	sort_rtable();
	arp_table = malloc(sizeof(struct  arp_entry) * 100);
	queue packet_q = queue_create();

	while (1) {
		/* 1. Receive packet */
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");

		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		
		/* ARP PACKET */
		struct arp_header* arp_hdr = parse_arp(m.payload);
		if(arp_hdr != NULL) {
			/* ARP_REQUEST */
			if(ntohs(arp_hdr->op) == ARPOP_REQUEST) {
				int interface = is_router_ip(arp_hdr->tpa);
				/* 2. ARP_REQUEST for router */
				if(interface >= 0) {
					/* Send ARP_REPLY */
					uint8_t interface_mac[ETH_ALEN];
					get_interface_mac(interface, interface_mac);
					struct ether_header arp_eth_hdr;
					build_ethhdr(&arp_eth_hdr, 				// ether_header 
								interface_mac,				// source MAC
								arp_hdr->sha,				// destination MAC
								htons(ETHERTYPE_ARP)		// ETHER type
						);
					send_arp(arp_hdr->spa,					// destination IP
							arp_hdr->tpa,					// source IP
							&arp_eth_hdr,					// ether_header
							interface,						// interface
							htons(ARPOP_REPLY)				// ARP code
						);
				}
			}
			/* ARP_REPLY */
			else if(ntohs(arp_hdr->op) == ARPOP_REPLY) {
				/* 3. ARP REPLY for router */
				if(is_router_ip(arp_hdr->tpa) >= 0) {
					/* Update ARP Table */
					add_arp_table(arp_hdr->spa, arp_hdr->sha);
					/* Get packets from queue and send them */
					if(!queue_empty(packet_q)) {
						packet *pack = (packet*)(queue_deq (packet_q));
						struct ether_header *pack_eth_hdr = (struct ether_header *)pack->payload;

						memcpy(pack_eth_hdr->ether_dhost, arp_hdr->sha, 6);
						get_interface_mac(m.interface, pack_eth_hdr->ether_shost);

						/* Routing packet */
						send_packet(m.interface, pack);
					}
				}
			}
			/* If ARP packet is not destined for router, throw it */
			continue;
		}

		/* Verify if packet is IP */
		if(ntohs(eth_hdr->ether_type) != ETHERTYPE_IP)
			continue;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));

		/* 4. Verify checksum */
		rc = ip_checksum(ip_hdr, sizeof(struct iphdr));
		if(rc != 0) {
			fprintf(stderr, "Packet corrupted (wrong checksum)\n");
			continue;
		}

		/* 5. Check TTL */
		if(ip_hdr->ttl <= 1) {
			/* Send ICMP_TIME_TO_LIVE_EXCEEDED */
			send_icmp_error(ip_hdr->saddr,				// destination IP
				 ip_hdr->daddr,							// source IP
				 eth_hdr->ether_dhost,					// source MAC
				 eth_hdr->ether_shost,					// destination MAC
				 ICMP_TIME_EXCEEDED,					// ICMP type
				 ICMP_EXC_TTL,							// ICMP code
				 m.interface);							// interface
			continue;
		}

		/* 6. ICMP ECHO REQ */
		struct icmphdr* icmp_hdr = parse_icmp(m.payload);
		if(icmp_hdr != NULL && icmp_hdr->type == ICMP_ECHO) {
			/* ICMP_ECHO_REQ for router */
			if(is_router_ip(ip_hdr->daddr) > 0) {
				/* Send ICMP_ECHO_REPLY */
				send_icmp(ip_hdr->saddr,				// destination IP
				 ip_hdr->daddr,							// source IP
				 eth_hdr->ether_dhost,					// source MAC
				 eth_hdr->ether_shost,					// destination MAC
				 ICMP_ECHOREPLY,						// ICMP type
				 0,										// ICMP code
				 m.interface,							// interface
				 icmp_hdr->un.echo.id, 					// ICMP Id
				 icmp_hdr->un.echo.sequence				// ICMP Seq
				);
				continue;
			}
		}

		/* 7. Decrement TTL, re-calculate check-sum */
		ip_hdr->ttl--;
		ip_hdr->check = 0;
		ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));

		/* 8. Finding next_hop */
		struct route_table_entry *r = get_best_route_binary(ip_hdr->daddr);
		if(r == NULL) {
			/* Can't find route, sending ICMP_DESTINATION_UNREACHABLE */
			send_icmp_error(ip_hdr->saddr,				// destination IP
				 ip_hdr->daddr,							// source IP
				 eth_hdr->ether_dhost,					// source MAC
				 eth_hdr->ether_shost,					// destination MAC
				 ICMP_DEST_UNREACH,						// ICMP type
				 ICMP_NET_UNREACH,						// ICMP code
				 m.interface);							// interface
			/* Throw packet */
			continue;
		}

		/* Get router's interface IP */
		char router_ip_char[50];
		memcpy(router_ip_char, get_interface_ip(m.interface), 16);
		struct sockaddr_in router_ip;
		if(inet_aton(router_ip_char, &router_ip.sin_addr) == 0) {
			fprintf(stderr, "Invalid IP\n");
			continue;
		}

		/* Get next hop */
		struct arp_entry *arp_entry = get_arp_entry(r->next_hop);
		if(arp_entry == NULL) {
			/* Send ARP REQUEST */
			struct ether_header arp_eth_hdr;
			uint8_t interface_mac[ETH_ALEN];
			uint8_t broadcast_address[6] = 
								{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
			get_interface_mac(r->interface, interface_mac);
			build_ethhdr(&arp_eth_hdr, 						// ether_header 
						interface_mac,						// source MAC
						broadcast_address,					// destination MAC
						htons(ETHERTYPE_ARP)				// ethernet type
				);
			memcpy(router_ip_char, get_interface_ip(r->interface), 16);
			if(inet_aton(router_ip_char, &router_ip.sin_addr) == 0) {
				fprintf(stderr, "Invadlid IP\n");
				continue;
			}

			send_arp(r->next_hop,							// destination IP
					router_ip.sin_addr.s_addr,				// source IP
					&arp_eth_hdr,							// ether_header
					r->interface,							// interface
					htons(ARPOP_REQUEST)					// ARP code
				);

			/* Queue packet */
			packet queue_packet = m;
			queue_enq(packet_q, &queue_packet);
			continue;
		}

		/* 9. Complete ether_header */
		memcpy(eth_hdr->ether_dhost, arp_entry->mac, 6);
		get_interface_mac(r->interface, eth_hdr->ether_shost);

		/* 10. Routing packet */
		send_packet(r->interface, &m);
	}

	free(rtable);
	free(arp_table);
	return 0;
}
