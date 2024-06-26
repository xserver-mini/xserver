
#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#define _CRT_SECURE_NO_WARNINGS
#define ssize_t size_t

#else

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>

#endif

#include "socket_os.h"
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>


#define MAX_SOCKET_P 18

#define SOCKET_DATA 0
#define SOCKET_CLOSE 1
#define SOCKET_OPEN 2
#define SOCKET_ACCEPT 3
#define SOCKET_ERR 4
#define SOCKET_EXIT 5
#define SOCKET_UDP 6
#define SOCKET_WARNING 7

#define PROTOCOL_UDP 1
#define PROTOCOL_UDPv6 2


//skynet socket. https://github.com/cloudwu/skynet
struct socket_message {
	int id;
	uintptr_t opaque;
	int ud;	// for accept, ud is new connection id ; for data, ud is size of data 
	char* data;
};

struct socket_udp_address
{
	uint8_t type;
	uint16_t port;
	char address[16];
};

struct socket_object_interface {
	void* (*buffer)(void*);
	int (*size)(void*);
	void (*free)(void*);
};


#define MAX_INFO 128
// MAX_SOCKET will be 2^MAX_SOCKET_P
#define MAX_EVENT 64
// #define MIN_READ_BUFFER 64
#define MIN_READ_BUFFER 128
#define SOCKET_TYPE_INVALID 0
#define SOCKET_TYPE_RESERVE 1
#define SOCKET_TYPE_PLISTEN 2
#define SOCKET_TYPE_LISTEN 3
#define SOCKET_TYPE_CONNECTING 4
#define SOCKET_TYPE_CONNECTED 5
#define SOCKET_TYPE_HALFCLOSE 6
#define SOCKET_TYPE_PACCEPT 7
#define SOCKET_TYPE_BIND 8

#define MAX_SOCKET (1<<MAX_SOCKET_P)

#define PRIORITY_HIGH 0
#define PRIORITY_LOW 1

#define HASH_ID(id) (((unsigned)id) % MAX_SOCKET)
#define ID_TAG16(id) ((id>>MAX_SOCKET_P) & 0xffff)

#define PROTOCOL_TCP 0
#define PROTOCOL_UDP 1
#define PROTOCOL_UDPv6 2
#define PROTOCOL_UNKNOWN 255

#define UDP_ADDRESS_SIZE 19	// ipv6 128bit + port 16bit + 1 byte type

#define MAX_UDP_PACKAGE 65535

// EAGAIN and EWOULDBLOCK may be not the same value.
#if (EAGAIN != EWOULDBLOCK)
#define AGAIN_WOULDBLOCK EAGAIN : case EWOULDBLOCK
#else
#define AGAIN_WOULDBLOCK EAGAIN
#endif

#define WARNING_SIZE (1024*1024)

struct write_buffer {
	struct write_buffer * next;
	void *buffer;
	char *ptr;
	int sz;
	bool userobject;
	uint8_t udp_address[UDP_ADDRESS_SIZE];
};

#define SIZEOF_TCPBUFFER (offsetof(struct write_buffer, udp_address[0]))
#define SIZEOF_UDPBUFFER (sizeof(struct write_buffer))

struct wb_list {
	struct write_buffer * head;
	struct write_buffer * tail;
};

struct socket_stat {
	uint64_t rtime;
	uint64_t wtime;
	uint64_t read;
	uint64_t write;
};

struct socket {
	uintptr_t opaque;
	struct wb_list high;
	struct wb_list low;
	int64_t wb_size;
	struct socket_stat stat;
	int fd;
	int id;
	uint8_t protocol;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	volatile long sending;
	long type;
	long udpconnecting;
#else
	volatile uint32_t sending;
	uint8_t type;
	uint16_t udpconnecting;
#endif
	int64_t warn_size;
	union {
		int size;
		uint8_t udp_address[UDP_ADDRESS_SIZE];
	} p;
	struct spinlock dw_lock;
	int dw_offset;
	const void * dw_buffer;
	size_t dw_size;
};

struct socket_server {
	volatile uint64_t time;
	int recvctrl_fd;
	int sendctrl_fd;
	int checkctrl;
	poll_fd event_fd;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	long alloc_id;
#else
	int alloc_id;
#endif
	int event_n;
	int event_index;
	struct socket_object_interface soi;
	struct event ev[MAX_EVENT];
	struct socket slot[MAX_SOCKET];
	char buffer[MAX_INFO];
	uint8_t udpbuffer[MAX_UDP_PACKAGE];
	fd_set rfds;
};

struct request_open {
	int id;
	int port;
	uintptr_t opaque;
	char host[1];
};

struct request_send {
	int id;
	int sz;
	char * buffer;
};

struct request_send_udp {
	struct request_send send;
	uint8_t address[UDP_ADDRESS_SIZE];
};

struct request_setudp {
	int id;
	uint8_t address[UDP_ADDRESS_SIZE];
};

struct request_close {
	int id;
	int shutdown;
	uintptr_t opaque;
};

struct request_listen {
	int id;
	int fd;
	uintptr_t opaque;
	char host[1];
};

struct request_bind {
	int id;
	int fd;
	uintptr_t opaque;
};

struct request_start {
	int id;
	uintptr_t opaque;
};

struct request_setopt {
	int id;
	int what;
	int value;
};

struct request_udp {
	int id;
	int fd;
	int family;
	uintptr_t opaque;
};

/*
	The first byte is TYPE

	S Start socket
	B Bind socket
	L Listen socket
	K Close socket
	O Connect to (Open)
	X Exit
	D Send package (high)
	P Send package (low)
	A Send UDP package
	T Set opt
	U Create UDP socket
	C set udp address
	Q query info
 */

struct request_package {
	uint8_t header[8];	// 6 bytes dummy
	union {
		char buffer[256];
		struct request_open open;
		struct request_send send;
		struct request_send_udp send_udp;
		struct request_close close;
		struct request_listen listen;
		struct request_bind bind;
		struct request_start start;
		struct request_setopt setopt;
		struct request_udp udp;
		struct request_setudp set_udp;
	} u;
	uint8_t dummy[256];
};

union sockaddr_all {
	struct sockaddr s;
	struct sockaddr_in v4;
	struct sockaddr_in6 v6;
};

struct send_object {
	void * buffer;
	int sz;
	void (*free_func)(void *);
};

 #define MALLOC malloc
 #define FREE free

struct socket_lock {
	struct spinlock *lock;
	int count;
};

static inline void
socket_lock_init(struct socket *s, struct socket_lock *sl) {
	sl->lock = &s->dw_lock;
	sl->count = 0;
}

static inline void
socket_lock(struct socket_lock *sl) {
	if (sl->count == 0) {
		spinlock_lock(sl->lock);
	}
	++sl->count;
}

static inline int
socket_trylock(struct socket_lock *sl) {
	if (sl->count == 0) {
		if (!spinlock_trylock(sl->lock))
			return 0;	// lock failed
	}
	++sl->count;
	return 1;
}

static inline void
socket_unlock(struct socket_lock *sl) {
	--sl->count;
	if (sl->count <= 0) {
		assert(sl->count == 0);
		spinlock_unlock(sl->lock);
	}
}

static inline bool
send_object_init(struct socket_server *ss, struct send_object *so, void *object, int sz) {
	if (sz < 0) {
		so->buffer = ss->soi.buffer(object);
		so->sz = ss->soi.size(object);
		so->free_func = ss->soi.free;
		return true;
	} else {
		so->buffer = object;
		so->sz = sz;
		so->free_func = FREE;
		return false;
	}
}

static inline void
write_buffer_free(struct socket_server *ss, struct write_buffer *wb) {
	if (wb->userobject) {
		ss->soi.free(wb->buffer);
	} else {
		FREE(wb->buffer);
	}
	FREE(wb);
}

static void
socket_keepalive(int fd) {
	int keepalive = 1;
	socket_setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive));
}

static int
reserve_id(struct socket_server *ss) {
	int i;
	for (i=0;i<MAX_SOCKET;i++) {
		int id = ATOM_INC(&(ss->alloc_id));
		if (id < 0) {
			id = ATOM_AND(&(ss->alloc_id), 0x7fffffff);
		}
		struct socket *s = &ss->slot[HASH_ID(id)];
		if (s->type == SOCKET_TYPE_INVALID) {
			if (ATOM_CAS(&s->type, SOCKET_TYPE_INVALID, SOCKET_TYPE_RESERVE)) {
				s->id = id;
				s->protocol = PROTOCOL_UNKNOWN;
				// socket_server_udp_connect may inc s->udpconncting directly (from other thread, before new_fd), 
				// so reset it to 0 here rather than in new_fd.
				s->udpconnecting = 0;
				s->fd = -1;
				return id;
			} else {
				// retry
				--i;
			}
		}
	}
	return -1;
}

static inline void
clear_wb_list(struct wb_list *list) {
	list->head = NULL;
	list->tail = NULL;
}

