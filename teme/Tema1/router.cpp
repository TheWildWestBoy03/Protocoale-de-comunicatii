#include "include/queue.h"
#include "include/lib.h"
#include "include/protocols.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>

using namespace std;

int nodes = 0;

class TrieNode {
	private:

	public:
		int bit_set;
		int routesLen;
		TrieNode *left_not_set_bit;
		TrieNode *right_set_bit;
		route_table_entry *route_entries;

		TrieNode() {
			this -> left_not_set_bit = NULL;
			this -> right_set_bit = NULL;
			this -> routesLen = 0;
		}

		TrieNode(int bit) {
			this -> bit_set = bit;
			this -> left_not_set_bit = NULL;
			this -> right_set_bit = NULL;
			this -> routesLen = 0;
		}

		~TrieNode() {
			freeTrie();
		}

		void freeTrie() {
			// la final
		}

};

class Trie {
	public:
		TrieNode *dummy;

		Trie() {
			dummy = new TrieNode();
		}

		void print_ip(uint32_t ip_address) {
			struct in_addr address = {ip_address};
			cout << inet_ntoa(address) << " ";
		}

		TrieNode *insert(route_table_entry *route_entry, TrieNode *treeDummy, uint32_t ip_prefix, uint32_t mask, uint32_t remaining) {
			int current_bit = ip_prefix & mask;

			// if (ip_prefix == 3232235776) {
			// 	if (current_bit == 0) {
			// 		cout << current_bit << " ";
			// 	} else {
			// 		cout << 1 << " ";
			// 	}
			// }
	
			if (mask < 1) {
				if (treeDummy -> left_not_set_bit == NULL) {
					treeDummy -> left_not_set_bit = new TrieNode(0);
				}
				if (treeDummy -> left_not_set_bit -> route_entries == NULL) {
					treeDummy -> left_not_set_bit -> route_entries = new route_table_entry[100];
				}

				treeDummy -> left_not_set_bit -> route_entries[treeDummy -> left_not_set_bit -> routesLen].interface = route_entry->interface;
				treeDummy -> left_not_set_bit -> route_entries[treeDummy -> left_not_set_bit -> routesLen].next_hop = route_entry->next_hop;
				treeDummy -> left_not_set_bit -> route_entries[treeDummy -> left_not_set_bit -> routesLen].prefix = route_entry->prefix;
				treeDummy -> left_not_set_bit -> route_entries[treeDummy -> left_not_set_bit -> routesLen].mask = route_entry->mask;

				if (ip_prefix == 3232235520 || ip_prefix == 3232235776) {
					cout << treeDummy -> left_not_set_bit -> routesLen << " ";
					print_ip(treeDummy -> left_not_set_bit -> route_entries[treeDummy -> left_not_set_bit -> routesLen].prefix);
					print_ip(treeDummy -> left_not_set_bit -> route_entries[treeDummy -> left_not_set_bit -> routesLen].next_hop);
					print_ip(treeDummy -> left_not_set_bit -> route_entries[treeDummy -> left_not_set_bit -> routesLen].mask);
					cout << treeDummy -> left_not_set_bit -> route_entries[treeDummy -> left_not_set_bit -> routesLen].interface;
					cout << endl;
				}
				
				treeDummy -> left_not_set_bit -> routesLen ++;
				treeDummy -> left_not_set_bit -> left_not_set_bit = treeDummy -> left_not_set_bit -> right_set_bit = NULL;
				return treeDummy;
			}

			if (current_bit != 0) {
				remaining -= mask;
				if (treeDummy -> right_set_bit == NULL) {
					treeDummy -> right_set_bit = new TrieNode(1);
				}
				treeDummy -> right_set_bit = insert(route_entry, treeDummy -> right_set_bit, ip_prefix, mask >> 1, remaining);
			} else {
				if (treeDummy -> left_not_set_bit == NULL) {
					treeDummy -> left_not_set_bit = new TrieNode(0);
				}
				treeDummy -> left_not_set_bit = insert(route_entry, treeDummy -> left_not_set_bit, ip_prefix, mask >> 1, remaining);
			}

			return treeDummy;
		} 

		void inorder(TrieNode *root) {
			if (root != NULL) {
				inorder(root -> left_not_set_bit);
				nodes ++;
				inorder(root -> right_set_bit);
			}
		}
};

class Router {
	private:
		char *packet;
		char *routingTable;
		int route_table_len;
		int queue_len;
		route_table_entry *route_table_entries;
		arp_table_list *arp_cache;
		queued arp_queue;
		int coming_interface;
		int buffer_len;
		Trie *routing_trie;
	
