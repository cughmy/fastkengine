//wordsegment.h

#ifndef _WORD_SEGMENT_H
#define _WORD_SEGMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fastcommon/common_define.h"
#include "fastcommon/fast_mblock.h"
#include "fastcommon/fast_mpool.h"
#include "keyword_iterator.h"

struct word_segment_hash_table;

typedef struct word_segment_hash_entry {
    string_t ch;   //Chinese character with UTF-8 charset
    bool is_keyword;
    struct word_segment_hash_table *children;  //children characters
    struct word_segment_hash_entry *next;
} WordSegmentHashEntry;

typedef struct word_segment_hash_table {
    struct word_segment_hash_entry **buckets;
    int capacity;
} WordSegmentHashTable;

typedef struct word_segment_context {
    WordSegmentHashTable *top;
    int top_capacity;
    int max_chinese_chars;
    struct fast_mblock_man hentry_allocator;
    struct fast_mblock_man htable_allocator;
    struct fast_mpool_man  string_allocator;
} WordSegmentContext;

#define MAX_KEYWORDS_ROWS  (MAX_KEYWORDS_COUNT * MAX_KEYWORDS_COUNT)

typedef struct word_segment_array {
    KeywordArray rows[MAX_KEYWORDS_ROWS];
    int count;

    char buff[256];   //for internal use
    string_t holder;  //for internal use
} WordSegmentArray;

#ifdef __cplusplus
extern "C" {
#endif

    //the keyword only support Chinese
    int word_segment_init(WordSegmentContext *context, const int top_capacity,
            const string_t *keywords, const int count);

    void word_segment_destroy(WordSegmentContext *context);

    void word_segment_normalize(const string_t *input, string_t *output);

    int word_segment_split(WordSegmentContext *context, const string_t *input,
            WordSegmentArray *output);

    void word_segment_free_result(WordSegmentArray *array);

#ifdef __cplusplus
}
#endif

#endif