struct socket_server * 
socket_server_create(uint64_t time) {
	socket_start();
	int i;
	int fd[2];
	poll_fd efd = sp_create();
	if (sp_invalid(efd)) {
		fprintf(stderr, "socket-server: create event pool failed.\n");
		return NULL;
	}
	if (socket_pipe(fd)) {
		sp_release(efd);
		fprintf(stderr, "socket-server: create socket pair failed.\n");
		return NULL;
	}
	if (sp_add(efd, fd[0], NULL)) {
		// add recvctrl_fd to event poll
		fprintf(stderr, "socket-server: can't add server fd to event pool.\n");
		socket_close(fd[0]);
		socket_close(fd[1]);
		sp_release(efd);
		return NULL;
	}

	struct socket_server *ss = (struct socket_server*)MALLOC(sizeof(*ss));
	if (!ss) return 0;
	ss->time = time;
	ss->event_fd = efd;
	ss->recvctrl_fd = fd[0];
	ss->sendctrl_fd = fd[1];
	ss->checkctrl = 1;
	struct socket* s = 0;
	for (i=0; i < MAX_SOCKET; ++i) {
		s = &ss->slot[i];
		s->type = SOCKET_TYPE_INVALID;
		clear_wb_list(&s->high);
		clear_wb_list(&s->low);
		spinlock_init(&s->dw_lock);
	}
	ss->alloc_id = 0;
	ss->event_n = 0;
	ss->event_index = 0;
	memset(&ss->soi, 0, sizeof(ss->soi));
	FD_ZERO(&ss->rfds);
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#else
	assert(ss->recvctrl_fd < FD_SETSIZE);
#endif
	return ss;
}

void
socket_server_updatetime(struct socket_server *ss, uint64_t time) {
	ss->time = time;
}

static void
free_wb_list(struct socket_server *ss, struct wb_list *list) {
	struct write_buffer *wb = list->head;
	while (wb) {
		struct write_buffer *tmp = wb;
		wb = wb->next;
		write_buffer_free(ss, tmp);
	}
	list->head = NULL;
	list->tail = NULL;
}

static void
free_buffer(struct socket_server *ss, const void * buffer, int sz) {
	struct send_object so;
	send_object_init(ss, &so, (void *)buffer, sz);
	so.free_func((void *)buffer);
}

static void
force_close(struct socket_server *ss, struct socket *s, struct socket_lock *l, struct socket_message *result) {
	result->id = s->id;
	result->ud = 0;
	result->data = NULL;
	result->opaque = s->opaque;
	if (s->type == SOCKET_TYPE_INVALID) {
		return;
	}
	assert(s->type != SOCKET_TYPE_RESERVE);
	free_wb_list(ss,&s->high);
	free_wb_list(ss,&s->low);
	if (s->type != SOCKET_TYPE_PACCEPT && s->type != SOCKET_TYPE_PLISTEN) {
		sp_del(ss->event_fd, s->fd);
	}
	socket_lock(l);
	if (s->type != SOCKET_TYPE_BIND) {
		if (socket_close(s->fd) < 0) {
			perror("close socket:");
		}
	}
	s->type = SOCKET_TYPE_INVALID;
	if (s->dw_buffer) {
		free_buffer(ss, s->dw_buffer, (int)s->dw_size);
		s->dw_buffer = NULL;
	}
	socket_unlock(l);
}

void 
socket_server_release(struct socket_server *ss) {
	int i = 0;
	struct socket_message dummy;
	for (i=0;i<MAX_SOCKET;i++) {
		struct socket *s = &ss->slot[i];
		struct socket_lock l;
		socket_lock_init(s, &l);
		if (s->type != SOCKET_TYPE_RESERVE) {
			force_close(ss, s, &l, &dummy);
		}
		spinlock_destroy(&s->dw_lock);
	}
	socket_close(ss->sendctrl_fd);
	socket_close(ss->recvctrl_fd);
	sp_release(ss->event_fd);
	FREE(ss);
	socket_stop();
}

void
socket_server_close(struct socket_server* ss) {
	int i = 0;
	struct socket_message dummy;
	for (i = 0; i < MAX_SOCKET; i++) {
		struct socket* s = &ss->slot[i];
		struct socket_lock l;
		socket_lock_init(s, &l);
		if (s->type != SOCKET_TYPE_RESERVE) {
			force_close(ss, s, &l, &dummy);
		}
		spinlock_destroy(&s->dw_lock);
	}
	//socket_close(ss->sendctrl_fd);
	//socket_close(ss->recvctrl_fd);
	sp_release(ss->event_fd);
}

static inline void
check_wb_list(struct wb_list *s) {
	assert(s->head == NULL);
	assert(s->tail == NULL);
}

static struct socket *
new_fd(struct socket_server *ss, int id, int fd, int protocol, uintptr_t opaque, bool add) {
	struct socket * s = &ss->slot[HASH_ID(id)];
	assert(s->type == SOCKET_TYPE_RESERVE);

	if (add) {
		if (sp_add(ss->event_fd, fd, s)) {
			s->type = SOCKET_TYPE_INVALID;
			return NULL;
		}
	}
	s->id = id;
	s->fd = fd;
	s->sending = ID_TAG16(id) << 16 | 0;
	s->protocol = protocol;
	s->p.size = MIN_READ_BUFFER;
	s->opaque = opaque;
	s->wb_size = 0;
	s->warn_size = 0;
	check_wb_list(&s->high);
	check_wb_list(&s->low);
	s->dw_buffer = NULL;
	s->dw_size = 0;
	memset(&s->stat, 0, sizeof(s->stat));
	return s;
}

static inline void
stat_read(struct socket_server *ss, struct socket *s, int n) {
	s->stat.read += n;
	s->stat.rtime = ss->time;
}

static inline void
stat_write(struct socket_server *ss, struct socket *s, int n) {
	s->stat.write += n;
	s->stat.wtime = ss->time;
}

// return -1 when connecting
static int
open_socket(struct socket_server *ss, struct request_open * request, struct socket_message *result) {
	int id = request->id;
	result->opaque = request->opaque;
	result->id = id;
	result->ud = 0;
	result->data = NULL;
	struct socket *ns;
	int status;
	struct addrinfo ai_hints;
	struct addrinfo *ai_list = NULL;
	struct addrinfo *ai_ptr = NULL;
	char port[16];
	snprintf(port, sizeof(port), "%d", request->port);
	memset(&ai_hints, 0, sizeof( ai_hints ) );
	ai_hints.ai_family = AF_UNSPEC;
	ai_hints.ai_socktype = SOCK_STREAM;
	ai_hints.ai_protocol = IPPROTO_TCP;

	status = getaddrinfo( request->host, port, &ai_hints, &ai_list );
	do
	{
		if (status != 0) {
			result->data = (char*)gai_strerror(status);
			break;
		}
		int sock = -1;
		for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
			sock = (int)socket(ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
			if (sock < 0) {
				continue;
			}
			socket_keepalive(sock);
			sp_nonblocking(sock);
			status = socket_connect(sock, ai_ptr->ai_addr, (int)ai_ptr->ai_addrlen);
			if (status != 0 && errno != EINPROGRESS) {
				socket_close(sock);
				sock = -1;
				continue;
			}
			break;
		}

		if (sock < 0) {
			result->data = strerror(errno);
			break;
		}

		ns = new_fd(ss, id, sock, PROTOCOL_TCP, request->opaque, true);
		if (ns == NULL) {
			socket_close(sock);
			result->data = (char*)"reach skynet socket number limit";
			break;
		}

		if (status == 0) {
			ns->type = SOCKET_TYPE_CONNECTED;
			struct sockaddr* addr = ai_ptr->ai_addr;
			void* sin_addr = (ai_ptr->ai_family == AF_INET) ? (void*)&((struct sockaddr_in*)addr)->sin_addr : (void*)&((struct sockaddr_in6*)addr)->sin6_addr;
			if (inet_ntop(ai_ptr->ai_family, sin_addr, ss->buffer, sizeof(ss->buffer))) {
				result->data = ss->buffer;
			}
			freeaddrinfo(ai_list);
			return SOCKET_OPEN;
		}
		else {
			ns->type = SOCKET_TYPE_CONNECTING;
			sp_write(ss->event_fd, ns->fd, ns, true);
		}

		freeaddrinfo(ai_list);
		return -1;
	} while (false);

	freeaddrinfo( ai_list );
	ss->slot[HASH_ID(id)].type = SOCKET_TYPE_INVALID;
	return SOCKET_ERR;
}

static int
send_list_tcp(struct socket_server *ss, struct socket *s, struct wb_list *list, struct socket_lock *l, struct socket_message *result) {
	while (list->head) {
		struct write_buffer * tmp = list->head;
		for (;;) {
			ssize_t sz = socket_write(s->fd, tmp->ptr, tmp->sz);
			if (sz < 0) {
				switch(errno) {
				case EINTR:
					continue;
				case AGAIN_WOULDBLOCK:
					return -1;
				}
				force_close(ss,s,l,result);
				return SOCKET_CLOSE;
			}
			stat_write(ss,s,(int)sz);
			s->wb_size -= sz;
			if (sz != tmp->sz) {
				tmp->ptr += sz;
				tmp->sz -= (int)sz;
				return -1;
			}
			break;
		}
		list->head = tmp->next;
		write_buffer_free(ss,tmp);
	}
	list->tail = NULL;

	return -1;
}

static socklen_t
udp_socket_address(struct socket *s, const uint8_t udp_address[UDP_ADDRESS_SIZE], union sockaddr_all *sa) {
	int type = (uint8_t)udp_address[0];
	if (type != s->protocol)
		return 0;
	uint16_t port = 0;
	memcpy(&port, udp_address+1, sizeof(uint16_t));
	switch (s->protocol) {
	case PROTOCOL_UDP:
		memset(&sa->v4, 0, sizeof(sa->v4));
		sa->s.sa_family = AF_INET;
		sa->v4.sin_port = port;
		memcpy(&sa->v4.sin_addr, udp_address + 1 + sizeof(uint16_t), sizeof(sa->v4.sin_addr));	// ipv4 address is 32 bits
		return sizeof(sa->v4);
	case PROTOCOL_UDPv6:
		memset(&sa->v6, 0, sizeof(sa->v6));
		sa->s.sa_family = AF_INET6;
		sa->v6.sin6_port = port;
		memcpy(&sa->v6.sin6_addr, udp_address + 1 + sizeof(uint16_t), sizeof(sa->v6.sin6_addr)); // ipv6 address is 128 bits
		return sizeof(sa->v6);
	}
	return 0;
}