	public:
		Router() {
			routing_trie = NULL;
		}
		void setQueueLen() {
			queue_len = 0;
		}
		void setBufferLen(int len) {
			this -> buffer_len = len;
			cout << buffer_len << endl;
		}
		void setQueue(queued arp_queue) {
			this -> arp_queue = arp_queue;
		}
		void setNewPacket(char *packet) {
			this -> packet = packet;
		}

		void setComingInterface(int interface) {
			this -> coming_interface = interface;
		}

		void setRTable(char *rtable) {
			this -> routingTable = rtable;
		}

		char *getRTable() {
			return routingTable;
		}

		int getComingInterface() {
			return coming_interface;
		}

		queued getQueue() {
			return arp_queue;
		}

		int getLen() {
			return buffer_len;
		}
		void setArpCache(arp_table_list *arp_cache) {
			this -> arp_cache = arp_cache;
		}

		void setRTableLen(int route_table_len) {
			this -> route_table_len = route_table_len;
		} 

		int getRTableLen() {
			return route_table_len;
		}
		arp_table_list *getArpCache() {
			return arp_cache;
		}

		void setRouteTableEntries(route_table_entry *route_table_entries) {
			this -> route_table_entries = route_table_entries;
		}
		route_table_entry *getRouteTableEntries() {
			return route_table_entries;
		}

		arp_table_list *add_new_mac(uint8_t *mac, uint32_t ip) {
			arp_table_entry_node *new_node = new arp_table_entry_node();
			arp_table_entry *new_arp_entry = new arp_table_entry();

			new_arp_entry -> ip = ip;
			memcpy(new_arp_entry -> mac, (void *) mac, 6);
			new_node -> arp_entry = new_arp_entry;
			new_node -> next = NULL;

			arp_table_entry_node *curr_head = arp_cache -> head;
			
			if (arp_cache -> arp_table_len >= 100) {
				while (curr_head -> next != NULL) {
					curr_head = curr_head -> next;
				}

				curr_head = new_node;
				new_node -> next = NULL;
			}

			if (curr_head == NULL) {
				curr_head = new_node;
				arp_cache -> arp_table_len ++;
				arp_cache -> head = curr_head;

				return arp_cache;
			} else {
				new_node -> next = curr_head;
				curr_head = new_node;
				arp_cache -> arp_table_len ++;
				arp_cache -> head = curr_head;
			}

			return arp_cache;
		}

		uint8_t *get_mac_from_ip(uint32_t dest_ip) {
			cout << "Starting getting mac function " << endl;
			arp_table_entry_node *current_head = arp_cache -> head;
			uint8_t *resulting_mac = NULL;
	
			cout << "Starting iteration" << endl;
			while (current_head != NULL) {
				arp_table_entry *arp_entry = current_head->arp_entry;
				cout << "ARP ENTRY GOT" << endl;
				if (arp_entry->ip == dest_ip) {
					resulting_mac = new uint8_t[6];
					memcpy(resulting_mac, arp_entry->mac, 6);
					return resulting_mac;
				}

				current_head = current_head -> next;
			}

			return resulting_mac;
		}

		void send_icmp(uint8_t type, uint8_t code) {
			iphdr *dropped_ip_header = (iphdr *) (packet + sizeof(ether_header));
			ether_header *dropped_ether_header = (ether_header *) packet;

			char *new_packet = new char[MAX_PACKET_LEN];
			ether_header *new_ether_header = (ether_header *) new_packet;
			iphdr *new_ip_header = (iphdr *) (new_packet + sizeof(ether_header));
			icmphdr *new_icmp_header = (icmphdr *) (new_packet + sizeof(ether_header) + sizeof(iphdr));

			int total_length = sizeof(ether_header) + sizeof(iphdr) + sizeof(icmphdr);
			memcpy(new_ether_header -> ether_dhost, dropped_ether_header -> ether_shost, 6);
			get_interface_mac(coming_interface, new_ether_header -> ether_shost);
			new_ether_header -> ether_type = htons(ETHERTYPE_IP);

			memcpy(new_ip_header, dropped_ip_header, sizeof(iphdr));
			new_ip_header -> tot_len = htons(sizeof(iphdr) + sizeof(icmphdr));
			new_ip_header -> ttl = 64;
			new_ip_header -> protocol = IPPROTO_ICMP;
			new_ip_header -> daddr = dropped_ip_header -> saddr;
			new_ip_header -> saddr = convert_address_to_decimal(get_interface_ip(coming_interface));
			new_ip_header -> check = 0;

			new_ip_header -> check = ntohs(checksum((uint16_t *) new_ip_header, sizeof(iphdr)));
			new_icmp_header -> type = type;
			new_icmp_header -> code = code;
			new_icmp_header -> checksum = new_ip_header -> check;

			char *remaining = new char [8];
			memcpy(remaining, packet, 8);
			
			char *last_bytes = packet + total_length;
			memcpy(last_bytes, dropped_ip_header, sizeof(iphdr));
			memcpy(last_bytes + 8, remaining, 8);

			total_length = total_length + sizeof(iphdr) + 8;
			send_to_link(coming_interface, new_packet, total_length);
		}

