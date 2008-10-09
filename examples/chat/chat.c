#include "bool.h"
#include "malloc.h"
#include "lists.h"
#include "stringBuffers.h"
#include "sockets.h"
#include "threading.h"

struct member {
    struct string_buffer *nick;
    struct writer *writer;
};

/*@
predicate member(struct member* mem)
  requires mem->nick |-> ?nick &*& mem->writer |-> ?writer &*& string_buffer(nick) &*& writer(writer) &*& malloc_block_member(mem);

lemma void member_distinct(struct member *m1, struct member *m2)
  requires member(m1) &*& member(m2);
  ensures member(m1) &*& member(m2) &*& m1 != m2;
{
  open member(m1);
  open member(m2);
  close member(m2);
  close member(m1);
}

lemma void foreach_member_not_contains(listval v, struct member *member)
  requires foreach(v, @member) &*& member(member);
  ensures foreach(v, @member) &*& member(member) &*& !contains(v, member);
{
  switch (v) {
    case nil:
    case cons(h, t):
      open foreach(v, @member);
      member_distinct(h, member);
      foreach_member_not_contains(t, member);
      close foreach(v, @member);
  }
}
@*/

struct room {
    struct list *members;
};

/*@
predicate room(struct room* r)
  requires r->members |-> ?list &*& list(list, ?v) &*& foreach(v, member) &*& malloc_block_room(r);
@*/

struct room *create_room()
  //@ requires emp;
  //@ ensures room(result);
{
    struct room *room = malloc(sizeof(struct room));
    struct list *members = create_list();
    room->members = members;
    //@ close foreach(nil, member);
    //@ close room(room);
    return room;
}

bool room_has_member(struct room *room, struct string_buffer *nick)
  //@ requires room(room) &*& string_buffer(nick);
  //@ ensures room(room) &*& string_buffer(nick);
{
    //@ open room(room);
    //@ assert foreach(?v, _);
    struct list *members = room->members;
    struct iter *iter = list_create_iter(members);
    bool hasMember = false;
    bool hasNext = iter_has_next(iter);
    //@ lengthPositive(v);
    while (hasNext && !hasMember)
      //@ invariant string_buffer(nick) &*& iter(iter, members, v, ?i) &*& foreach(v, @member) &*& hasNext==(i<length(v)) &*& 0<=i &*& i<= length(v);
    {
        struct member *member = iter_next(iter);
        //@ containsIth(v, i);
        //@ foreach_remove(v, member);
        //@ open member(member);
        struct string_buffer *memberNick = member->nick;
        hasMember = string_buffer_equals(memberNick, nick);
        //@ close member(member);
        //@ foreach_unremove(v, member);
        hasNext = iter_has_next(iter);
    }
    iter_dispose(iter);
    //@ close room(room);
    return hasMember;
}

void room_broadcast_message(struct room *room, struct string_buffer *senderNick, struct string_buffer *message)
  //@ requires room(room) &*& string_buffer(senderNick) &*& string_buffer(message);
  //@ ensures room(room) &*& string_buffer(senderNick) &*& string_buffer(message);
{
    //@ open room(room);
    //@ assert foreach(?v, _);
    struct list *members = room->members;
    struct iter *iter = list_create_iter(members);
    bool hasNext = iter_has_next(iter);
    //@ lengthPositive(v);
    while (hasNext)
      //@ invariant iter(iter, members, ?v, ?i) &*& foreach(v, @member) &*& string_buffer(senderNick) &*& string_buffer(message) &*& hasNext==(i<length(v)) &*& 0<=i &*& i<= length(v);
    {
        struct member *member = iter_next(iter);
        //@ containsIth(v, i);
        //@ foreach_remove(v, member);
        //@ open member(member);
        struct writer *memberWriter = member->writer;
        writer_write_string_buffer(memberWriter, senderNick);
        writer_write_string(memberWriter, " says: ");
        writer_write_string_buffer(memberWriter, message);
        writer_write_string(memberWriter, "\r\n");
        //@ close member(member);
        //@ foreach_unremove(v, member);
        hasNext = iter_has_next(iter);
    }    
    iter_dispose(iter);
    //@ close room(room);
}