static void
drop_udp(struct socket_server *ss, struct socket *s, struct wb_list *list, struct write_buffer *tmp) {
	s->wb_size -= tmp->sz;
	list->head = tmp->next;
	if (list->head == NULL)
		list->tail = NULL;
	write_buffer_free(ss,tmp);
}

static int
send_list_udp(struct socket_server *ss, struct socket *s, struct wb_list *list, struct socket_message *result) {
	while (list->head) {
		struct write_buffer * tmp = list->head;
		union sockaddr_all sa;
		socklen_t sasz = udp_socket_address(s, tmp->udp_address, &sa);
		if (sasz == 0) {
			fprintf(stderr, "socket-server : udp (%d) type mismatch.\n", s->id);
			drop_udp(ss, s, list, tmp);
			return -1;
		}
		int err = sendto(s->fd, tmp->ptr, tmp->sz, 0, &sa.s, sasz);
		if (err < 0) {
			switch(errno) {
			case EINTR:
			case AGAIN_WOULDBLOCK:
				return -1;
			}
			fprintf(stderr, "socket-server : udp (%d) sendto error (%d)%s.\n",s->id, errno, strerror(errno));
			drop_udp(ss, s, list, tmp);
			return -1;
		}
		stat_write(ss,s,tmp->sz);
		s->wb_size -= tmp->sz;
		list->head = tmp->next;
		write_buffer_free(ss,tmp);
	}
	list->tail = NULL;

	return -1;
}

static int
send_list(struct socket_server *ss, struct socket *s, struct wb_list *list, struct socket_lock *l, struct socket_message *result) {
	if (s->protocol == PROTOCOL_TCP) {
		return send_list_tcp(ss, s, list, l, result);
	} else {
		return send_list_udp(ss, s, list, result);
	}
}

static inline int
list_uncomplete(struct wb_list *s) {
	struct write_buffer *wb = s->head;
	if (wb == NULL)
		return 0;
	
	return (void *)wb->ptr != wb->buffer;
}

static void
raise_uncomplete(struct socket * s) {
	struct wb_list *low = &s->low;
	struct write_buffer *tmp = low->head;
	low->head = tmp->next;
	if (low->head == NULL) {
		low->tail = NULL;
	}

	// move head of low list (tmp) to the empty high list
	struct wb_list *high = &s->high;
	assert(high->head == NULL);

	tmp->next = NULL;
	high->head = high->tail = tmp;
}

static inline int
send_buffer_empty(struct socket *s) {
	return (s->high.head == NULL && s->low.head == NULL);
}

/*
	Each socket has two write buffer list, high priority and low priority.

	1. send high list as far as possible.
	2. If high list is empty, try to send low list.
	3. If low list head is uncomplete (send a part before), move the head of low list to empty high list (call raise_uncomplete) .
	4. If two lists are both empty, turn off the event. (call check_close)
 */
static int
send_buffer_(struct socket_server *ss, struct socket *s, struct socket_lock *l, struct socket_message *result) {
	assert(!list_uncomplete(&s->low));
	// step 1
	if (send_list(ss,s,&s->high,l,result) == SOCKET_CLOSE) {
		return SOCKET_CLOSE;
	}
	if (s->high.head == NULL) {
		// step 2
		if (s->low.head != NULL) {
			if (send_list(ss,s,&s->low,l,result) == SOCKET_CLOSE) {
				return SOCKET_CLOSE;
			}
			// step 3
			if (list_uncomplete(&s->low)) {
				raise_uncomplete(s);
				return -1;
			}
			if (s->low.head)
				return -1;
		} 
		// step 4
		assert(send_buffer_empty(s) && s->wb_size == 0);
		sp_write(ss->event_fd, s->fd, s, false);			

		if (s->type == SOCKET_TYPE_HALFCLOSE) {
			force_close(ss, s, l, result);
			return SOCKET_CLOSE;
		}
		if(s->warn_size > 0){
			s->warn_size = 0;
			result->opaque = s->opaque;
			result->id = s->id;
			result->ud = 0;
			result->data = NULL;
			return SOCKET_WARNING;
		}
	}

	return -1;
}

static int
send_buffer(struct socket_server *ss, struct socket *s, struct socket_lock *l, struct socket_message *result) {
	if (!socket_trylock(l))
		return -1;	// blocked by direct write, send later.
	if (s->dw_buffer) {
		// add direct write buffer before high.head
		struct write_buffer* buf = (struct write_buffer*)MALLOC(SIZEOF_TCPBUFFER);
		if (!buf)
		{
			return -1;
		}
		struct send_object so;
		buf->userobject = send_object_init(ss, &so, (void *)s->dw_buffer, (int)s->dw_size);
		buf->ptr = (char*)so.buffer+s->dw_offset;
		buf->sz = so.sz - s->dw_offset;
		buf->buffer = (void *)s->dw_buffer;
		s->wb_size+=buf->sz;
		if (s->high.head == NULL) {
			s->high.head = s->high.tail = buf;
			buf->next = NULL;
		} else {
			buf->next = s->high.head;
			s->high.head = buf;
		}
		s->dw_buffer = NULL;
	}
	int r = send_buffer_(ss,s,l,result);
	socket_unlock(l);

	return r;
}

static struct write_buffer *
append_sendbuffer_(struct socket_server *ss, struct wb_list *s, struct request_send * request, int size) {
	struct write_buffer * buf = (struct write_buffer*)MALLOC(size);
	if (!buf)
	{
		return 0;
	}
	struct send_object so;
	buf->userobject = send_object_init(ss, &so, request->buffer, request->sz);
	buf->ptr = (char*)so.buffer;
	buf->sz = so.sz;
	buf->buffer = request->buffer;
	buf->next = NULL;
	if (s->head == NULL) {
		s->head = s->tail = buf;
	} else {
		assert(s->tail != NULL);
		assert(s->tail->next == NULL);
		s->tail->next = buf;
		s->tail = buf;
	}
	return buf;
}

static inline void
append_sendbuffer_udp(struct socket_server *ss, struct socket *s, int priority, struct request_send * request, const uint8_t udp_address[UDP_ADDRESS_SIZE]) {
	struct wb_list *wl = (priority == PRIORITY_HIGH) ? &s->high : &s->low;
	struct write_buffer *buf = append_sendbuffer_(ss, wl, request, (int)SIZEOF_UDPBUFFER);
	memcpy(buf->udp_address, udp_address, UDP_ADDRESS_SIZE);
	s->wb_size += buf->sz;
}

static inline void
append_sendbuffer(struct socket_server *ss, struct socket *s, struct request_send * request) {
	struct write_buffer *buf = append_sendbuffer_(ss, &s->high, request, (int)SIZEOF_TCPBUFFER);
	s->wb_size += buf->sz;
}

static inline void
append_sendbuffer_low(struct socket_server *ss,struct socket *s, struct request_send * request) {
	struct write_buffer *buf = append_sendbuffer_(ss, &s->low, request, (int)SIZEOF_TCPBUFFER);
	s->wb_size += buf->sz;
}


/*
	When send a package , we can assign the priority : PRIORITY_HIGH or PRIORITY_LOW

	If socket buffer is empty, write to fd directly.
		If write a part, append the rest part to high list. (Even priority is PRIORITY_LOW)
	Else append package to high (PRIORITY_HIGH) or low (PRIORITY_LOW) list.
 */
static int
send_socket(struct socket_server *ss, struct request_send * request, struct socket_message *result, int priority, const uint8_t *udp_address) {
	int id = request->id;
	struct socket * s = &ss->slot[HASH_ID(id)];
	struct send_object so;
	send_object_init(ss, &so, request->buffer, request->sz);
	if (s->type == SOCKET_TYPE_INVALID || s->id != id 
		|| s->type == SOCKET_TYPE_HALFCLOSE
		|| s->type == SOCKET_TYPE_PACCEPT) {
		so.free_func(request->buffer);
		return -1;
	}
	if (s->type == SOCKET_TYPE_PLISTEN || s->type == SOCKET_TYPE_LISTEN) {
		fprintf(stderr, "socket-server: write to listen fd %d.\n", id);
		so.free_func(request->buffer);
		return -1;
	}
	if (send_buffer_empty(s) && s->type == SOCKET_TYPE_CONNECTED) {
		if (s->protocol == PROTOCOL_TCP) {
			append_sendbuffer(ss, s, request);	// add to high priority list, even priority == PRIORITY_LOW
		} else {
			// udp
			if (udp_address == NULL) {
				udp_address = s->p.udp_address;
			}
			union sockaddr_all sa;
			socklen_t sasz = udp_socket_address(s, udp_address, &sa);
			if (sasz == 0) {
				// udp type mismatch, just drop it.
				fprintf(stderr, "socket-server: udp socket (%d) type mistach.\n", id);
				so.free_func(request->buffer);
				return -1;
			}
			int n = sendto(s->fd, (char*)so.buffer, so.sz, 0, &sa.s, sasz);
			if (n != so.sz) {
				append_sendbuffer_udp(ss,s,priority,request,udp_address);
			} else {
				stat_write(ss,s,n);
				so.free_func(request->buffer);
				return -1;
			}
		}
		sp_write(ss->event_fd, s->fd, s, true);
	} else {
		if (s->protocol == PROTOCOL_TCP) {
			if (priority == PRIORITY_LOW) {
				append_sendbuffer_low(ss, s, request);
			} else {
				append_sendbuffer(ss, s, request);
			}
		} else {
			if (udp_address == NULL) {
				udp_address = s->p.udp_address;
			}
			append_sendbuffer_udp(ss,s,priority,request,udp_address);
		}
	}
	if (s->wb_size >= WARNING_SIZE && s->wb_size >= s->warn_size) {
		s->warn_size = s->warn_size == 0 ? WARNING_SIZE * 2 : s->warn_size * 2;
		result->opaque = s->opaque;
		result->id = s->id;
		result->ud = (int)(s->wb_size % 1024 == 0 ? s->wb_size / 1024 : s->wb_size / 1024 + 1);
		result->data = NULL;
		if (result->ud <= 0) {
			result->ud = 1;
		}
		return SOCKET_WARNING;
	}
	s->warn_size = 1;
	result->opaque = s->opaque;
	result->id = s->id;
	result->ud = 1;
	result->data = NULL;
	return SOCKET_WARNING;
}