		uint32_t convert_address_to_decimal(char *ip_address) {
			char *pointer = strtok(ip_address, ".");
			uint32_t final_conversion = 0;
			int exponent = 24;

			while (pointer != NULL) {
				uint32_t number = atoi(pointer);
				final_conversion = final_conversion | (number << exponent);
				exponent -= 8;
				pointer = strtok(NULL, ".");
			}

			return final_conversion;
		}
		void handleIpType() {
			ether_header *curr_ether_header = (ether_header *) packet;
			iphdr *ip_header = (iphdr *) (packet + sizeof(ether_header));

			// UNDER CONSTRUCTION
			
			uint16_t ip_hdr_sum = ntohs(ip_header -> check);
			uint16_t ip_checksum = ip_header -> check;

			ip_header -> check = 0;

			if (checksum((uint16_t *)ip_header, sizeof(iphdr)) != ip_hdr_sum) {
				cout << "Packet corrupted" << endl;
				return;
			}

			ip_header -> check = ip_checksum;
			int old_ttl = ip_header->ttl;
			int old_check = ip_header->check;
			ip_header->ttl--;
			ip_header->check = ~(~old_check + ~((uint16_t) old_ttl) + (uint16_t) ip_header->ttl) - 1;

			if (convert_address_to_decimal(get_interface_ip(coming_interface)) == ntohl(ip_header -> daddr)) {
				send_icmp(0, 0);
				return;
			}

			if (ip_header -> ttl <= 1) {
				cout << "This packet is too old!" << endl;
				send_icmp(11, 0);
				return;
			}

			route_table_entry *best_entry = getTheBestRoute(routing_trie -> dummy, ntohl(ip_header -> daddr), 2147483648, ntohl(ip_header -> daddr));

			if (best_entry == NULL) {
				cout << "No ip in this table" << endl;
				send_icmp(3, 0);
				return;
			}

			uint8_t *mac = get_mac_from_ip(best_entry -> next_hop);

			if (mac == NULL) {
				// we need to do broadcasting

				Packet *inserted_packet = new Packet();
				inserted_packet -> packet = new char[MAX_PACKET_LEN];
				memcpy(inserted_packet -> packet, packet, buffer_len);
				inserted_packet->interface = best_entry -> interface;
				inserted_packet->best_route = best_entry;
				inserted_packet->packet_len = buffer_len;
				queue_enq(arp_queue, (void *)inserted_packet);

				// creating new arp header
				char new_packet[MAX_PACKET_LEN];
				ether_header *new_ether_header = (ether_header *) new_packet;

				uint8_t broadcast[6];
				for (int i = 0; i < 6; i++) {
					broadcast[i] = 0XFF;
				}

				new_ether_header -> ether_type = htons(ETHERTYPE_ARP);
				get_interface_mac(best_entry -> interface, new_ether_header -> ether_shost);
				memcpy(new_ether_header -> ether_dhost, broadcast, sizeof(new_ether_header -> ether_dhost));

				arp_header *new_arp_header = (arp_header*) ((char *)new_packet + sizeof(ether_header));
				new_arp_header -> hlen = 6;
				new_arp_header -> plen = 4;
				new_arp_header -> htype = htons(1);
				new_arp_header -> ptype = htons(ETHERTYPE_IP);
				memcpy(new_arp_header -> sha, new_ether_header -> ether_shost, 6);
				memcpy(new_arp_header -> tha, new_ether_header -> ether_dhost, 6);
				new_arp_header -> tpa = best_entry -> next_hop;
				new_arp_header -> spa = inet_addr(get_interface_ip(best_entry->interface));
				new_arp_header -> op = htons(1);
			
				send_to_link(best_entry -> interface, new_packet, sizeof(ether_header) + sizeof(arp_header));
			} else {
				memcpy(curr_ether_header -> ether_dhost, mac, 6);
				
				send_to_link(best_entry -> interface, packet, buffer_len);
			}
		}
	
