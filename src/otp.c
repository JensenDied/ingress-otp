#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <set>

#include <time.h>
#include <map>
#include "md5.h"
#include <google/dense_hash_map>

using std::multimap;
using std::pair;
using std::string;
using std::iterator;
using std::set;
using std::advance;

// Ideas
// Random strings based on valid keyspace for letters
// Random assembling of words and padding and xor'ing against the cipher and hashing the result
// Markov chain words and the above.
void test(const string, const string, const string);
multimap <string, string> mm_init();

static char *keybuff;

inline string random_element(const multimap <string, string> mm, const string key) {
    pair <multimap <string, string>::const_iterator, multimap <string, string>::const_iterator> its = mm.equal_range(key);
    std::size_t sz = std::distance(its.first, its.second);
    if(sz == 0)
        return "";
    std::size_t idx = std::rand() % sz;
    advance(its.first, idx);
    return its.first->second;
}

string random_element(const string elements[]) {
    int num = sizeof(elements) / sizeof(elements[0]);
    return elements[std::rand()%num];
}

string get_key_for_search(const string crypt, const string search) {
    //C:WJYDJZ
    //K:EFWPWW
    //S:SECOND
    string rot;
    int len = search.length();
    char c;
    for(int i = 0; i<len; i++) {
        c = crypt[i] - search[i] - 'A';
        if(c < 0)
            c+=26;
        rot += 'A'+c;
    }
    return rot;
}

string get_key_for_cipher_phrase(const string cipher, const string phrase) {
    //C:WJYDJZ
    //K:EFWPWW
    //S:SECOND
    //string rot;
    int len = cipher.length();
    char c;
    for(int i = 0; i<len; i++) {
        c = cipher[i] - (phrase[i] - 'A');
        if(c < 'A')
            c+=26;
        keybuff[i]= c;
    }
    keybuff[len] = 0;
    return keybuff;
}

string pad_phrase(const string phrase[], const unsigned int words, const unsigned int plen, const unsigned int clen) {
    string ret = "";
    for(unsigned int i = 0; i <= words; ++i) {
        ret += phrase[i];
        if(i < words) {
            ret += "X";
        }
    }
    return ret;
}

string pad_phrase_md5(const string phrase[], const unsigned int words, const unsigned int plen, const unsigned int clen, const string md5) {
    int budget = clen - plen;
    string ret = "";

    for(unsigned int i = 0; i <= words; ++i) {
        ret += phrase[i];
        if(i < words)
            ret += "X";
    }
    return ret;
}