static int
listen_socket(struct socket_server *ss, struct request_listen * request, struct socket_message *result) {
	int id = request->id;
	int listen_fd = request->fd;
	struct socket *s = new_fd(ss, id, listen_fd, PROTOCOL_TCP, request->opaque, false);
	if (s == NULL) {
		goto _failed;
	}
	s->type = SOCKET_TYPE_PLISTEN;
	return -1;
_failed:
	socket_close(listen_fd);
	result->opaque = request->opaque;
	result->id = id;
	result->ud = 0;
	result->data = (char*)"reach skynet socket number limit";
	ss->slot[HASH_ID(id)].type = SOCKET_TYPE_INVALID;

	return SOCKET_ERR;
}

static inline int
nomore_sending_data(struct socket *s) {
	return send_buffer_empty(s) && s->dw_buffer == NULL && (s->sending & 0xffff) == 0;
}

static int
close_socket(struct socket_server *ss, struct request_close *request, struct socket_message *result) {
	int id = request->id;
	struct socket * s = &ss->slot[HASH_ID(id)];
	if (s->type == SOCKET_TYPE_INVALID || s->id != id) {
		result->id = id;
		result->opaque = request->opaque;
		result->ud = 0;
		result->data = NULL;
		return SOCKET_CLOSE;
	}
	struct socket_lock l;
	socket_lock_init(s, &l);
	if (!nomore_sending_data(s)) {
		int type = send_buffer(ss,s,&l,result);
		// type : -1 or SOCKET_WARNING or SOCKET_CLOSE, SOCKET_WARNING means nomore_sending_data
		if (type != -1 && type != SOCKET_WARNING)
			return type;
	}
	if (request->shutdown || nomore_sending_data(s)) {
		force_close(ss,s,&l,result);
		result->id = id;
		result->opaque = request->opaque;
		return SOCKET_CLOSE;
	}
	s->type = SOCKET_TYPE_HALFCLOSE;

	return -1;
}

static int
bind_socket(struct socket_server *ss, struct request_bind *request, struct socket_message *result) {
	int id = request->id;
	result->id = id;
	result->opaque = request->opaque;
	result->ud = 0;
	struct socket *s = new_fd(ss, id, request->fd, PROTOCOL_TCP, request->opaque, true);
	if (s == NULL) {
		result->data = (char*)"reach skynet socket number limit";
		return SOCKET_ERR;
	}
	sp_nonblocking(request->fd);
	s->type = SOCKET_TYPE_BIND;
	result->data = (char*)"binding";
	return SOCKET_OPEN;
}

static int
start_socket(struct socket_server *ss, struct request_start *request, struct socket_message *result) {
	int id = request->id;
	result->id = id;
	result->opaque = request->opaque;
	result->ud = 0;
	result->data = NULL;
	struct socket *s = &ss->slot[HASH_ID(id)];
	if (s->type == SOCKET_TYPE_INVALID || s->id !=id) {
		result->data = (char*)"invalid socket";
		return SOCKET_ERR;
	}
	struct socket_lock l;
	socket_lock_init(s, &l);
	if (s->type == SOCKET_TYPE_PACCEPT || s->type == SOCKET_TYPE_PLISTEN) {
		if (sp_add(ss->event_fd, s->fd, s)) {
			force_close(ss, s, &l, result);
			result->data = strerror(errno);
			return SOCKET_ERR;
		}
		s->type = (s->type == SOCKET_TYPE_PACCEPT) ? SOCKET_TYPE_CONNECTED : SOCKET_TYPE_LISTEN;
		s->opaque = request->opaque;
		result->data = (char*)"start";
		return SOCKET_OPEN;
	} else if (s->type == SOCKET_TYPE_CONNECTED) {
		// todo: maybe we should send a message SOCKET_TRANSFER to s->opaque
		s->opaque = request->opaque;
		result->data = (char*)"transfer";
		return SOCKET_OPEN;
	}
	// if s->type == SOCKET_TYPE_HALFCLOSE , SOCKET_CLOSE message will send later
	return -1;
}

static void
setopt_socket(struct socket_server *ss, struct request_setopt *request) {
	int id = request->id;
	struct socket *s = &ss->slot[HASH_ID(id)];
	if (s->type == SOCKET_TYPE_INVALID || s->id !=id) {
		return;
	}
	int v = request->value;
	socket_setsockopt(s->fd, IPPROTO_TCP, request->what, &v, sizeof(v));
}

static void
block_readpipe(int pipefd, void *buffer, int sz) {
	for (;;) {
		int n = socket_read(pipefd, buffer, sz);
		if (n<0) {
			if (errno == EINTR)
				continue;
			fprintf(stderr, "socket-server : read pipe error %s.\n",strerror(errno));
			return;
		}
		// must atomic read from a pipe
		assert(n == sz);
		return;
	}
}

static int
has_cmd(struct socket_server *ss) {
	struct timeval tv = {0,0};
	FD_SET(ss->recvctrl_fd, &ss->rfds);
	int retval = select(ss->recvctrl_fd+1, &ss->rfds, NULL, NULL, &tv);
	if (retval == 1) {
		return 1;
	}
	return 0;
}

static void
add_udp_socket(struct socket_server *ss, struct request_udp *udp) {
	int id = udp->id;
	int protocol;
	if (udp->family == AF_INET6) {
		protocol = PROTOCOL_UDPv6;
	} else {
		protocol = PROTOCOL_UDP;
	}
	struct socket *ns = new_fd(ss, id, udp->fd, protocol, udp->opaque, true);
	if (ns == NULL) {
		socket_close(udp->fd);
		ss->slot[HASH_ID(id)].type = SOCKET_TYPE_INVALID;
		return;
	}
	ns->type = SOCKET_TYPE_CONNECTED;
	memset(ns->p.udp_address, 0, sizeof(ns->p.udp_address));
}

static int
set_udp_address(struct socket_server *ss, struct request_setudp *request, struct socket_message *result) {
	int id = request->id;
	struct socket *s = &ss->slot[HASH_ID(id)];
	if (s->type == SOCKET_TYPE_INVALID || s->id !=id) {
		return -1;
	}
	int type = request->address[0];
	if (type != s->protocol) {
		// protocol mismatch
		result->opaque = s->opaque;
		result->id = s->id;
		result->ud = 0;
		result->data = (char*)"protocol mismatch";

		return SOCKET_ERR;
	}
	if (type == PROTOCOL_UDP) {
		memcpy(s->p.udp_address, request->address, 1+2+4);	// 1 type, 2 port, 4 ipv4
	} else {
		memcpy(s->p.udp_address, request->address, 1+2+16);	// 1 type, 2 port, 16 ipv6
	}
	ATOM_DEC(&s->udpconnecting);
	return -1;
}

static inline void
inc_sending_ref(struct socket *s, int id) {
	if (s->protocol != PROTOCOL_TCP)
		return;
	for (;;) {
		uint32_t sending = s->sending;
		if ((sending >> 16) == ID_TAG16(id)) {
			if ((sending & 0xffff) == 0xffff) {
				// s->sending may overflow (rarely), so busy waiting here for socket thread dec it. see issue #794
				continue;
			}
			// inc sending only matching the same socket id
			if (ATOM_CAS((long*)&s->sending, sending, sending + 1))
				return;
			// atom inc failed, retry
		} else {
			// socket id changed, just return
			return;
		}
	}
}

static inline void
dec_sending_ref(struct socket_server *ss, int id) {
	struct socket * s = &ss->slot[HASH_ID(id)];
	// Notice: udp may inc sending while type == SOCKET_TYPE_RESERVE
	if (s->id == id && s->protocol == PROTOCOL_TCP) {
		assert((s->sending & 0xffff) != 0);
		ATOM_DEC((long*)&s->sending);
	}
}

// return type
static int
ctrl_cmd(struct socket_server *ss, struct socket_message *result) {
	int fd = ss->recvctrl_fd;
	// the length of message is one byte, so 256+8 buffer size is enough.
	uint8_t buffer[256];
	uint8_t header[2];
	block_readpipe(fd, header, sizeof(header));
	int type = header[0];
	int len = header[1];
	block_readpipe(fd, buffer, len);
	// ctrl command only exist in local fd, so don't worry about endian.
	// printf("[skynet-socket]ctrl_cmd type=%c\n", type);
	switch (type) {
	case 'S':
		return start_socket(ss,(struct request_start *)buffer, result);
	case 'B':
		return bind_socket(ss,(struct request_bind *)buffer, result);
	case 'L':
		return listen_socket(ss,(struct request_listen *)buffer, result);
	case 'K':
		return close_socket(ss,(struct request_close *)buffer, result);
	case 'O':
		return open_socket(ss, (struct request_open *)buffer, result);
	case 'X':
		result->opaque = 0;
		result->id = 0;
		result->ud = 0;
		result->data = NULL;
		return SOCKET_EXIT;
	case 'D':
	case 'P': {
		int priority = (type == 'D') ? PRIORITY_HIGH : PRIORITY_LOW;
		struct request_send * request = (struct request_send *) buffer;
		int ret = send_socket(ss, request, result, priority, NULL);
		//printf("ctrl_cmd ==<< id =%d\n", request->id);
		dec_sending_ref(ss, request->id);
		return ret;
	}
	case 'A': {
		struct request_send_udp * rsu = (struct request_send_udp *)buffer;
		return send_socket(ss, &rsu->send, result, PRIORITY_HIGH, rsu->address);
	}
	case 'C':
		return set_udp_address(ss, (struct request_setudp *)buffer, result);
	case 'T':
		setopt_socket(ss, (struct request_setopt *)buffer);
		return -1;
	case 'U':
		add_udp_socket(ss, (struct request_udp *)buffer);
		return -1;
	default:
		fprintf(stderr, "socket-server: Unknown ctrl %c.\n",type);
		return -1;
	};

	return -1;
}

