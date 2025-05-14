#include "..\src\r2_string.h"
#include <stdio.h>
#include <assert.h>


/**
 * @brief Test naive substring.
 * 
 */
static void test_r2_naive_substring()
{
        const char *str = "Romario";
        assert(r2_naive_substring(str, "io")  == 5); 
        assert(r2_naive_substring(str, "o")   == 1); 
        assert(r2_naive_substring(str, "fun") == -1); 
        assert(r2_naive_substring(str, "oo")  == -1); 
        assert(r2_naive_substring(str, "oo")  == -1);
        assert(r2_naive_substring(str, "") == 0);
        assert(r2_naive_substring(str, str) == 0);
        assert(r2_naive_substring("", str) == -1);
        assert(r2_naive_substring("", "") == 0);
}

/**
 * @brief Test robin karp substring search.
 * 
 */
static void test_r2_rabin_karp_substring()
{
        const char *str = "Romario";
        assert(r2_rabin_karp(str, "io")  == 5); 
        assert(r2_rabin_karp(str, "o")   == 1); 
        assert(r2_rabin_karp(str, "fun") == -1); 
        assert(r2_rabin_karp(str, "oo")  == -1); 
        assert(r2_rabin_karp(str, "oo")  == -1);
        assert(r2_rabin_karp(str, "") == 0);
        assert(r2_rabin_karp(str, str) == 0);
        assert(r2_rabin_karp("", str) == -1);
        assert(r2_rabin_karp("", "") == 0); 

}

/**
 * @brief Test naive dfa.
 * 
 */
static void test_r2_naive_dfa()
{
        const char *str = "Romario";
        assert(r2_naive_dfa(str, "io")  == 5); 
        assert(r2_naive_dfa(str, "o")   == 1); 
        assert(r2_naive_dfa(str, "fun") == -1); 
        assert(r2_naive_dfa(str, "oo")  == -1); 
        assert(r2_naive_dfa(str, "oo")  == -1);
        assert(r2_naive_dfa(str, "") == 0);
        assert(r2_naive_dfa(str, str) == 0);
        assert(r2_naive_dfa("", str) == -1);
        assert(r2_naive_dfa("", "") == 0); 
}

static void test_r2_kmp()
{
        const char *str = "bacbababaabcbab";
        assert(r2_kmp(str, "cbab")  == 2); 
        assert(r2_kmp(str, "o")   == -1); 
        assert(r2_kmp(str, "aa") == 8); 
        assert(r2_kmp(str, "oo")  == -1); 
        assert(r2_kmp(str, "") == 0);
        assert(r2_kmp(str, str) == 0);
        assert(r2_kmp("", str) == -1);
        assert(r2_kmp("", "") == 0); 
}

void test_r2_string_run()
{
        test_r2_naive_substring();
        test_r2_rabin_karp_substring();
        test_r2_naive_dfa();
        test_r2_kmp();
}