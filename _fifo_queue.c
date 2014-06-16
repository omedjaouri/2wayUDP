
/******************************************************************************
 *
 * \file fifo_queue.h
 * \brief FIFO-queue implementation.
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Copyright (C) 2014   Kevin George
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more detail.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 *
 *****************************************************************************/


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "_fifo_queue.h"
#include "stdio.h"
#include "pthread.h"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

bool queue_init(struct _queue *queue, size_t bufsize) {
  bool ret;
  memset(queue, 0, sizeof(struct _queue));
  // Init locks.
  ret = pthread_mutex_init(&queue->put_lock, NULL) == 0;
  if (!ret) {
    goto queue_init_err0;
  }
  ret = pthread_mutex_init(&queue->get_lock, NULL) == 0;
  if (!ret) {
    goto queue_init_err1;
  }

  printf("50");
  // Initialize queue.
  queue->size = bufsize;
  queue->buffer = (uint8_t*)malloc( queue->size);
  ret = queue->buffer != NULL;
  if (!ret) {
    goto queue_init_err2;
  }

  // Error handling.
queue_init_err2:
  pthread_mutex_destroy(&queue->get_lock);
queue_init_err1:
  pthread_mutex_destroy(&queue->put_lock);
queue_init_err0:
  return ret;
}

void queue_deinit(struct _queue *queue) {
  pthread_mutex_lock(&queue->get_lock);
  pthread_mutex_lock(&queue->put_lock);
  // Free the buffer and set the size to 0, invalidating the queue.
  free(queue->buffer);
  queue->size = 0;
  queue->buffer = NULL;
  queue->get = queue->put = 0;
  queue->get_parity = queue->put_parity = 0;
  pthread_mutex_unlock(&queue->put_lock);
  pthread_mutex_unlock(&queue->get_lock);
  pthread_mutex_destroy(&queue->put_lock);
  pthread_mutex_destroy(&queue->get_lock);
  return;
}

size_t queue_get(struct _queue *queue, char *buf, size_t size) {
  int block, read = 0;
  pthread_mutex_lock(&queue->get_lock);
  while (read < size) {
    // How much space we have in the buffer.
    // Update in case bg thread added some while we were busy.
    int bufleft = queue_space(queue, QUEUE_SPACE_GET);
    // If there's no more space, return.
    if (bufleft == 0)
      break;
    // How much we can transfer with memcpy.
    block = queue->size - queue->get;
    if (block > bufleft)
      block = bufleft;
    // We shouldn't transfer more data than is available.
    if (block > size - read)
      block = size - read;
    block = MIN(block, MIN(bufleft, size - read));
    // Perform the copy and update variables.
    memcpy(buf + read, queue->buffer + queue->get, block);
    queue->get = queue->get + block;
    if (queue->get >= queue->size) {
      queue->get = 0;
      queue->get_parity = ~queue->get_parity;
    }
    read = read + block;
  }
  pthread_mutex_unlock(&queue->get_lock);
  return read;
}

size_t queue_put(struct _queue *queue, char *buf, size_t size) {
  int block, read = 0;
  pthread_mutex_lock(&queue->put_lock);
  while (read < size) {
    // How much space we have in the buffer.
    // Update in case main thread read some while we were busy.
    int bufleft = queue_space(queue, QUEUE_SPACE_PUT);
    // If there's no more space, return.
    if (bufleft == 0)
      break;
    // How much we can transfer with memcpy.
    block = queue->size - queue->put;
    if (block > bufleft)
      block = bufleft;
    // We shouldn't transfer more data than is available.
    if (block > size - read)
      block = size - read;
    // Perform the copy and update variables.
    memcpy(queue->buffer + queue->put, buf + read, block);
    queue->put = queue->put + block;
    if (queue->put >= queue->size) {
      queue->put = 0;
      queue->put_parity = ~queue->put_parity;
    }
    read = read + block;
  }
  pthread_mutex_unlock(&queue->put_lock);
  return read;
}

size_t queue_space(struct _queue *queue, int way) {
  int buf;
  if (way == QUEUE_SPACE_GET) {
    buf = queue->put - queue->get;
  } else if (way == QUEUE_SPACE_PUT) {
    buf = queue->get - queue->put;
  } else {
    return -1;
  }

  if (buf < 0) {
    return buf + queue->size;
  } else if (buf == 0) {
    if (queue->put_parity == queue->get_parity) {
      return (way == QUEUE_SPACE_GET) ? 0 : queue->size;
    } else {
      return (way == QUEUE_SPACE_GET) ? queue->size : 0;
    }
  } else {
    return buf;
  }
}

size_t queue_flush(struct _queue *queue) {
  pthread_mutex_lock(&queue->get_lock);
  pthread_mutex_lock(&queue->put_lock);
  // Fast-forward the get pointer and set its parity.
  size_t ret = queue_space(queue, QUEUE_SPACE_GET);
  queue->get = queue->put;
  queue->get_parity = queue->put_parity;
  pthread_mutex_unlock(&queue->put_lock);
  pthread_mutex_unlock(&queue->get_lock);
  return ret;
}