// return -1 (ignore) when error
static int
forward_message_tcp(struct socket_server *ss, struct socket *s, struct socket_lock *l, struct socket_message * result) {
	int sz = s->p.size;
	char * buffer = (char*)MALLOC(sz + 1);
	//memset(buffer, 0, sz + 1);
	int n = (int)socket_read(s->fd, buffer, sz);
	if (n < 0) {
		FREE(buffer);
		switch(errno) {
		case EINTR:
			break;
		case AGAIN_WOULDBLOCK:
			fprintf(stderr, "socket-server: EAGAIN capture.\n");
			break;
		default:
			// close when error
			force_close(ss, s, l, result);
			result->data = strerror(errno);
			return SOCKET_ERR;
		}
		return -1;
	}
	if (n == 0) {
		FREE(buffer);
		force_close(ss, s, l, result);
		return SOCKET_CLOSE;
	}

	if (s->type == SOCKET_TYPE_HALFCLOSE) {
		// discard recv data
		FREE(buffer);
		return -1;
	}

	stat_read(ss,s,n);

	if (n == sz) {
		s->p.size *= 2;
	} else if (sz > MIN_READ_BUFFER && n*2 < sz) {
		s->p.size /= 2;
	}

	result->opaque = s->opaque;
	result->id = s->id;
	result->ud = n;
	result->data = buffer;
	return SOCKET_DATA;
}

static int
gen_udp_address(int protocol, union sockaddr_all *sa, uint8_t * udp_address) {
	int addrsz = 1;
	udp_address[0] = (uint8_t)protocol;
	if (protocol == PROTOCOL_UDP) {
		memcpy(udp_address+addrsz, &sa->v4.sin_port, sizeof(sa->v4.sin_port));
		addrsz += sizeof(sa->v4.sin_port);
		memcpy(udp_address+addrsz, &sa->v4.sin_addr, sizeof(sa->v4.sin_addr));
		addrsz += sizeof(sa->v4.sin_addr);
	} else {
		memcpy(udp_address+addrsz, &sa->v6.sin6_port, sizeof(sa->v6.sin6_port));
		addrsz += sizeof(sa->v6.sin6_port);
		memcpy(udp_address+addrsz, &sa->v6.sin6_addr, sizeof(sa->v6.sin6_addr));
		addrsz += sizeof(sa->v6.sin6_addr);
	}
	return addrsz;
}

static int
forward_message_udp(struct socket_server *ss, struct socket *s, struct socket_lock *l, struct socket_message * result) {
	union sockaddr_all sa = {0};
	socklen_t slen = sizeof(sa);
	int n = socket_recvfrom(s->fd, ss->udpbuffer, MAX_UDP_PACKAGE, 0, &sa.s, &slen);
	if (n<0) {
		switch(errno) {
		case EINTR:
		case AGAIN_WOULDBLOCK:
			break;
		default:
			// close when error
			force_close(ss, s, l, result);
			result->data = strerror(errno);
			return SOCKET_ERR;
		}
		return -1;
	}
	stat_read(ss,s,n);

	uint8_t* data = 0;
	if (slen == sizeof(sa.v4)) {
		if (s->protocol != PROTOCOL_UDP)
			return -1;

		//data = (uint8_t*)MALLOC(n + 1 + 2 + 4 + 1);
		//if (data) memset(data, 0, n + 1 + 2 + 4 + 1);
		data = (uint8_t*)MALLOC(n + 1 + 2 + 16 + 1);
		if (data) memset(data, 0, n + 1 + 2 + 16 + 1);
		gen_udp_address(PROTOCOL_UDP, &sa, data + n);
	} else {
		if (s->protocol != PROTOCOL_UDPv6)
			return -1;

		data = (uint8_t*)MALLOC(n + 1 + 2 + 16 + 1);
		if(data) memset(data, 0, n + 1 + 2 + 16 + 1);
		gen_udp_address(PROTOCOL_UDPv6, &sa, data + n);
	}
	if (data)
	{
		memcpy(data, ss->udpbuffer, n);
	}

	result->opaque = s->opaque;
	result->id = s->id;
	result->ud = n;
	result->data = (char *)data;

	return SOCKET_UDP;
}

static int
report_connect(struct socket_server *ss, struct socket *s, struct socket_lock *l, struct socket_message *result) {
	int error;
	socklen_t len = sizeof(error);  
	int code = socket_getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &error, &len);  
	if (code < 0 || error) {  
		force_close(ss,s,l, result);
		if (code >= 0)
			result->data = strerror(error);
		else
			result->data = strerror(errno);
		return SOCKET_ERR;
	} else {
		s->type = SOCKET_TYPE_CONNECTED;
		result->opaque = s->opaque;
		result->id = s->id;
		result->ud = 0;
		if (nomore_sending_data(s)) {
			sp_write(ss->event_fd, s->fd, s, false);
		}
		union sockaddr_all u = {0};
		socklen_t slen = sizeof(u);
		if (getpeername(s->fd, &u.s, &slen) == 0) {
			void * sin_addr = (u.s.sa_family == AF_INET) ? (void*)&u.v4.sin_addr : (void *)&u.v6.sin6_addr;
			if (inet_ntop(u.s.sa_family, sin_addr, ss->buffer, sizeof(ss->buffer))) {
				result->data = ss->buffer;
				return SOCKET_OPEN;
			}
		}
		result->data = NULL;
		return SOCKET_OPEN;
	}
}

static int
getname(union sockaddr_all *u, char *buffer, size_t sz) {
	char tmp[INET6_ADDRSTRLEN];
	void * sin_addr = (u->s.sa_family == AF_INET) ? (void*)&u->v4.sin_addr : (void *)&u->v6.sin6_addr;
	int sin_port = ntohs((u->s.sa_family == AF_INET) ? u->v4.sin_port : u->v6.sin6_port);
	if (inet_ntop(u->s.sa_family, sin_addr, tmp, sizeof(tmp))) {
		snprintf(buffer, sz, "%s:%d", tmp, sin_port);
		return 1;
	}
	buffer[0] = '\0';
	return 0;
}

// return 0 when failed, or -1 when file limit
static int
report_accept(struct socket_server *ss, struct socket *s, struct socket_message *result) {
	union sockaddr_all u = {0};
	socklen_t len = sizeof(u);
	int client_fd = (int)accept(s->fd, &u.s, &len);
	if (client_fd < 0) {
		if (errno == EMFILE || errno == ENFILE) {
			result->opaque = s->opaque;
			result->id = s->id;
			result->ud = 0;
			result->data = strerror(errno);
			return -1;
		} else {
			return 0;
		}
	}
	int id = reserve_id(ss);
	if (id < 0) {
		socket_close(client_fd);
		return 0;
	}
	socket_keepalive(client_fd);
	sp_nonblocking(client_fd);
	struct socket *ns = new_fd(ss, id, client_fd, PROTOCOL_TCP, s->opaque, false);
	if (ns == NULL) {
		socket_close(client_fd);
		return 0;
	}
	// accept new one connection
	stat_read(ss,s,1);

	ns->type = SOCKET_TYPE_PACCEPT;
	result->opaque = s->opaque;
	result->id = s->id;
	result->ud = id;
	result->data = NULL;

	if (getname(&u, ss->buffer, sizeof(ss->buffer))) {
		result->data = ss->buffer;
	}
	return 1;
}

static inline void 
clear_closed_event(struct socket_server *ss, struct socket_message * result, int type) {
	if (type == SOCKET_CLOSE || type == SOCKET_ERR) {
		int id = result->id;
		int i  = 0;
		for (i=ss->event_index; i<ss->event_n; i++) {
			struct event *e = &ss->ev[i];
			struct socket *s = (struct socket*)e->s;
			if (s) {
				if (s->type == SOCKET_TYPE_INVALID && s->id == id) {
					e->s = NULL;
					break;
				}
			}
		}
	}
}