		void print_ip(uint32_t ip_address) {
			struct in_addr address = {ip_address};
			cout << inet_ntoa(address) << " ";
		}

		void print_mac(uint8_t *mac) {
			printf("%x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}

		bool is_mac_in_cache(uint8_t *mac) {
			arp_table_entry_node *head = arp_cache -> head;

			while (head != NULL) {
				if (!memcmp(head -> arp_entry -> mac, mac, 6)) {
					return true;
				}

				head = head -> next;
			}

			return false;
		}
		void handleArpType() {
			ether_header *current_ether_header = (ether_header *) packet;
			arp_header *current_arp_header = (arp_header *) (packet + sizeof(ether_header));

			if (current_arp_header -> op == ntohs(1)) {	

				cout << "ARP REQUEST from " << endl;
				print_ip(current_arp_header->spa);
				print_ip(current_arp_header->tpa);

				current_arp_header -> op = htons(2);

				current_arp_header -> tpa = current_arp_header -> spa;
				current_arp_header -> spa = inet_addr(get_interface_ip(coming_interface));

				memcpy(current_arp_header -> tha, current_arp_header -> sha, 6);
				get_interface_mac(coming_interface, current_arp_header -> sha);

				memcpy(current_ether_header -> ether_dhost, current_ether_header -> ether_shost, 6);
				get_interface_mac(coming_interface, current_ether_header -> ether_shost);

				print_mac(current_arp_header -> sha);
				print_mac(current_arp_header -> tha);

				send_to_link(coming_interface, packet, buffer_len);

			} else if (current_arp_header -> op == ntohs(2)) {

				cout << "ARP REPLY" << endl;
				if (!is_mac_in_cache(current_arp_header -> sha)) {
					arp_cache = add_new_mac(current_arp_header -> sha, current_arp_header -> spa);
				}

				while (!queue_empty(arp_queue)) {
					Packet *full_packet = (Packet *) queue_deq(arp_queue);
				
					char *current_packet = full_packet -> packet;
					ether_header *curr_ether_header = (ether_header *) current_packet;
	
					route_table_entry *best_route = full_packet -> best_route;
					uint8_t *mac = get_mac_from_ip(best_route -> next_hop);

					cout << "FINISHED" << endl;
					if (mac != NULL && best_route -> next_hop == current_arp_header -> spa) {
						memcpy(curr_ether_header -> ether_dhost, mac, 6);
						queue_len--;

						send_to_link(best_route -> interface, full_packet -> packet, full_packet -> packet_len);
					} else {
						break;
					}

				} 
			}
		}

		route_table_entry *getTheBestEntry(uint32_t destination_ip) {
			route_table_entry *best_entry = NULL;

			for (int i = 0; i < route_table_len; i++) {
				if ((destination_ip & route_table_entries[i].mask) == route_table_entries[i].prefix) {		
					best_entry = &route_table_entries[i];
					return best_entry;
				}
			}

			return best_entry;
		}


		void prefixesInsertion() {
			routing_trie = new Trie();

			for (int i = 0; i < route_table_len; i++) {
				routing_trie -> insert(&route_table_entries[i], routing_trie -> dummy, ntohl(route_table_entries[i].prefix),
																		 2147483648, ntohl(route_table_entries[i].prefix));
			}

			routing_trie->inorder(routing_trie -> dummy);
			cout << nodes << endl;
		}

		void print_route_entry(route_table_entry *route_entry) {
			cout << "Prefix: ";
			print_ip(route_entry -> prefix);
			cout << endl;

			cout << "Next hop: ";
			print_ip(route_entry -> next_hop);
			cout << endl;

			cout << "Mask: ";
			print_ip(route_entry -> mask);
			cout << endl;
			
		}

		route_table_entry *obtain_route_entry(TrieNode *current_node, uint32_t destination_ip) {
			route_table_entry *best_route = NULL;
			cout << "number of route entries is: " << current_node -> left_not_set_bit -> routesLen << endl;
			cout << "Destination ip is: " << endl;
			print_ip(destination_ip);
			cout << endl;
			if (current_node -> left_not_set_bit -> route_entries == NULL || current_node -> left_not_set_bit -> routesLen == 0) {
				return NULL;
			}
			for (int i = 0; i < current_node -> left_not_set_bit -> routesLen; i++) {
				
				print_ip(current_node -> left_not_set_bit -> route_entries[i].prefix);
				print_ip(current_node -> left_not_set_bit -> route_entries[i].next_hop);
				print_ip(current_node -> left_not_set_bit -> route_entries[i].mask);
				cout << current_node -> left_not_set_bit -> route_entries[i].interface;
				cout << endl;
				if ((htonl(destination_ip) & current_node -> left_not_set_bit -> route_entries[i].mask) == current_node -> left_not_set_bit -> route_entries[i].prefix) {
					if (best_route == NULL) {
						best_route = &current_node -> left_not_set_bit -> route_entries[i];
					}
				}
			}
			
			return best_route;
		}

		route_table_entry *getTheBestRoute(TrieNode *current_node, uint32_t destination_ip, uint32_t mask, uint32_t remaining) {
			int currentBit = destination_ip & mask;
			
			if (mask == 0) {
				cout << "MASCA S A GOLIT BAAAAAAAAAAAAAAAAAAAAAAAA " << endl;
				return obtain_route_entry(current_node, destination_ip);
			}

			if (currentBit != 0) {
				cout << 1 << " ";
				remaining -= mask;
				if (current_node -> right_set_bit != NULL) {
					return getTheBestRoute(current_node -> right_set_bit, destination_ip, mask >> 1, remaining);
				}
				cout << " Right side not available ";
				if (current_node -> left_not_set_bit != NULL) {
					return getTheBestRoute(current_node -> left_not_set_bit, destination_ip, mask >> 1, remaining);
				} else {
					return NULL;
				}
			} else {
				cout << 0 << " ";
				if (current_node -> left_not_set_bit != NULL) {
					return getTheBestRoute(current_node -> left_not_set_bit, destination_ip, mask >> 1, remaining);
				} else {
					return NULL;
				}
			} 
		}
};


int addresses_comparator(const void *entry1, const void *entry2) {
	struct route_table_entry *new_entry1 = (struct route_table_entry *) entry1;
	struct route_table_entry *new_entry2 = (struct route_table_entry *) entry2;

	if (new_entry1 -> prefix != new_entry2 -> prefix) {
		return (new_entry2 -> prefix - new_entry1 -> prefix);
	}
	if (new_entry1 -> prefix == new_entry2 -> prefix) {
		return (new_entry2 -> mask - new_entry1 -> mask);
	}

	return 0;
}

class Comparer {
	private:
		uint16_t header_type;
		Router *router;
	
	public:
		void set_header_type(uint16_t header_type) {
			this -> header_type = header_type;
		}

		void setRouter(Router *router) {
			this -> router = router;
		}

		bool compare_header_types() {

			if (header_type == ntohs(ETHERTYPE_IP)) {
				cout << "Handle IP Type" << endl;
				router -> handleIpType();
				return true;
			} else if (header_type == ntohs(ETHERTYPE_ARP)) {
				cout << "Handle ARP Type" << endl;
				router -> handleArpType();
				return true;
			} else {
				return false;
			}

			return true;
		}


};


int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_LEN];
	
