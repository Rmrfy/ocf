/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef UTILS_CLEANER_H_
#define UTILS_CLEANER_H_

#include "../ocf_request.h"

/**
 * @brief Getter for next cache line to be cleaned
 *
 * @param cache[in] Cache instance
 * @param getter_context[in] Context for cleaner caller
 * @param item[in] Current iteration item when collection cache lines
 * @param line[out] line to be cleaned
 * @retval 0 When caller return zero it means take this cache line to clean
 * @retval Non-zero Means skip this cache line and do not clean it
 */
typedef int (*ocf_cleaner_get_item)(struct ocf_cache *cache,
		void *getter_context, uint32_t item, ocf_cache_line_t *line);

/**
 * @brief Cleaning attributes for clean request
 */
struct ocf_cleaner_attribs {
	uint8_t  cache_line_lock : 1;	/*!< Clean under cache line lock */

	uint8_t  do_sort : 1;	/*!< Sort cache lines which will be cleaned */

	uint32_t count; /*!< max number of cache lines to be cleaned */

	void *cmpl_context; /*!< Completion context of cleaning requester */
	void (*cmpl_fn)(void *priv, int error); /*!< Completion function of requester */

	ocf_cleaner_get_item getter;
		/*!< Getter for collecting cache lines which will be cleaned */
	void *getter_context;
		/*!< Context for getting cache lines */
	uint32_t getter_item;
		/*!< Additional variable that can be used by cleaner caller
		 * to iterate over items
		 */

	ocf_queue_t io_queue;
};

/**
 * @brief Flush table entry structure
 */
struct flush_data {
	uint64_t core_line;
	uint32_t cache_line;
	ocf_core_id_t core_id;
};

/**
 * @brief Flush table container
 */
struct flush_container {
	ocf_core_id_t core_id;
	struct flush_data *flush_data;
	uint32_t count;
	uint32_t iter;

	struct ocf_cleaner_attribs attribs;
	ocf_cache_t cache;
	env_atomic *progress;
	env_atomic *error;
	env_waitqueue *wq;
	env_atomic completed;

	uint64_t flush_portion;
	uint64_t ticks1;
	uint64_t ticks2;
};

/**
 * @brief Run cleaning procedure
 *
 * @param cache - Cache instance
 * @param attribs - Cleaning attributes
 */
void ocf_cleaner_fire(struct ocf_cache *cache,
		const struct ocf_cleaner_attribs *attribs);

/**
 * @brief Perform cleaning procedure for specified flush data. Only dirty
 * cache lines will be cleaned.
 *
 * @param cache - Cache instance
 * @param flush - flush data to be cleaned
 * @param count - Count of cache lines to be cleaned
 * @param attribs - Cleaning attributes
 * @return - Cleaning result. 0 - no errors, non zero errors occurred
 */
int ocf_cleaner_do_flush_data_async(struct ocf_cache *cache,
		struct flush_data *flush, uint32_t count,
		struct ocf_cleaner_attribs *attribs);

/**
 * @brief Sort flush data by core sector
 *
 * @param tbl Flush data to sort
 * @param num Number of entries in tbl
 */
void ocf_cleaner_sort_sectors(struct flush_data *tbl, uint32_t num);

/**
 * @brief Sort flush data in all flush containters
 *
 * @param tbl Flush containers to sort
 * @param num Number of entries in fctbl
 */
void ocf_cleaner_sort_flush_containers(struct flush_container *fctbl,
		uint32_t num);

#endif /* UTILS_CLEANER_H_ */