// return type
int 
socket_server_poll(struct socket_server *ss, struct socket_message * result, int * more) {
	for (;;) {
		if (ss->checkctrl) {
			if (has_cmd(ss)) {
				// printf("[skynet-socket]socket_server_poll has_cmd\n");
				int type = ctrl_cmd(ss, result);
				if (type != -1) {
					clear_closed_event(ss, result, type);
					return type;
				} else
					continue;
			} else {
				ss->checkctrl = 0;
			}
		}
		if (ss->event_index == ss->event_n) {
			// printf("[skynet-socket]socket_server_poll sp_wait\n");
			ss->event_n = sp_wait(ss->event_fd, ss->ev, MAX_EVENT);
			ss->checkctrl = 1;
			if (more) {
				*more = 0;
			}
			ss->event_index = 0;
			if (ss->event_n <= 0) {
				ss->event_n = 0;
				if (errno == EINTR) {
					continue;
				}
				return -1;
			}
		}
		struct event *e = &ss->ev[ss->event_index++];
		struct socket *s = (struct socket*)e->s;
		if (s == NULL) {
			// dispatch pipe message at beginning
			continue;
		}
		struct socket_lock l;
		socket_lock_init(s, &l);
		switch (s->type) {
		case SOCKET_TYPE_CONNECTING:
			return report_connect(ss, s, &l, result);
		case SOCKET_TYPE_LISTEN: {
			int ok = report_accept(ss, s, result);
			if (ok > 0) {
				return SOCKET_ACCEPT;
			} if (ok < 0 ) {
				return SOCKET_ERR;
			}
			// when ok == 0, retry
			break;
		}
		case SOCKET_TYPE_INVALID:
			fprintf(stderr, "socket-server: invalid socket\n");
			break;
		default:
			if (e->read) {
				int type;
				if (s->protocol == PROTOCOL_TCP) {
					type = forward_message_tcp(ss, s, &l, result);
				} else {
					type = forward_message_udp(ss, s, &l, result);
					if (type == SOCKET_UDP) {
						// try read again
						--ss->event_index;
						return SOCKET_UDP;
					}
				}
				if (e->write && type != SOCKET_CLOSE && type != SOCKET_ERR) {
					// Try to dispatch write message next step if write flag set.
					e->read = false;
					--ss->event_index;
				}
				if (type == -1)
					break;				
				return type;
			}
			if (e->write) {
				int type = send_buffer(ss, s, &l, result);
				if (type == -1)
					break;
				return type;
			}
			if (e->error) {
				// close when error
				int error;
				socklen_t len = sizeof(error);  
				int code = socket_getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &error, &len);  
				const char * err = NULL;
				if (code < 0) {
					err = strerror(errno);
				} else if (error != 0) {
					err = strerror(error);
				} else {
					err = "Unknown error";
				}
				force_close(ss, s, &l, result);
				result->data = (char *)err;
				return SOCKET_ERR;
			}
			if(e->eof) {
				force_close(ss, s, &l, result);
				return SOCKET_CLOSE;
			}
			break;
		}
	}
}

static void
send_request(struct socket_server *ss, struct request_package *request, char type, int len) {
	request->header[6] = (uint8_t)type;
	request->header[7] = (uint8_t)len;
	for (;;) {
		ssize_t n = socket_write(ss->sendctrl_fd, &request->header[6], len+2);
		if (n<0) {
			if (errno != EINTR) {
				fprintf(stderr, "socket-server : send ctrl command error %s.\n", strerror(errno));
			}
			continue;
		}
		assert(n == len+2);
		return;
	}
}

static int open_request(struct socket_server *ss, struct request_package *req, uintptr_t opaque, const char *addr, int port) {
	int len = (int)strlen(addr);
	if (len + sizeof(req->u.open) >= 256) {
		fprintf(stderr, "socket-server : Invalid addr %s.\n",addr);
		return -1;
	}
	int id = reserve_id(ss);
	if (id < 0)
		return -1;
	req->u.open.opaque = opaque;
	req->u.open.id = id;
	req->u.open.port = port;
	memcpy(req->u.open.host, addr, len);
	req->u.open.host[len] = '\0';

	return len;
}

static inline int can_direct_write(struct socket *s, int id) {
	return s->id == id && nomore_sending_data(s) && s->type == SOCKET_TYPE_CONNECTED && s->udpconnecting == 0;
}

// return -1 when error, 0 when success
int socket_server_send(struct socket_server *ss, int id, const void * buffer, int sz) {
	struct socket * s = &ss->slot[HASH_ID(id)];
	if (s->id != id || s->type == SOCKET_TYPE_INVALID) {
		free_buffer(ss, buffer, sz);
		return -1;
	}

	struct socket_lock l;
	socket_lock_init(s, &l);

	if (can_direct_write(s,id) && socket_trylock(&l)) {
		// may be we can send directly, double check
		if (can_direct_write(s,id)) {
			// send directly
			struct send_object so;
			send_object_init(ss, &so, (void *)buffer, sz);
			ssize_t n;
			if (s->protocol == PROTOCOL_TCP) {
				n = socket_write(s->fd, so.buffer, so.sz);
			} else {
				union sockaddr_all sa;
				socklen_t sasz = udp_socket_address(s, s->p.udp_address, &sa);
				if (sasz == 0) {
					fprintf(stderr, "socket-server : set udp (%d) address first.\n", id);
					socket_unlock(&l);
					so.free_func((void *)buffer);
					return -1;
				}
				n = sendto(s->fd, (char*)so.buffer, so.sz, 0, &sa.s, sasz);
			}
			if (n<0) {
				// ignore error, let socket thread try again
				n = 0;
			}
			stat_write(ss, s, (int)n);
			if (n == so.sz) {
				// write done
				socket_unlock(&l);
				so.free_func((void *)buffer);
				return 0;
			}
			// write failed, put buffer into s->dw_* , and let socket thread send it. see send_buffer()
			s->dw_buffer = buffer;
			s->dw_size = sz;
			s->dw_offset = (int)n;
			sp_write(ss->event_fd, s->fd, s, true);

			socket_unlock(&l);
			return 0;
		}
		socket_unlock(&l);
	}
	//printf("socket_server_send ==>> id =%d\n", id);
	inc_sending_ref(s, id);

	struct request_package request = {0};
	request.u.send.id = id;
	request.u.send.sz = sz;
	request.u.send.buffer = (char *)buffer;

	send_request(ss, &request, 'D', sizeof(request.u.send));
	//return 0;
	return 1;
}

// return -1 when error, 0 when success
int 
socket_server_send_lowpriority(struct socket_server *ss, int id, const void * buffer, int sz) {
	struct socket * s = &ss->slot[HASH_ID(id)];
	if (s->id != id || s->type == SOCKET_TYPE_INVALID) {
		free_buffer(ss, buffer, sz);
		return -1;
	}

	inc_sending_ref(s, id);

	struct request_package request = {0};
	request.u.send.id = id;
	request.u.send.sz = sz;
	request.u.send.buffer = (char *)buffer;

	send_request(ss, &request, 'P', sizeof(request.u.send));
	return 0;
}

void
socket_server_exit(struct socket_server *ss) {
	struct request_package request;
	send_request(ss, &request, 'X', 0);
}


// return -1 means failed
// or return AF_INET or AF_INET6
static int
do_bind(const char *host, int port, int protocol, int *family) {
	int fd;
	int status;
	int reuse = 1;
	struct addrinfo ai_hints;
	struct addrinfo *ai_list = NULL;
	char portstr[16];
	if (host == NULL || host[0] == 0) {
		host = "0.0.0.0";	// INADDR_ANY
	}
	snprintf(portstr, sizeof(portstr), "%d", port);
	memset( &ai_hints, 0, sizeof( ai_hints ) );
	ai_hints.ai_family = AF_UNSPEC;
	if (protocol == IPPROTO_TCP) {
		ai_hints.ai_socktype = SOCK_STREAM;
	} else {
		assert(protocol == IPPROTO_UDP);
		ai_hints.ai_socktype = SOCK_DGRAM;
	}
	ai_hints.ai_protocol = protocol;

	status = getaddrinfo( host, portstr, &ai_hints, &ai_list );
	if ( status != 0 ) {
		return -1;
	}
	*family = ai_list->ai_family;
	fd = (int)socket(*family, ai_list->ai_socktype, 0);
	if (fd < 0) {
		goto _failed_fd;
	}
	if (socket_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(int))==-1) {
		goto _failed;
	}
	status = bind(fd, (struct sockaddr *)ai_list->ai_addr, (int)ai_list->ai_addrlen);
	if (status != 0)
		goto _failed;

	freeaddrinfo(ai_list);
	return fd;
_failed:
	socket_close(fd);
_failed_fd:
	freeaddrinfo(ai_list);
	return -1;
}

static int
do_listen(const char * host, int port, int backlog) {
	int family = 0;
	int listen_fd = do_bind(host, port, IPPROTO_TCP, &family);
	if (listen_fd < 0) {
		return -1;
	}
	if (listen(listen_fd, backlog) == -1) {
		socket_close(listen_fd);
		return -1;
	}
	return listen_fd;
}

void socket_server_userobject(struct socket_server *ss, struct socket_object_interface *soi) {
	ss->soi = *soi;
}

// UDP
int socket_server_udp(struct socket_server *ss, uintptr_t opaque, const char * addr, int port) {
	int fd;
	int family;
	if (port != 0 || addr != NULL) {
		// bind
		fd = do_bind(addr, port, IPPROTO_UDP, &family);
		if (fd < 0) {
			return -1;
		}
	} else {
		family = AF_INET;
		fd = (int)socket(family, SOCK_DGRAM, 0);
		if (fd < 0) {
			return -1;
		}
	}
	sp_nonblocking(fd);

	int id = reserve_id(ss);
	if (id < 0) {
		socket_close(fd);
		return -1;
	}
	struct request_package request = {0};
	request.u.udp.id = id;
	request.u.udp.fd = fd;
	request.u.udp.opaque = opaque;
	request.u.udp.family = family;

	send_request(ss, &request, 'U', sizeof(request.u.udp));	
	return id;
}