int main(int argc, char **argv) {
    if(argc == 5) {
        fprintf(stderr, "c: %s, m: %s, p: %s, o: %s\nk2: %s",
                argv[1], argv[2], argv[3], argv[4], get_key_for_cipher_phrase(argv[1], argv[3]).c_str());
        exit(1);
    }
    if(argc != 3) {
        fprintf(stderr, "Usage:\n\t%s crypt md5\n", argv[0]);
        exit(1);
    }
    srand ( time(NULL) );
    string cipher = argv[1];
    string MD5 = argv[2];
    if(MD5.length() != 32) {
        fprintf(stderr, "Malformed MD5 string\n");
        exit(1);
    }
    //test(cipher, MD5);
    multimap<string,string> mm = mm_init();
    string num[] = { "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN", "SIXTEEN", "SEVENTEEN", "EIGHTEEN", "NINETEEN", "TWENTY", "TWENTYONE", "TWENTYTWO", "TWENTYTHREE", "TWENTYFOUR", "TWENTYFIVE", "TWENTYSIX", "TWENTYSEVEN", "TWENTYEIGHT", "TWENTYNINE", "THIRTY", "THIRTYONE", "THIRTYTWO", "THIRTYTHREE", "THIRTYFOUR", "THIRTYFIVE", "THIRTYSIX", "THIRTYSEVEN", "THIRTYEIGHT", "THIRTYNINE", "FORTY", "FORTYONE", "FORTYTWO", "FORTYTHREE", "FORTYFOUR", "FORTYFIVE", "FORTYSIX", "FORTYSEVEN", "FORTYEIGHT", "FORTYNINE", "FIFTY", "FIFTYONE", "FIFTYTWO", "FIFTYTHREE", "FIFTYFOUR", "FIFTYFIVE", "FIFTYSIX", "FIFTYSEVEN", "FIFTYEIGHT", "FIFTYNINE", };

    set<string> myset;
    for(multimap<string, string>::iterator it = mm.begin(), end = mm.end(); it != end; it = mm.upper_bound(it->first)) {
        myset.insert(it->first);
    }

    set<string>::iterator mbegin = myset.begin(), mysetit;
    int setlen = myset.size();
    unsigned int clen = cipher.length();
    unsigned int phraselen = 0;
    unsigned int words = 0;
    string phrase[50];
    string phrasestr = "";
    unsigned int attempts = 0, skips=0;
    string word, lword;
    unsigned int fails = 0;
    keybuff = new char[clen+1];
    printf("cipher: %s(%i)\n", cipher.c_str(), clen);

    for(int hour = 1; hour < 12; hour++)
        for(int minute = 0; minute < 60; minute++)
            for(int second = 0; second < 60; second++) {
                /*
                phrase = format_1(hour, minute, second); //FIVE O CLOCK TWO MINUTE(S) AND TWENTY SEVEN SECOND(S)
                phrase = format_2(hour, minute, second); //FIFTY FIVE SECONDS AND THREE MINUTES AFTER SEVEN
                phrase = format_3(hour, minute, second); //MEASUREMENT THREE IS AT NINE O THREE AND THIRTY SECONDS
                phrase = format_4(hour, minute, second); //THREE MINUTES FIFTY TWO SECONDS PAST THREE O CLOCK
                phrase = format_5(hour, minute, second); //TWO MINUTES AND SEVEN SECONDS PAST EIGHT PM
                //phrase = format_6(hour, minute, second); //TWO MINUTES AND SEVEN SECONDS PAST EIGHT
                phrase = format_7(hour, minute, second); //SIX O CLOCK AND FOUR MINUTES AND THIRTEEN SECONDS
                phrase = format_8(hour, minute, second); //THIRTEEN SECONDS PAST SEVEN O CLOCK SHARP
                */
                words = 0;
                word = num[hour]; phrase[words] = word; ++words;
                word = "O"; phrase[words] = word; ++words;
                word = "CLOCK"; phrase[words] = word; ++words;
                if(minute > 20 && (minute%10) != 0) {
                    word = num[minute - minute%10]; phrase[words] = word; ++words;
                    word = num[minute%10]; phrase[words] = word; ++words;
                } else {
                    word = num[minute]; phrase[words] = word; ++words;
                }
                word = "MINUTES"; phrase[words] = word; ++words;
                word = "AND"; phrase[words] = word; ++words;
                if(second > 20 && (second%10) != 0) {
                    word = num[second - second%10]; phrase[words] = word; ++words;
                    word = num[second%10]; phrase[words] = word; ++words;
                } else {
                    word = num[second]; phrase[words] = word; ++words;
                }
                word = "SECONDS"; phrase[words] = word;
                phrasestr = pad_phrase(phrase, words, phraselen, clen);
                //printf("phrase: %s, otp: %s, md5(otp): %s\n", phrasestr.c_str(), get_key_for_cipher_phrase(cipher, phrasestr).c_str(),md5(get_key_for_cipher_phrase(cipher, phrasestr)).c_str());
                if(MD5 == md5(get_key_for_cipher_phrase(cipher, phrasestr))) {
                    printf("phrase: %s\n"
                           "MD5:    %s\n"
                           "OTP:    %s\n", phrasestr.c_str(), MD5.c_str(), get_key_for_cipher_phrase(cipher, phrasestr).c_str());
                    exit(0);
                }
            }
    fprintf(stderr,"[ii] Format Checks Finished, Starting Random Guessing\n");
    while(MD5.compare(md5(get_key_for_cipher_phrase(cipher, phrasestr))) != 0) {
        phraselen = 0;
        words = 0;
        fails = 0;
        /*mysetit = mbegin;
        advance(mysetit, std::rand() % setlen);
        word = *mysetit;*/
        word = "FIVE";
        phrase[words]= word;
        phraselen = word.length();
        while((phraselen+words) < clen) {
            ++words;
            lword = word;
            word = random_element(mm, lword);
            if(word.length() == 0  ||(phraselen + words + word.length()) > clen) {
                --words;
                word = lword;
                if(fails == 5) {
                     break;
                }
                ++fails;
                continue;
            }
            phrase[words]= word;
            phraselen += word.length();
        }
        phrasestr = pad_phrase(phrase, words, phraselen, clen);
        if(++attempts %10000 == 0) {
            fprintf(stderr, "Hashes: %i, Skips: %i, str: %s(%i)\n", attempts, skips, phrasestr.c_str(), phrasestr.length());
        }
    }
    std::cout << phrasestr << std::endl;

}


