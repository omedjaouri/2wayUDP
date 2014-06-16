
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


#ifndef __FIFO_QUEUE_H
#define __FIFO_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define QUEUE_SPACE_GET 1
#define QUEUE_SPACE_PUT 2

struct _queue {
  uint32_t size;
  uint8_t* buffer;
  volatile uint32_t get        : 31;
  volatile uint32_t get_parity : 1;
  volatile uint32_t put        : 31;
  volatile uint32_t put_parity : 1;
  pthread_mutex_t put_lock;
  pthread_mutex_t get_lock;
};

// Forward declarations
bool queue_init(struct _queue *queue, size_t bufsize);

void queue_deinit(struct _queue *queue);

size_t queue_space(struct _queue *queue, int way);

size_t queue_flush(struct _queue *queue);

/** queue_get: [thread-safe] FIFO buffer get operation.
 */
size_t queue_get(struct _queue *queue, char *buf, size_t size);

/** queue_put: [thread-safe] FIFO buffer put operation.
 */
size_t queue_put(struct _queue *queue, char *buf, size_t size);

#endif

