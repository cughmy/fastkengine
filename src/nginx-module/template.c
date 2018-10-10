
static int check_alloc_index_node_array()
{
    int bytes;

    if (index_temp_node_array.alloc > index_temp_node_array.count) {
        return 0;
    }

    if (index_temp_node_array.alloc == 0) {
        index_temp_node_array.alloc = 2;
    } else {
        index_temp_node_array.alloc *= 2;
    }

    bytes = sizeof(TemplateNode) * index_temp_node_array.alloc;
    index_temp_node_array.nodes = (TemplateNode *)realloc(
            index_temp_node_array.nodes, bytes);
    if (index_temp_node_array.nodes == NULL) {
        logError("file: "__FILE__", line: %d, "
                "realloc %d bytes fail", __LINE__, bytes);
        return ENOMEM;
    }

    return 0;
}


static int add_index_template_node(const int type, string_t *value)
{
    int result;

    if (value->len == 0) {
        return 0;
    }

    if ((result=check_alloc_index_node_array()) != 0) {
        return result;
    }

    index_temp_node_array.nodes[index_temp_node_array.count].type = type;
    index_temp_node_array.nodes[index_temp_node_array.count].value = *value;
    index_temp_node_array.count++;
    return 0;
}

static inline int add_index_template_node_ex(const int type,
        char *str, const int len)
{
    string_t value;
    FC_SET_STRING_EX(value, str, len);
    return add_index_template_node(type, &value);
}

static bool is_variable(const string_t *value)
{
#define MAX_VARIABLE_LENGTH   64
    const char *p;
    const char *end;

    if (value->len > MAX_VARIABLE_LENGTH) {
        return false;
    }

    end = value->str + value->len;
    for (p = value->str; p < end; p++) {
        if (!(FC_IS_LETTER(*p) || *p == '_')) {
            return false;
        }
    }
    return true;
}

static int parse_index_template(string_t *content)
{
#define VARIABLE_MARK_START_STR "${"
#define VARIABLE_MARK_START_LEN (sizeof(VARIABLE_MARK_START_STR) - 1)

    int result;
    char *p;
    char *end;
    char *var_start;
    char *var_end;
    char *text_start;
    string_t var;

    result = 0;
    text_start = p = content->str;
    end = content->str + content->len;
    while (p < end) {
        var_start = strstr(p, VARIABLE_MARK_START_STR);
        if (var_start == NULL) {
            break;
        }

        var.str = var_start + VARIABLE_MARK_START_LEN;
        var_end = strchr(var.str, '}');
        if (var_end == NULL) {
            break;
        }

        var.len = var_end - var.str;
        if (!is_variable(&var)) {
            p = var.str;
            continue;
        }

        if ((result=add_index_template_node_ex(TEMPLATE_NODE_TYPE_STRING,
                        text_start, var_start - text_start)) != 0)
        {
            return result;
        }
        if ((result=add_index_template_node(TEMPLATE_NODE_TYPE_VARIABLE,
                        &var)) != 0)
        {
            return result;
        }

        text_start = p = var_end + 1;
    }

    return add_index_template_node_ex(TEMPLATE_NODE_TYPE_STRING,
        text_start, end - text_start);
}

static int load_index_template()
{
    const char *filename = "/etc/fken/template/index.html";
    int64_t file_size;
    int result;

    if ((result=getFileContent(filename, &index_file_content.str, &file_size)) != 0) {
        return result;
    }
    index_file_content.len = file_size;
    return parse_index_template(&index_file_content);
}

static int alloc_output_buffer(ngx_http_request_t *r,
        const key_value_array_t *params, string_t *output)
{
    key_value_pair_t *kv;
    key_value_pair_t *kv_end;
    int value_len;
    int buff_len;
    int len;

    value_len = 0;
    kv_end = params->kv_pairs + params->count;
    for (kv=params->kv_pairs; kv<kv_end; kv++) {
        len = kv->value.len - kv->key.len;
        value_len += (len > 0) ? len : 0;
    }

    buff_len = index_file_content.len + value_len;
    logInfo("value_len: %d, buff_len: %d", value_len, buff_len);

    output->str = (char *)ngx_palloc(r->pool, sizeof(char) * buff_len);
    if (output->str == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "ngx_palloc %d bytes fail", buff_len);
        return ENOMEM;
    }

    return 0;
}

static int render_index_template(ngx_http_request_t *r,
        const key_value_array_t *params, string_t *output)
{
    int i;
    int result;
    key_value_pair_t *kv;
    key_value_pair_t *kv_end;
    string_t *value;
    char *p;

    if ((result=alloc_output_buffer(r, params, output)) != 0) {
        return result;
    }

    p = output->str;
    kv_end = params->kv_pairs + params->count;
    for (i=0; i<index_temp_node_array.count; i++) {
        if (index_temp_node_array.nodes[i].type == TEMPLATE_NODE_TYPE_STRING) {
            value = &index_temp_node_array.nodes[i].value;
        } else {
            value = &empty_string;
            for (kv=params->kv_pairs; kv<kv_end; kv++) {
                if (fc_string_equal(&index_temp_node_array.nodes[i].value,
                            &kv->key))
                {
                    value = &kv->value;
                    break;
                }
            }
        }

        if (value->len > 0) {
            memcpy(p, value->str, value->len);
            p += value->len;
        }
    }
    output->len = p - output->str;
    return 0;
}