void test(const string PADDING, const string crypted, const string MD5) {
    string WORDLIST[] = {"O", "AND", "SECS", "CLOCK", "PAST", "AFTER", "MEASUREMENT", "IS", "AT", "SHARP", "PM", "MINUNTE", "SECOND",
"ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN", "SIXTEEN", "EVENTEEN", "EIGHTEEN", "NINETEEN", "TWENTY",
/*
"TWENTYONE", "TWENTYTWO", "TWENTYTHREE", "TWENTYFOUR", "TWENTYFIVE", "TWENTYSIX", "TWENTYSEVEN", "TWENTYEIGHT", "TWENTYNINE",
*/
"THIRTY",
/*
"THIRTYONE", "THIRTYTWO", "THIRTYTHREE", "THIRTYFOUR", "THIRTYFIVE", "THIRTYSIX", "THIRTYSEVEN", "THIRTYEIGHT", "THIRTYNINE",
*/
"FORTY",
/*
"FORTYONE", "FORTYTWO", "FORTYTHREE", "FORTYFOUR", "FORTYFOUR", "FORTYFIVE", "FORTYSIX", "FORTYSEVEN", "FORTYEIGHT", "FORTYNINE",
*/
"FIFTY",
/*
"FIFTYONE", "FIFTYTWO", "FIFTYTHREE", "FIFTYFOUR", "FIFTYFIVE", "FIFTYSIX", "FIFTYSEVEN", "FIFTYEIGHT", "FIFTYNINE",
*/
// SIXTY
};
    string cipher = crypted;
    int clen = cipher.length();
    int wlen = sizeof(WORDLIST) / sizeof(WORDLIST[0]);

    //printf("cipher: %s(%i)\n", cipher.c_str(), clen);
    //printf("words: (%i)\n", wlen);
    int count = 100000;
    while(count --> 0) {
        cipher = md5(cipher);
    }
    //printf("%s\n", md5(cipher).c_str());
    string randword = random_element(WORDLIST);
    //printf("%s %s\n", randword.c_str(), md5(randword).c_str());
    string keypart =  get_key_for_search("WJYDJZ" , "SECOND");
    //printf("Keypart: %s, Expected: %s\n", keypart.c_str(), "EFWPWW");

    char **letters;
    letters = new char*[clen];
    int i, j, k;
    for(i = 0; i < clen; i++) {
        letters[i]= new char[26];
    }
    string s;
    int word_len;
    int found;
    for(i = 0; i<clen; ++i) {
        found = 0;
        for(j = 0; j < wlen; ++j) {
            word_len = WORDLIST[j].length(); 
            if(word_len + i >= clen)
                continue;
            found=1;
            s = get_key_for_search(crypted.substr(i, word_len), WORDLIST[j]);
            for(k = 0; k< word_len; k++) {
                letters[i+k][s[k]-'A'] = 1;
                //letters[i+k][WORDLIST[j][k]-'A'] = 1;
            }
        }
        if(!found)
            break;
    }
    for(i = 0; i<clen; i++) {
        for(j=0; j<26; j++) {
            if(letters[i][j] >0) {
                printf("%c", j+'A');
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

}

multimap <string, string> mm_init() {
    // http://www.cplusplus.com/reference/map/multimap/equal_range/
    // Plan: iterate through all keys, hashing final string.
    //  padding: after every stage, all variants to str length
    //  next(terms[], next, max)
    multimap <string, string> mm;
    for(string str : { "SECONDS", "MINUTES", "AND", "O", /*"IS"*/ }) {
        mm.insert(pair<const string, string>("ONE", str));
        mm.insert(pair<const string, string>("TWO", str));
        mm.insert(pair<const string, string>("THREE", str));
        mm.insert(pair<const string, string>("FOUR", str));
    }
    for(string str : { "SECONDS", "MINUTES", "AND", "O" }) {
        mm.insert(pair<const string, string>("FIVE", str));
        mm.insert(pair<const string, string>("SIX", str));
        mm.insert(pair<const string, string>("SEVEN", str));
        mm.insert(pair<const string, string>("EIGHT", str));
        mm.insert(pair<const string, string>("NINE", str));
        mm.insert(pair<const string, string>("TEN", str));
        mm.insert(pair<const string, string>("ELEVEN", str));
        mm.insert(pair<const string, string>("TWELVE", str));
    }
    for(string str : { "SECONDS", "MINUTES", "AND" }) {
        mm.insert(pair<const string, string>("THIRTEEN", str));
        mm.insert(pair<const string, string>("FOURTEEN", str));
        mm.insert(pair<const string, string>("FIFTEEN", str));
        mm.insert(pair<const string, string>("SIXTEEN", str));
        mm.insert(pair<const string, string>("SEVENTEEN", str));
        mm.insert(pair<const string, string>("EIGHTEEN", str));
        mm.insert(pair<const string, string>("NINETEEN", str));
    }
    for(string str : { "SECONDS", "MINUTES", "AND", "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", }) {
        mm.insert(pair<const string, string>("TWENTY", str));
        mm.insert(pair<const string, string>("THIRTY", str));
        mm.insert(pair<const string, string>("FOURTY", str));
        mm.insert(pair<const string, string>("FIFTY", str));
        //mm.insert(pair<const string, string>("SIXTY", str));
    }
    for(string str : { "CLOCK", "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE" })
        mm.insert(pair<const string, string>("O", str));

    for(string str : {
            "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN",
            "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN",  "SIXTEEN",  "SEVENTEEN",
            "EIGHTEEN", "NINETEEN", "TWENTY",   "THIRTY",   "FOURTY",   "FIFTY",/*    "SIXTY",*/
        }) mm.insert(pair<const string, string>("AND", str));
    for(string str : {
            "AND",// "SHARP",
            "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN",
            "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN",  "SIXTEEN",  "SEVENTEEN",
            "EIGHTEEN", "NINETEEN", "TWENTY",   "THIRTY",   "FOURTY",   "FIFTY",    /*"SIXTY",*/
        }) mm.insert(pair<const string, string>("CLOCK", str));
    for(string str : { "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE" }) {
        //mm.insert(pair<const string, string>("AFTER", str));
        //mm.insert(pair<const string, string>("PAST", str));
    }
    for(string str : {
            "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN",
            "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN",  "SIXTEEN",  "SEVENTEEN",
            "EIGHTEEN", "NINETEEN", "TWENTY",   "THIRTY",   "FOURTY",   "FIFTY",    /*"SIXTY",*/
        }) mm.insert(pair<const string, string>("AT", str));
    //mm.insert(pair<const string, string>("IS", "AT"));
    //mm.insert(pair<const string, string>("SHARP", ""));
    //mm.insert(pair<const string, string>("PM", ""));
    //for(string str : { "ONE", "TWO", "THREE", "FOUR" })
    //    mm.insert(pair<const string, string>("MEASUREMENT", str));
    for(string str : {
             "AND",// "AFTER", "PAST",
             "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN",
             "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN",  "SIXTEEN",  "SEVENTEEN",
             "EIGHTEEN", "NINETEEN", "TWENTY",   "THIRTY",   "FOURTY",   "FIFTY",    /*"SIXTY", */
        }) mm.insert(pair<const string, string>("MINUTES", str));
    for(string str : { "AND",/* "PAST", "AFTER",*/ })
        mm.insert(pair<const string, string>("SECONDS", str));

    return mm;
}

