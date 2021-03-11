#ifndef skynet_databuffer_h
#define skynet_databuffer_h

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MESSAGEPOOL 1023

struct message {
    char * buffer;
    int size;
    struct message * next;
};

struct databuffer {
    int header; //在databuffer_readheader中赋值,保存的不知道是头部大小还是消息体的大小--comment by dengkai
    int offset; //表示head所指向的节点已被读取的字节数,即(head->buffer+offset)开始的数据是未被读取的--comment by dengkai
    int size;   //size of all the message
    struct message * head;
    struct message * tail;
};

struct messagepool_list {
    struct messagepool_list *next;
    struct message pool[MESSAGEPOOL];
};

struct messagepool {
    struct messagepool_list * pool;
    struct message * freelist;
};


static void
messagepool_free(struct messagepool * pool) {
}

static inline void
_return_message(struct databuffer *db, struct messagepool *mp) {
    //将databuffer 中的head向前移动一位，将释放了的message 连接到 messagepool中的freelist头部
    struct message *m = db->head;
    if (m->next == NULL) {
        assert(db->tail == m);
        db->head = db->tail = NULL;
    } else {
        db->head = m->next;
    }
    skynet_free(m->buffer);
    m->buffer = NULL;
    m->size = 0;
    m->next = mp->freelist;
    mp->freelist = m;
}

//从databuffer中读取sz大小字节到buffer，并回收dadabuffer中读完的message到pool中
static void
databuffer_read(struct databuffer *db, struct messagepool *mp, void *buffer, int sz) {
    assert(db->size >= sz);
    db->size -= sz;
    for (;;) {
        struct message *current = db->head;
        int bsz = current->size - db->offset;
        if (bsz > sz) {
            memcpy(buffer, current->buffer + db->offset, sz);
            db->offset += sz;
            return;
        }
        if (bsz == sz) {
            memcpy(buffer, current->buffer + db->offset, sz);
            db->offset = 0;
            _return_message(db, mp);
            return;
        } else {
            memcpy(buffer, current->buffer + db->offset, bsz);
            _return_message(db, mp);
            db->offset = 0;
            buffer += bsz;
            sz -= bsz;
        }
    }

}

//从messagepool 的freelist中取出一个message 并赋值，将其放入databuffer中
static void
databuffer_push(struct databuffer *db, struct messagepool *mp, void *data, int sz) {
    // 1.从messagepool的freelist中取一个message节点
    struct message *m;
    if (mp->freelist) {
        m = mp->freelist;
        mp->freelist = m->next;
    } else {
        struct messagepool_list * mpl = skynet_malloc(sizeof(*mpl));
        struct message * temp = mpl->pool;
        int i;
        for (i = 1; i < MESSAGEPOOL; ++i) {
            temp[i].buffer = NULL;
            temp[i].size = 0;
            temp[i].next = &temp[i+1];
        }
        temp[MESSAGEPOOL-1].next = NULL;
        mpl->next = mp->pool;
        mp->pool = mpl;
        m = &temp[0];
        mp->freelist = &temp[1];
    }

    // 2.将message节点添加到databuffer的尾部
    m->buffer = data;
    m->size = sz;
    m->next = NULL;
    db->size += sz;
    if (db->head == NULL) {
        assert(db->tail == NULL);
        db->head = db->tail = m;
    } else {
        db->tail->next = m;
        db->tail = m;
    }
}

// 从databuffer中读取当前节点(即head所指节点)的大小,并保存到db->header中, header_size取值为2或4
static int
databuffer_readheader(struct databuffer *db, struct messagepool *mp, int header_size) {
    if (db->header == 0) {
        // parser header (2 or 4)
        if (db->size < header_size) {
            return -1;
        }
        uint8_t plen[4];
        databuffer_read(db,mp,(char *)plen,header_size);
        // big-endian
        if (header_size == 2) {
            db->header = plen[0] << 8 | plen[1];
        } else {
            db->header = plen[0] << 24 | plen[1] << 16 | plen[2] << 8 | plen[3];
        }
    }
    if (db->size < db->header) {
        return -1;
    }
    return db->header;
}

static inline void
databuffer_reset(struct databuffer *db) {
    db->header = 0;
}

static void
databuffer_clear(struct databuffer *db, struct messagepool *mp) {
    while (db->head) {
        _return_message(db, mp);
    }
    memset(db, 0, sizeof(*db));
}

#endif
