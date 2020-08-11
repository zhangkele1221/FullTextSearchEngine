#include "search.h"
#include "encoding.h"
#include "token.h"
#include "ftse_env.h"
#include "postings.h"
#include <vector>
#include <algorithm>

using namespace ftse;


void Search::split_query_to_tokens(FullTextSearchEngineEnv& ftse_env,
        const UTF32Char *text,
        const unsigned int text_len,
        const int n, QueryToken* query_tokens) {
    return Token::text_to_postings_lists(ftse_env,
            0, /* 将document_id设为0 */
            text, text_len, n,
            query_tokens);
}

void Search::search_docs(FullTextSearchEngineEnv& ftse_env, const QueryToken& query_tokens,
        SearchResults* results) {
    typedef std::pair<int, InvertIndexEntry> PAIR;
    std::vector<PAIR> tokens(
            query_tokens.begin(), query_tokens.end());
    // 按照token在所有doc中出现的次数排序
    sort(tokens.begin(), tokens.end(),[](const PAIR& a, const PAIR& b)
            { return a.second.documents_count < b.second.documents_count; });
    std::vector<DocSearchCursor> cursors(tokens.size());
    for(int i = 0; i < tokens.size(); ++i) {
        /* 当前的token在构建索引的过程中从未出现过 */
        if(tokens[i].second.token_id == 0) {
            return;
        }
        Postings::fetch_postings(ftse_env, tokens[i].second.token_id, &cursors[i].documents);
        /* 虽然当前的token存在，但是由于更新或删除导致其倒排列表为空 */
        if(cursors[i].documents.size() == 0) {
            return;
        }
        cursors[i].current = cursors[i].documents.begin();
    }
    while(cursors[0].current != cursors[0].documents.end()) {
    }
}

void Search::search(FullTextSearchEngineEnv& ftse_env, const char* query) {
    int query32_len;
    UTF32Char* query32; 
    if (RET_SUCC == Encoding::utf8toutf32(query, strlen(query), &query32, &query32_len)) {
        SearchResults results;
        if (query32_len < ftse_env.get_token_len()) {
            Encoding::print_error("too short query.");
        } else { 
            QueryToken query_tokens;
            split_query_to_tokens(ftse_env, query32, query32_len, ftse_env.get_token_len(), &query_tokens);
            search_docs(ftse_env, query_tokens, &results);
        }
    }
}