int 
socket_server_udp_send(struct socket_server *ss, int id, const struct socket_udp_address *addr, const void *buffer, int sz) {
	struct socket * s = &ss->slot[HASH_ID(id)];
	if (s->id != id || s->type == SOCKET_TYPE_INVALID) {
		free_buffer(ss, buffer, sz);
		return -1;
	}

	const uint8_t *udp_address = (const uint8_t *)addr;
	int addrsz;
	switch (udp_address[0]) {
	case PROTOCOL_UDP:
		addrsz = 1+2+4;		// 1 type, 2 port, 4 ipv4
		break;
	case PROTOCOL_UDPv6:
		addrsz = 1+2+16;	// 1 type, 2 port, 16 ipv6
		break;
	default:
		free_buffer(ss, buffer, sz);
		return -1;
	}

	struct socket_lock l;
	socket_lock_init(s, &l);

	if (can_direct_write(s,id) && socket_trylock(&l)) {
		// may be we can send directly, double check
		if (can_direct_write(s,id)) {
			// send directly
			struct send_object so;
			send_object_init(ss, &so, (void *)buffer, sz);
			union sockaddr_all sa;
			socklen_t sasz = udp_socket_address(s, udp_address, &sa);
			if (sasz == 0) {
				socket_unlock(&l);
				so.free_func((void *)buffer);
				return -1;
			}
			int n = sendto(s->fd, (char*)so.buffer, so.sz, 0, &sa.s, sasz);
			if (n >= 0) {
				// sendto succ
				stat_write(ss,s,n);
				socket_unlock(&l);
				so.free_func((void *)buffer);
				return 0;
			}
		}
		socket_unlock(&l);
		// let socket thread try again, udp doesn't care the order
	}

	struct request_package request = {0};
	request.u.send_udp.send.id = id;
	request.u.send_udp.send.sz = sz;
	request.u.send_udp.send.buffer = (char *)buffer;

	memcpy(request.u.send_udp.address, udp_address, addrsz);

	send_request(ss, &request, 'A', sizeof(request.u.send_udp.send)+addrsz);
	return 0;
}

int
socket_server_udp_connect(struct socket_server *ss, int id, const char * addr, int port) {
	struct socket * s = &ss->slot[HASH_ID(id)];
	if (s->id != id || s->type == SOCKET_TYPE_INVALID) {
		return -1;
	}
	struct socket_lock l;
	socket_lock_init(s, &l);
	socket_lock(&l);
	if (s->id != id || s->type == SOCKET_TYPE_INVALID) {
		socket_unlock(&l);
		return -1;
	}
	ATOM_INC(&s->udpconnecting);
	socket_unlock(&l);

	int status;
	struct addrinfo ai_hints;
	struct addrinfo *ai_list = NULL;
	char portstr[16];
	snprintf(portstr, sizeof(portstr), "%d", port);
	memset( &ai_hints, 0, sizeof( ai_hints ) );
	ai_hints.ai_family = AF_UNSPEC;
	ai_hints.ai_socktype = SOCK_DGRAM;
	ai_hints.ai_protocol = IPPROTO_UDP;

	status = getaddrinfo(addr, portstr, &ai_hints, &ai_list );
	if ( status != 0 ) {
		return -1;
	}
	struct request_package request = {0};
	request.u.set_udp.id = id;
	int protocol = 0;

	if (ai_list->ai_family == AF_INET) {
		protocol = PROTOCOL_UDP;
	} else if (ai_list->ai_family == AF_INET6) {
		protocol = PROTOCOL_UDPv6;
	} else {
		freeaddrinfo(ai_list);
		return -1;
	}
	int addrsz = gen_udp_address(protocol, (union sockaddr_all *)ai_list->ai_addr, request.u.set_udp.address);
	freeaddrinfo(ai_list);
	send_request(ss, &request, 'C', sizeof(request.u.set_udp) - sizeof(request.u.set_udp.address) +addrsz);
	return 0;
}

const struct socket_udp_address *
socket_server_udp_address(struct socket_server *ss, struct socket_message *msg, int *addrsz) {
	uint8_t * address = (uint8_t *)(msg->data + msg->ud);
	int type = address[0];
	switch(type) {
	case PROTOCOL_UDP:
		*addrsz = 1+2+4;
		break;
	case PROTOCOL_UDPv6:
		*addrsz = 1+2+16;
		break;
	default:
		return NULL;
	}
	return (const struct socket_udp_address *)address;
}

#ifdef __cplusplus
}
#endif


#include "opensocket.h"
#include <time.h>
#include <map>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#ifdef __cplusplus
extern "C" {
#endif

#include <winsock2.h> 
#include <process.h>
#include <ws2tcpip.h>

struct pthread_t_
{
	HANDLE thread_handle;
	DWORD  thread_id;
};
typedef struct pthread_t_ pthread_t;
typedef int pthread_attr_t;

typedef unsigned(__stdcall* routinefunc)(void*);
static int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine1) (void*), void* arg)
{
	int _intThreadId = 0;
	routinefunc start_routine = (routinefunc)start_routine1;
	(*thread).thread_handle = (HANDLE)_beginthreadex(NULL, 0, start_routine, arg, 0, (unsigned int*)&_intThreadId);
	(*thread).thread_id = _intThreadId;
	return (*thread).thread_handle == 0 ? errno : 0;
}

#ifdef __cplusplus
}
#endif

#else
#include <pthread.h>
#include<arpa/inet.h>
#endif

namespace open
{

OpenSocket::Msg::Msg()
	:type_(ESocketClose)
	, fd_(0)
	, uid_(0)
	, ud_(0)
	, buffer_(0)
	, size_(0)
	, option_(0)
{
}

OpenSocket::Msg::~Msg()
{
	if (buffer_)
	{
		FREE(buffer_);
		buffer_ = 0;
	}
}

OpenSocket::OpenSocket()
	:socket_server_(0)
{
	cb_ = 0;
	isRunning_ = false;
	isClose_ = true;
	socket_server_ = (void*)socket_server_create(time(NULL));
	assert(socket_server_);
}

OpenSocket::~OpenSocket()
{
	if (!isClose_)
	{
		if (isRunning_)
		{
			socket_server_close((struct socket_server*)socket_server_);
			isRunning_ = false;
			while (!isClose_)
			{
				Sleep(100);
			}
		}
	}
	isRunning_ = false;
	Sleep(300);
	if (socket_server_)
	{
		socket_server_release((struct socket_server*)socket_server_);
		socket_server_ = 0;
	}
}

bool OpenSocket::run(void (*cb)(const Msg*))
{
	if (!cb)
	{
		assert(false);
		return false;
	}
	if (isRunning_)
	{
		assert(false);
		return false;
	}
	cb_ = cb;
	pthread_t thread;
	int ret = pthread_create(&thread, NULL, &OpenSocket::ThreadSocket, this);
	if (ret != 0)
	{
		fprintf(stderr, "Create thread failed");
		return false;
	}
	int count = 0;
	while (!isRunning_)
	{
		OpenSocket::Sleep(1);
		if (++count > 5000)
		{
			assert(false);
			break;
		}
	}
	return true;
}

void* OpenSocket::ThreadSocket(void* p)
{
	OpenSocket* that = dynamic_cast<OpenSocket*>((OpenSocket*)p);
	if (!that)
	{
		assert(false);
		return 0;
	}
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#else
#ifdef _GNU_SOURCE
	pthread_setname_np(pthread_self(), "OpenSocket");
#else
	prctl(PR_SET_NAME, (unsigned long)"OpenSocket");
#endif
#endif
	that->isRunning_ = true;
	that->isClose_ = false;
	int r = 0;
	while (that->isRunning_)
	{
		r = that->poll();
		if (r == 0) break;
	}
	that->isClose_ = true;
	return 0;
}

void OpenSocket::forwardMsg(EMsgType type, bool padding, struct socket_message* result)
{
	if (!cb_) return;
	Msg* msg = new Msg;
	msg->type_ = type;
	msg->fd_ = result->id;
	msg->ud_ = result->ud;
	msg->uid_ = result->opaque;
	if (padding) {
		if (result->data) {
			int msg_sz = (int)strlen(result->data);
			if (msg_sz > 128) {
				msg_sz = 128;
			}
			msg->size_ = msg_sz + 1;
			msg->buffer_ = (char*)malloc(msg->size_);
			if (msg->buffer_)
			{
				memset(msg->buffer_, 0, msg->size_);
				memcpy(msg->buffer_, result->data, msg->size_);
			}
		}
		else {
			msg->buffer_ = NULL;
			msg->size_ = 0;
		}
	}
	else {
		msg->buffer_ = result->data;
		msg->size_ = result->ud;
		if (msg->type_ == ESocketUdp) {
			msg->option_ = msg->buffer_ + msg->ud_;
		}
		msg->ud_ = 0;
	}
	cb_(msg);
}

int OpenSocket::poll()
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	assert(ss);
	int more = 1;
	struct socket_message result;
	int type = socket_server_poll(ss, &result, &more);
	switch (type)
	{
	case SOCKET_EXIT:
		return 0;
	case SOCKET_DATA:
		forwardMsg(ESocketData, false, &result);
		break;
	case SOCKET_CLOSE:
		forwardMsg(ESocketClose, false, &result);
		break;
	case SOCKET_OPEN:
		forwardMsg(ESocketOpen, true, &result);
		break;
	case SOCKET_ERR:
		forwardMsg(ESocketError, true, &result);
		break;
	case SOCKET_ACCEPT:
		forwardMsg(ESocketAccept, true, &result);
		break;
	case SOCKET_UDP:
		forwardMsg(ESocketUdp, false, &result);
		break;
	case SOCKET_WARNING:
		forwardMsg(ESocketWarning, false, &result);
		break;
	default:
		if (type != -1) {
			fprintf(stderr, "Unknown socket message type %d.\n", type);
		}
		return -1;
	}
	if (more) {
		return -1;
	}
	return 1;
}

int OpenSocket::send(int fd, const void* buffer, int sz)
{
	char* sbuffer = (char*)malloc(sz);
	if (!sbuffer) return -1;
	memcpy(sbuffer, buffer, sz);
	struct socket_server* ss = (struct socket_server*)socket_server_;
	return socket_server_send(ss, fd, sbuffer, sz);
}

