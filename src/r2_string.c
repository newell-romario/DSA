#include "r2_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 * @brief               Performs a naive substring search for a pattern in a string.
 * 
 * @param str           String.
 * @param pat           Pattern.
 * @return r2_int64     Returns the location of the first match found, else -1.
 */
r2_int64 r2_naive_substring(const r2_c *str, const r2_c *pat)
{
        /**
         * @brief The naive string matching algorithm is the first intuitive algorithm everyone thinks of. 
         * The naive algorithm runs in O(mn) where m is the length of the pattern P and n is the length of the text T to be searched.
         * The naive substring matching algorithm tries to the find the substring at every n-m+1 position. It uses no information
         * gained from mismatch searches to speed up computation. 
         * 
         * CLRS Section 32.1 describes the naive string matching algorithm and provides a good example.
         */
        r2_int64 len;/*length of pattern*/
        for(r2_int64 i = 0; str[i] != '\0'; ++i){
                len = 0;
                for(;str[i+len] != '\0' && pat[len] != '\0'; ++len){
                        if(pat[len] != str[i+len])
                                break;
                }

                if(pat[len] == '\0')
                        return i;
        }

        return pat[0] == '\0' && str[0] == '\0'? 0 : -1;
}

/**
 * @brief               An implmenetation of the rabin karp string  matching algorithm.
 * 
 * @param str           String.
 * @param pat           Pattern.                  
 * @return r2_int64     Returns the location of the first match found, else -1. 
 */
r2_int64 r2_rabin_karp(const r2_c *str, const r2_c *pat)
{
        /**
         * @brief The rabin karp string matching algorithm works by creating a hash of the pattern P and then looking for that
         * hash in the text T. This string matching algorithm is very elegant and relies on ideas from hashing.
         * Recall an important way of checking for the existence of a key is to build a hash table and store that key at the
         * table location returned by the hash function. If that location is empty we can certainly say the key doesn't exist. 
         * On the other hand if the location is non-empty we would have to check every key stored at that position to determine
         * if the key exists. The rabin karp string matching algorithm implicitly builds a hash table and stores the key.
         *  
         * For every T[i...i+m] substring we find the hash and compare it to the hash of the pattern P. If both hash match
         * we have a potential match, we further compare the substring starting at T[i] with the pattern to determine if it actually matches.
         *  
         * With a bit of preprocessing for the pattern P we can achieve the average case of O(n+m) where m is the length of the pattern and n is
         * the length of the text.
         * 
         * The bottleneck in this algorithm is the computation of the hash for each T[i...i+m]. However, we can compute the hash of T[i+1...m+1] in
         * constant time using a quite straightforward observation. Suppose we are trying to find the string 12. The string 12 can be represented as 1*10^1 + 2*10^0. 
         * T[0...1] can be represented 4*10^1 + 5*10^0. The hash of T[0...1] would be equal to (4*10^1 + 5*10^0) mod 101 
         * and the hash of the pattern is (1*10^1 + 2*10^0) mod 101. Notice both hash are unequal so the string starting T[0] is a mismatch but what about the 
         * string starting at T[1]? The string starting at T[1] is T[1..2] and it's hash is (5*10^1 + 0*10^0) mod 101.  
         * 
         * Rabin and Karp noticed that you can derive the hash of the string at T[1] using the string at T[0]. We can get T[1] by ((4*10^1 + 5*10^0) - 4*10^1)*10 + 0*10^0.
         * The rabin karp algorithm exploits this to speed up the computation. See https://www.cs.cmu.edu/afs/cs/academic/class/15451-f14/www/lectures/lec6/karp-rabin-09-15-14.pdf for a
         * thorough explanation.
         *  
         */
        r2_uint64 pat_hash  = 0;/*hash of pattern*/
        r2_uint64 str_hash  = 0;/*hash of text*/
        r2_uint64 sig_digit = 1;/*significant digit*/
        r2_uint64 prime = 155654281135519;/*prime number*/
        r2_uint64 pl = 0;/*length of pattern*/
        r2_uint64 k  = 0;
        r2_uint64 alpha_size = 256;/*alphabet size*/

        /*Computing hash of pattern*/
        for(r2_uint64 i = 0; pat[i] != '\0'; pl = ++i)
                pat_hash  = (pat_hash*alpha_size + pat[i])%prime;
        
        /*Computing significant digit*/
        for(r2_int64 i = 1; i < pl ; ++i)
                sig_digit = (sig_digit*alpha_size)%prime;
        
        /*Computing hash of string*/
        for(r2_uint64 i = 0; str[i] != '\0' && i != pl; ++i)
                str_hash  = (str_hash*alpha_size + str[i])%prime; 
        
        if(pat_hash == str_hash){
                while(pat[k] == str[k] && pat[k] != '\0' && str[k] != '\0')
                        ++k; 
                /*We matched*/
                if(k == pl)
                        return 0;
        }
                
        for(r2_uint64 i = pl; str[0] !='\0' && str[i] != '\0'; ++i){
                str_hash = (str_hash - str[i-pl]*sig_digit)%prime; 
                str_hash = (str_hash*alpha_size + str[i])%prime;
                if(str_hash == pat_hash){
                        /*Confirming possible match*/
                        k = 0;
                        for(r2_uint64 j = i-pl+1; k != pl; ++k)
                                if(pat[k] != str[k+j])
                                        break;
                        if(k == pl)
                                return i+1-pl;
                }
        }
        return -1;
}

/**
 * @brief               Performs a substring search by building a DFA of the pattern.
 * 
 * @param str           String.
 * @param pat           Pattern.
 * @return r2_int64     Returns the location of the first match found, else -1.
 */