	// Do not modify this line
	init(argc - 2, argv + 2);

	Comparer *comparer = new Comparer();
	Router *router = new Router();
	queued queue = queue_create();

	router -> setQueue(queue);
	comparer -> setRouter(router);
	router -> setRTable(argv[1]);

	/* route table initialization */

	route_table_entry *rtable_entries = (route_table_entry *) malloc(sizeof(route_table_entry) * 1000000);
	if (rtable_entries == NULL) {
		return 0;
	}

	int rtableLen = read_rtable(argv[1], rtable_entries);

	router -> setRTableLen(rtableLen);
	router -> setRouteTableEntries(rtable_entries);
	router -> setQueueLen();

	// /* arp cache initialization */
	arp_table_list *arp_cache = new arp_table_list();
	arp_cache -> arp_table_len = 0;
	arp_cache -> head = NULL; 

	router -> setArpCache(arp_cache);

	// route table sorting 

	qsort((void *)rtable_entries, rtableLen, sizeof(struct route_table_entry), addresses_comparator);
	router -> setRouteTableEntries(rtable_entries);
	router -> prefixesInsertion();

	while (1) {

		/* Note that packets received are in network order,
		any header field which has more than 1 byte will need to be conerted to
		host order. For example, ntohs(eth_hdr->ether_type). The oposite is needed when
		sending a packet on the link, */

		int interface;
		size_t len;

		interface = recv_from_any_link(buf, &len);
		DIE(interface < 0, "recv_from_any_links");

		struct ether_header *eth_hdr = (struct ether_header *) buf;
		comparer -> set_header_type(eth_hdr -> ether_type);
		router -> setBufferLen(len);
		router -> setNewPacket(buf);
		router -> setComingInterface(interface);
		cout << "Salut " << interface << endl;
		comparer -> compare_header_types();

	}
}

