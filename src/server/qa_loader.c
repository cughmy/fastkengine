#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "fastcommon/shared_func.h"
#include "fastcommon/logger.h"
#include "fastcommon/fast_buffer.h"
#include "server_global.h"
#include "qa_reader.h"
#include "qa_loader.h"

static inline const string_t *try_insert_keyword_and_similar(
        const string_t *keyword, const string_t *similar)
{
    KeywordHashEntry *hentry;
    const string_t *hsimilar;

    hsimilar = keyword_hashtable_find_similar(&g_server_vars.kh_context,
            keyword);
    if (hsimilar != NULL) {
        return hsimilar;
    }
    if ((hentry=keyword_hashtable_insert_ex(&g_server_vars.kh_context,
            keyword, similar)) != NULL) {
        return &hentry->similar;
    }

    return NULL;
}

const string_t *keyword_to_similar(string_t *keyword)
{
    string_t formatted;
    string_t concated;
    const string_t *fsimilar;
    const string_t *csimilar;
    char fholder[256];
    char cholder[256];

    if (keyword->len > sizeof(fholder)) {
        logWarning("file: "__FILE__", line: %d, "
                "keywords length %d exceeds %d",
                __LINE__, keyword->len, (int)sizeof(fholder));
        return NULL;
    }

    formatted.str = fholder;
    concated.str = cholder;
    keyword_normalize(keyword, &formatted, &concated);
    if (formatted.len == 0) {
        return NULL;
    }

    if (concated.len > 0) {
        if ((csimilar=try_insert_keyword_and_similar(&concated,
                        &concated)) == NULL)
        {
            return NULL;
        }
        fsimilar = csimilar;
    } else {
        fsimilar = &formatted;
    }

    return try_insert_keyword_and_similar(&formatted, fsimilar);
}

static void keywords_to_similar(KeywordArray *karray)
{
    string_t *p;
    string_t *end;
    const string_t *similar;

    //logInfo("count==: %d", karray->count);
    end = karray->keywords + karray->count;
    for (p = karray->keywords; p < end; p++) {
        //logInfo("keyword===: %.*s", p->len, p->str);

        similar = keyword_to_similar(p);
        if (similar == NULL) {
            FC_SET_STRING_NULL(*p);
            continue;
        }

        /* MUST to copy members for persistent in question_index hashtable */
        *p = *similar;
    }
}

static inline void keyword_records_normalize(KeywordRecords *results)
{
    KeywordArray *p;
    KeywordArray *end;

    end = results->rows + results->count;
    for (p=results->rows; p<end; p++) {
        keywords_to_similar(p);
    }
}

static int load_question_answers(QAReaderContext *reader)
{
    int result;
    QAReaderEntry entry;

    result = 0;
    while (1) {
        if ((result=qa_reader_next(reader, &entry)) != 0) {
            break;
        }

        keyword_records_normalize(&entry.questions);
        keyword_records_unique(&entry.questions);

        //logInfo("line: %d, questions count: %d", __LINE__, entry.questions.count);

        if ((result=question_index_adds(&g_server_vars.ki_context,
            &entry.questions, &entry.answer)) != 0)
        {
            break;
        }
    }

    return result;
}
 
static int add_answer_slinks(FastBuffer *buffer)
{
    KeywordIndexContext *context;
    KeywordIndexHashEntry *hentry;
    ConditionAnswerEntry *p;
    ConditionAnswerEntry *end;
    ConditionAnswerArray *answer_array;
    int result;

    context = &g_server_vars.ki_context;
    hentry = context->list;
    while (hentry != NULL) {
        answer_array = &hentry->answer.condition_answers;
        end = answer_array->entries + answer_array->count;
        for (p=answer_array->entries; p<end; p++) {
            if ((result=word_segment_add_slinks(&p->answer.origin,
                            hentry->answer.id, buffer)) != 0)
            {
                return result;
            }

            if ((result=fast_mpool_strdup_ex(&context->string_allocator,
                            &p->answer.for_html, buffer->data,
                            buffer->length)) != 0)
            {
                return result;
            }
        }

        hentry = hentry->lnext;
    }

    return 0;
}

int qa_loader_init(char **filenames, const int count)
{
    int result;
    QAReaderContext reader;
    FastBuffer buffer;
    char **pp;
    char **end;

    if ((result=fast_buffer_init_ex(&buffer, 4096)) != 0) {
        return result;
    }

    end = filenames + count;
    for (pp=filenames; pp<end; pp++) {
        /*
        if (strstr(*pp, "fastken.ken") == NULL) {
            continue;
        }
        logInfo("filename: %s", *pp);
        */
        result = qa_reader_init(&reader, &g_server_vars.kh_context.
                string_allocator, &buffer, *pp);

        if (result != 0) {
            break;
        }

        result = load_question_answers(&reader);
        if (result == ENOENT) {
            result = 0;
        }
        qa_reader_destroy(&reader);

        if (result != 0) {
            break;
        }
    }

    if (result == 0) {
        result = add_answer_slinks(&buffer);
    }

    fast_buffer_destroy(&buffer);
    return result;
}