r2_int64 r2_naive_dfa(const r2_c *str, const r2_c *pat)
{
        /**
         * @brief A finite automaton can be used to recognize a string. It recognizes the string by processing all characters in the string and then outputing if the string is accepted or not. 
         * A finite automata is defined as a 5 tuple (E, Q, T, F, S) where  
         * E - represents the alphabet 
         * Q - Set of states
         * T - Transition function
         * F - Final states
         * S - Start state
         * 
         * When a finite automaton outputs if a string is accepted that means after processing the string, the final state of the automaton is an accepting state otherwise its final state is failure state. 
         * Since a finite automaton can detect a string we can build a finite automaton of the pattern P and then let automaton process all the characters of text T. 
         * If the automaton outputs yes at any time then we found a substring match.
         * 
         * To understand the algorithm clearly see CLRS section 32.3.
         * 
         */
        
        r2_uint16 FAILED  = FALSE;
        r2_uint64 len     = strlen(pat);/*length of pattern*/
        if(len == 0) return 0;
        r2_uint64 **table = calloc(len+1, sizeof(r2_uint64 *));/*transition table*/
        r2_c *suffix = calloc(len + 1, sizeof(r2_c));
        r2_int64 pos = -1;
        const r2_c *prefix = pat;
        const r2_uint16 ALPHABET_SIZE = 256;
        if(table == NULL)
                goto CLEANUP;
        
        

        for(r2_uint64 row = 0; row < len+1; ++row){
                table[row] = calloc(ALPHABET_SIZE, sizeof(r2_uint64)); 
                if(table[row] == NULL)
                        FAILED = TRUE;
        }

        if(FAILED == TRUE)
                goto CLEANUP;

        
        for(r2_int64 row = 0, state = 0; row <= len; ++row, ++state){
                for(r2_c col = 0; col < ALPHABET_SIZE; ++col){
                        suffix[row]  = col;
                        r2_uint64 lp   = 0;/*longest prefix*/
                        for(r2_int64 i = row, j=i, p = 0; i >= 0; --i, p = 0){
                                for(r2_int64 k = row; j >= 0; --k, --j)
                                        if(suffix[k] == prefix[j])
                                                ++p;
                                        else break;
                                if(j == -1 && p > lp)
                                        lp = p;
                        }
                        table[state][col] = lp;
                }
                suffix[row] = prefix[row];
        }

        for(r2_int64 row = 0, state = 0; str[row] != '\0'; ++row){
                state = table[state][str[row]];
                if(state == len){
                        pos = row - state + 1;
                        break;
                }     
        }
        CLEANUP:
                for(r2_uint64 row = 0; row < len + 1; ++row)
                        if(table[row] != NULL)
                                free(table[row]);
                
                free(table); 
                free(suffix);
                return pos;
}


 /**
  * @brief              An implementation of the Knuth-Morris-Pratt algorithm.
  * 
  * @param str          String. 
  * @param pat          Pattern.
  * @return r2_int64    Returns the location of the first match found, else -1. 
  */
 r2_int64 r2_kmp(const r2_c *str, const r2_c *pat)
{
        /**
         * @brief The Knuth-Morris-Pratt string matching algorithm is a fast string matching algorithm
         * that finds a pattern in O(m + n) time where m is the length of the pattern and n the length of the string.
         * 
         * Knuth-Morris-Pratt algorithm tackles the string matching problem by building a DFA implicitly. 
         * It doesn't outright build the DFA, instead it focuses on determining the state of the DFA after a mismatch
         * occurs. Knowing which state to place the DFA in after a mistmatch speeds up computation because we can
         * avoid redundant comparisons. 
         * 
         * Example given the pattern abab and the text abac. The naive string matching algorithm when looking for the
         * pattern in the string would check each location in the text starting from a up to c and then realizing that it can't match
         * because of the mismatch at c. However, because the naive string matching isn't smart it would repeat the same action starting from b this time
         * although we know it can't match.  
         * 
         * KMP avoids these extra comparison by shifting the pattern to the correct alignment with the text. 
         * 
         * How do we determine valid shifts in KMP? We do this by using the failure function which is defined as the longest proper prefix of
         * P[:k] that is also a suffix of P[:k]. We can compute this in O(m) time using memoization. The longest proper prefix that is also
         * a suffix of P[:k] is stored in an array. The longest proper prefix at position i is dependent on the i-1 proper prefixes already 
         * been found. We simply extend one of those prefixes to get the longest prefix at position i. See CLRS section 32.9 or 
         * https://yurichev.com/mirrors/Knuth-Morris-Pratt/Knuth/Knuth77.pdf.
         */
        r2_uint64 len = strlen(pat);/*length of pattern*/
        if(len == 0)
                return len;
        
        r2_uint64 *ft = calloc(len, sizeof(r2_uint64));/*failure table*/
        r2_int64 j = -1; 
        if(ft == NULL)
                goto CLEANUP;
        
        r2_uint64 lp  = 0;/*longest proper prefix of pat[:k]*/
        for(r2_uint64 i = 1; i < len; ++i){
                while(lp > 0 && pat[lp] != pat[i])
                        lp = ft[lp];

                if(pat[lp] == pat[i])
                        lp = lp + 1;
                ft[i] = lp;
        }

        lp = 0;
        for(r2_uint64 i = 0; str[i] != '\0'; ++i){
                while(lp > 0 && pat[lp] != str[i])
                        lp = ft[lp-1];

                if(pat[lp] == str[i])
                        lp = lp + 1; 
                
                if(lp == len){
                        j = i-lp+1;
                        break;
                }         
        }

        CLEANUP:
                free(ft);
                return j;
}    