int OpenSocket::sendLowpriority(int fd, const void* buffer, int sz)
{
	char* sbuffer = (char*)malloc(sz);
	if (!sbuffer) return -1;
	memcpy(sbuffer, buffer, sz);
	struct socket_server* ss = (struct socket_server*)socket_server_;
	return socket_server_send_lowpriority(ss, fd, sbuffer, sz);
}

void OpenSocket::nodelay(int fd)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	struct request_package request = {0};
	request.u.setopt.id = fd;
	request.u.setopt.what = TCP_NODELAY;
	request.u.setopt.value = 1;
	send_request(ss, &request, 'T', sizeof(request.u.setopt));
}

int OpenSocket::listen(uintptr_t uid, const std::string& host, int port, int backlog)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	int fd = do_listen(host.c_str(), port, backlog);
	if (fd < 0) {
		return -1;
	}
	struct request_package request = {0};
	int id = reserve_id(ss);
	if (id < 0) {
		socket_close(fd);
		return id;
	}
	request.u.listen.opaque = uid;
	request.u.listen.id = id;
	request.u.listen.fd = fd;
	send_request(ss, &request, 'L', sizeof(request.u.listen));
	return id;
}

int OpenSocket::connect(uintptr_t uid, const std::string& host, int port)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	struct request_package request;
	int len = open_request(ss, &request, uid, host.c_str(), port);
	if (len < 0)
		return -1;
	send_request(ss, &request, 'O', sizeof(request.u.open) + len);
	return request.u.open.id;
}

int OpenSocket::bind(uintptr_t uid, int fd)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	struct request_package request = {0};
	int id = reserve_id(ss);
	if (id < 0)
		return -1;
	request.u.bind.opaque = uid;
	request.u.bind.id = id;
	request.u.bind.fd = fd;
	send_request(ss, &request, 'B', sizeof(request.u.bind));
	return id;
}

void OpenSocket::close(uintptr_t uid, int fd)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	struct request_package request = {0};
	request.u.close.id = fd;
	request.u.close.shutdown = 0;
	request.u.close.opaque = uid;
	send_request(ss, &request, 'K', sizeof(request.u.close));
}

void OpenSocket::shutdown(uintptr_t uid, int fd)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	struct request_package request = {0};
	request.u.close.id = fd;
	request.u.close.shutdown = 1;
	request.u.close.opaque = uid;
	send_request(ss, &request, 'K', sizeof(request.u.close));
}

void OpenSocket::start(uintptr_t uid, int fd)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	struct request_package request = {0};
	request.u.start.id = fd;
	request.u.start.opaque = uid;
	send_request(ss, &request, 'S', sizeof(request.u.start));
}

int OpenSocket::udp(uintptr_t uid, const char* addr, int port)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	return socket_server_udp(ss, uid, addr, port);
}

int OpenSocket::udpConnect(int fd, const char* addr, int port)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	return socket_server_udp_connect(ss, fd, addr, port);
}

int OpenSocket::udpSend(int fd, const char* address, const void* buffer, int sz)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	int size = sz;
	char* sbuffer = (char*)MALLOC(size);
	if (!sbuffer) return -1;
	memcpy(sbuffer, buffer, sz);
	return socket_server_udp_send(ss, fd, (const struct socket_udp_address*)address, sbuffer, sz);
}

int OpenSocket::UDPAddressToIpPort(const char* address, std::string& addr, int& po)
{
	if (!address) return -1;
	int type = address[0];
	int family;
	switch (type) {
	case PROTOCOL_UDP:
		family = AF_INET;
		break;
	case PROTOCOL_UDPv6:
		family = AF_INET6;
		break;
	default:
		return -1;
	}
	uint16_t port = 0;
	memcpy(&port, address + 1, sizeof(uint16_t));
	port = ntohs(port);
	const void* addrptr = address + 3;
	char strptr[256] = { 0 };
	if (!inet_ntop(family, addrptr, strptr, sizeof(strptr))) {
		return -1;
	}
	addr = strptr;
	po = port;
	return 0;
}

int OpenSocket::IpPortToUDPAddress(const char* addr, int port, uint8_t* udp_address, int udp_size) {
	if (udp_size < UDP_ADDRESS_SIZE)
	{
		assert(false);
		return -1;
	}
	int status;
	struct addrinfo ai_hints;
	struct addrinfo* ai_list = NULL;
	char portstr[16];
	snprintf(portstr, sizeof(portstr), "%d", port);
	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_UNSPEC;
	ai_hints.ai_socktype = SOCK_DGRAM;
	ai_hints.ai_protocol = IPPROTO_UDP;
	status = getaddrinfo(addr, portstr, &ai_hints, &ai_list);
	if (status != 0) {
		return -1;
	}
	int protocol = 0;
	if (ai_list->ai_family == AF_INET) {
		protocol = PROTOCOL_UDP;
	}
	else if (ai_list->ai_family == AF_INET6) {
		protocol = PROTOCOL_UDPv6;
	}
	else {
		freeaddrinfo(ai_list);
		return -1;
	}
	int addrsz = gen_udp_address(protocol, (union sockaddr_all*)ai_list->ai_addr, udp_address);
	freeaddrinfo(ai_list);
	return addrsz;
}

static int getname(union sockaddr_all* u, std::string& name) 
{
	char tmp[INET6_ADDRSTRLEN];
	void* sin_addr = (u->s.sa_family == AF_INET) ? (void*)&u->v4.sin_addr : (void*)&u->v6.sin6_addr;
	int sin_port = ntohs((u->s.sa_family == AF_INET) ? u->v4.sin_port : u->v6.sin6_port);
	char buffer[256] = {};
	if (inet_ntop(u->s.sa_family, sin_addr, tmp, sizeof(tmp))) {
		snprintf(buffer, sizeof(buffer), "%s:%d", tmp, sin_port);
		name = buffer;
		return 1;
	}
	else {
		name.clear();
		return 0;
	}
}

static int query_info(struct socket* s, OpenSocket::Info& info)
{
	union sockaddr_all u = { 0 };
	socklen_t slen = sizeof(u);
	switch (s->type) {
	case SOCKET_TYPE_BIND:
		info.type_ = OpenSocket::EInfoBing;
		info.name_.clear();
		break;
	case SOCKET_TYPE_LISTEN:
		info.type_ = OpenSocket::EInfoListen;
		if (getsockname(s->fd, &u.s, &slen) == 0) {
			getname(&u, info.name_);
		}
		break;
	case SOCKET_TYPE_CONNECTED:
		if (s->protocol == PROTOCOL_TCP) {
			info.type_ = OpenSocket::EInfoTcp;
			if (getpeername(s->fd, &u.s, &slen) == 0) {
				getname(&u, info.name_);
			}
		}
		else {
			info.type_ = OpenSocket::EInfoUdp;
			if (udp_socket_address(s, s->p.udp_address, &u)) {
				getname(&u, info.name_);
			}
		}
		break;
	default:
		return 0;
	}
	info.id_ = s->id;
	info.opaque_ = (uint64_t)s->opaque;
	info.read_ = s->stat.read;
	info.write_ = s->stat.write;
	info.rtime_ = s->stat.rtime;
	info.wtime_ = s->stat.wtime;
	info.wbuffer_ = s->wb_size;
	return 1;
}

static bool CheckIp(const char* ip)
{
	int a = 0, b = 0, c = 0, d = 0;
	int ret = sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d);
	if (ret == 4 && a >= 0 && a <= 255 && b >= 0 && b <= 255 && c >= 0 && c <= 255 && d >= 0 && d <= 255)
	{
		char temp[64] = { 0 };
		sprintf(temp, "%d.%d.%d.%d", a, b, c, d);
		if (strcmp(temp, ip) == 0)
			return true;
	}
	return false;
}

const std::string OpenSocket::DomainToIp(const std::string& domain)
{
	if (CheckIp(domain.c_str())) return domain;
	std::string ip;
	struct addrinfo* result = NULL;
	struct addrinfo hints = { 0 };
	hints.ai_family   = AF_INET;     //ipv4
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags    = AI_PASSIVE;  /* For wildcard IP address */
	//    hints.ai_protocol = 0;         /* Any protocol */
	hints.ai_protocol = IPPROTO_IP;
	int retval = getaddrinfo(domain.c_str(), NULL, &hints, &result);
	if (retval != 0)
		return ip;
	
	struct addrinfo* cur = result;
	struct sockaddr_in* addr_in = 0;
	char str[64] = { 0 };
	do {
		addr_in = (struct sockaddr_in*)cur->ai_addr;
		ip = inet_ntop(cur->ai_family, &addr_in->sin_addr, str, sizeof(str));
		if (!ip.empty()) break;
	} while ((cur = cur->ai_next));
	freeaddrinfo(result);
	return ip;
}

void OpenSocket::socketInfo(std::vector<Info>& vectInfo)
{
	struct socket_server* ss = (struct socket_server*)socket_server_;
	int i = 0;
	Info temp;
	vectInfo.clear();
	vectInfo.reserve(64);
	for (i = 0; i < MAX_SOCKET; i++) {
		struct socket* s = &ss->slot[i];
		int id = s->id;
		temp.clear();
		if (query_info(s, temp) && s->id == id) 
		{
			vectInfo.push_back(temp);
		}
	}
}

void OpenSocket::Sleep(int64_t milliSecond)
{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
	::Sleep((DWORD)milliSecond);
#else
	::usleep(milliSecond * 1000);
#endif
}

void OpenSocket::Start(void (*cb)(const Msg*))
{
	if (Instance_.isRunning())
	{
		assert(false);
		return;
	}
	Instance_.run(cb);
}

OpenSocket OpenSocket::Instance_;

};