void room_broadcast_goodbye_message(struct room *room, struct string_buffer *senderNick)
  //@ requires room(room) &*& string_buffer(senderNick);
  //@ ensures room(room) &*& string_buffer(senderNick);
{
    //@ open room(room);
    //@ assert foreach(?v, _);
    struct list *members = room->members;
    struct iter *iter = list_create_iter(members);
    bool hasNext = iter_has_next(iter);
    //@ lengthPositive(v);
    while (hasNext)
      //@ invariant iter(iter, members, ?v, ?i) &*& foreach(v, @member) &*& string_buffer(senderNick) &*& hasNext==(i<length(v)) &*& 0<=i &*& i<= length(v);
    {
        struct member *member = iter_next(iter);
        //@ containsIth(v, i);
        //@ foreach_remove(v, member);
        //@ open member(member);
        struct writer *memberWriter = member->writer;
        writer_write_string_buffer(memberWriter, senderNick);
        writer_write_string(memberWriter, " left the room.\r\n");
        //@ close member(member);
        //@ foreach_unremove(v, member);
        hasNext = iter_has_next(iter);
    }
    iter_dispose(iter);
    //@ close room(room);
}

void room_broadcast_join_message(struct room *room, struct string_buffer *senderNick)
  //@ requires room->members |-> ?list &*& list(list, ?v) &*& foreach(v, member) &*& string_buffer(senderNick);
  //@ ensures room->members |-> list &*& list(list, v) &*& foreach(v, member) &*& string_buffer(senderNick);
{
    //@ assert foreach(?v, _);
    struct list *members = room->members;
    struct iter *iter = list_create_iter(members);
    bool hasNext = iter_has_next(iter);
    //@ lengthPositive(v);
    while (hasNext)
      //@ invariant iter(iter, members, v, ?i) &*& foreach(v, @member) &*& string_buffer(senderNick) &*& hasNext==(i<length(v)) &*& 0<=i &*& i<= length(v);
    {
        struct member *member = iter_next(iter);
        //@ containsIth(v, i);
        //@ foreach_remove(v, member);
        //@ open member(member);
        struct writer *memberWriter = member->writer;
        writer_write_string_buffer(memberWriter, senderNick);
        writer_write_string(memberWriter, " joined the room.\r\n");
        //@ close member(member);
        //@ foreach_unremove(v, member);
        hasNext = iter_has_next(iter);
    }
    iter_dispose(iter);
}

struct session {
    struct room *room;
    struct lock *room_lock;
    struct socket *socket;
};

/*@

predicate session(struct session *session)
    requires session->room |-> ?room &*& session->room_lock |-> ?roomLock &*& session->socket |-> ?socket &*& malloc_block_session(session)
        &*& lock_permission(roomLock, room_label, room) &*& socket(socket, ?reader, ?writer) &*& reader(reader) &*& writer(writer);

@*/

struct session *create_session(struct room *room, struct lock *roomLock, struct socket *socket)
  //@ requires lock_permission(roomLock, room_label, room) &*& socket(socket, ?reader, ?writer) &*& reader(reader) &*& writer(writer);
  //@ ensures session(result);
{
    struct session *session = malloc(sizeof(struct session));
    session->room = room;
    session->room_lock = roomLock;
    session->socket = socket;
    //@ close session(session);
    return session;
}

void room_label() 
	//@ requires emp; 
	//@ ensures emp; 
{ } // Used only as a label.

/*@

predicate_family_instance lock_invariant(room_label)(void *data)
    requires room(data);

lemma void assume(bool b);
    requires emp;
    ensures b;

@*/

void session_run_with_nick(struct room *room, struct lock *roomLock, struct reader *reader, struct writer *writer, struct string_buffer *nick)
  //@ requires lock_permission(roomLock, room_label, room) &*& room(room) &*& reader(reader) &*& writer(writer) &*& string_buffer(nick);
  //@ ensures lock_permission(roomLock, room_label, room) &*& reader(reader) &*& writer(writer) &*& string_buffer(nick);
{
	
    //@ open room(room);
    //@ open foreach(?v, @member);
    //@ close foreach(v, @member);
	struct list *roomMembers = 0;
	struct string_buffer *memberNick = 0;
    struct list *members = room->members;
	
    struct member *member = malloc(sizeof(struct member));
    struct string_buffer *nickCopy = string_buffer_copy(nick);
    room_broadcast_join_message(room,nick);
    member->nick = nickCopy;
    member->writer = writer;
    //@ close member(member);
    list_add(members, member);
    //@ foreach_member_not_contains(v, member);
    //@ close foreach(cons(member, v), @member);
    //@ close room(room);
    //@ close lock_invariant(room_label)(room);
    lock_release(roomLock);
    
    {
        bool eof = false;
        struct string_buffer *message = create_string_buffer();
        while (!eof)
          //@ invariant reader(reader) &*& string_buffer(nick) &*& string_buffer(message) &*& lock_permission(roomLock, room_label, room);
        {
            eof = reader_read_line(reader, message);
            if (eof) {
            } else {
                lock_acquire(roomLock);
                //@ open lock_invariant(room_label)(room);
                room_broadcast_message(room, nick, message);
                //@ close lock_invariant(room_label)(room);
                lock_release(roomLock);
            }
        }
        string_buffer_dispose(message);
    }
    
    lock_acquire(roomLock);
    //@ open lock_invariant(room_label)(room);
    //@ open room(room);
    roomMembers = room->members;
    //@ assert list(roomMembers, ?roomMembersValue);
    //@ assume(contains(roomMembersValue, member));
    list_remove(roomMembers, member);
    //@ foreach_remove(roomMembersValue, member);
    //@ close room(room);
    room_broadcast_goodbye_message(room, nick);
    //@ close lock_invariant(room_label)(room);
    lock_release(roomLock);
    
    //@ open member(member);
    memberNick = member->nick;
    string_buffer_dispose(memberNick);
    //@ assert writer(?memberWriter);
    //@ assume(memberWriter == writer);
    free(member);
}

/*@

predicate_family_instance thread_run_data(session_run)(void *data)
    requires session(data);

@*/

void session_run(void *data) //@ : thread_run
{
    //@ open thread_run_data(session_run)(data);
    struct session *session = data;
    //@ open session(session);
    struct room *room = session->room;
    struct lock *roomLock = session->room_lock;
    struct socket *socket = session->socket;
    struct writer *writer = socket_get_writer(socket);
    struct reader *reader = socket_get_reader(socket);
    free(session);
    
    writer_write_string(writer, "Welcome to the chat room.\r\n");
    writer_write_string(writer, "The following members are present:\r\n");
    
    lock_acquire(roomLock);
    //@ open lock_invariant(room_label)(room);
    //@ open room(room);
    {
        struct list *members = room->members;
        //@ assert list(members, ?membersValue);
        struct iter *iter = list_create_iter(members);
        bool hasNext = iter_has_next(iter);
        //@ lengthPositive(membersValue);
        while (hasNext)
            //@ invariant writer(writer) &*& iter(iter, members, membersValue, ?i) &*& foreach(membersValue, @member) &*& hasNext == (i < length(membersValue)) &*& 0 <= i &*& i <= length(membersValue);
        {
            struct member *member = iter_next(iter);
            //@ containsIth(membersValue, i);
            //@ foreach_remove(membersValue, member);
            //@ open member(member);
            struct string_buffer *nick = member->nick;
            writer_write_string_buffer(writer, nick);
            writer_write_string(writer, "\r\n");
            //@ close member(member);
            //@ foreach_unremove(membersValue, member);
            hasNext = iter_has_next(iter);
        }
        iter_dispose(iter);
    }
    //@ close room(room);
    //@ close lock_invariant(room_label)(room);
    lock_release(roomLock);

    {
        struct string_buffer *nick = create_string_buffer();
        bool done = false;
        while (!done)
          //@ invariant writer(writer) &*& reader(reader) &*& string_buffer(nick) &*& lock_permission(roomLock, room_label, room);
        {
            writer_write_string(writer, "Please enter your nick: ");
            {
                bool eof = reader_read_line(reader, nick);
                if (eof) {
                    done = true;
                } else {
                    lock_acquire(roomLock);
                    //@ open lock_invariant(room_label)(room);
                    {
                        bool hasMember = room_has_member(room, nick);
                        if (hasMember) {
                            //@ close lock_invariant(room_label)(room);
                            lock_release(roomLock);
                            writer_write_string(writer, "Error: This nick is already in use.\r\n");
                        } else {
                            session_run_with_nick(room, roomLock, reader, writer, nick);
                            done = true;
                        }
                    }
                }
            }
        }
        string_buffer_dispose(nick);
    }

    socket_close(socket);
}

int main()
  //@ requires true;
  //@ ensures false;
{
    struct room *room = create_room();
    //@ close lock_invariant(room_label)(room);
    struct lock *roomLock = create_lock();
    struct server_socket *serverSocket = create_server_socket(12345);

    while (true)
      //@ invariant lock_permission(roomLock, room_label, room) &*& server_socket(serverSocket);
    {
        struct socket *socket = server_socket_accept(serverSocket);
        //@ split_lock_permission(roomLock);
        struct session *session = create_session(room, roomLock, socket);
        //@ close thread_run_data(session_run)(session);
        thread_start(session_run, session);
    }
}
