#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <set>

#include <time.h>
#include <map>
#include "md5.h"

using std::multimap;
using std::pair;
using std::string;
using std::iterator;
using std::set;
using std::advance;

// Ideas
// Random strings based on valid keyspace for letters
// Random assembling of words and padding and xor'ing against the crypt and hashing the result
// Markov chain words and the above.
void test();
multimap <string, string> mm_init();
string get_key_for_crypt_phrase(const string crypt, const string phrase);

static char *keybuff;
static char *padbuff;
static char *padarr;
static string crypt;
static const char *crypt_c;
static unsigned int crypt_len;
static string MD5;
static const char *MD5_c;

void checkphrase(string str) {
    string  key = get_key_for_crypt_phrase(crypt, str);
    if(MD5 == md5(key)) {
        printf("phrase: %s(%i)\n"
                "MD5:    %s\n"
                "OTP:    %s\n", str.c_str(), str.length(), MD5_c, key.c_str());
        exit(0);
    }
}

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

string get_key_for_crypt_phrase(const string crypt, const string phrase) {
    //C:WJYDJZ
    //K:EFWPWW
    //S:SECOND
    //string rot;
    int len = crypt.length();
    char c;
    for(int i = 0; i<len; i++) {
        c = crypt[i] - (phrase[i] - 'A');
        if(c < 'A')
            c+=26;
        keybuff[i]= c;
    }
    keybuff[len] = 0;
    return keybuff;
}

string pad_phrase(const string phrase[], const unsigned int words, const unsigned int plen) {
    int budget = crypt_len - plen - words+1; //only required to fill spaces between words
    if(budget < 0)
        return "";
    memset(padarr, 0, crypt_len);
    memset(padbuff, 0, crypt_len);
    unsigned int i, j, k, l;
    padarr[0] = budget;

    k = 0;
    // i word counter / padarr pos
    // k padbuff position
    // l = phrase[i] length
    // j = misc
    for(i = 0; i < words; ++i) {
        j = padarr[i];
        while(j --> 0) {
            padbuff[k] = 'X';
            ++k;
        }
        l = phrase[i].length();
        for(j = 0; j < l; ++j, ++k) {
            padbuff[k] = phrase[i][j];
        }
        if(i+1 < words) { // Are there going to be more words?
            padbuff[k] = 'X';
            ++k;
        }
    }
    for(j = padarr[words]; j > 0; --j, ++k) {
        padbuff[k] = 'X';
    }
    return string(padbuff);
}

int main(int argc, char **argv) {
    if(argc == 5) {
        fprintf(stderr, "c: %s, m: %s, p: %s, o: %s\nk2: %s",
                argv[1], argv[2], argv[3], argv[4], get_key_for_crypt_phrase(argv[1], argv[3]).c_str());
        exit(1);
    }
    if(argc != 3) {
        fprintf(stderr, "Usage:\n\t%s crypt md5\n", argv[0]);
        exit(1);
    }
    srand ( time(NULL) );
    crypt = argv[1];
    crypt_c = crypt.c_str();
    crypt_len= crypt.length();
    MD5 = argv[2];
    if(MD5.length() != 32) {
        fprintf(stderr, "Malformed MD5 string\n");
        exit(1);
    }
    MD5_c = MD5.c_str();;
    keybuff = new char[crypt_len+1];
    padbuff = new char[crypt_len+1];
    padarr = new char[crypt_len+1];
    //test();
    multimap<string,string> mm = mm_init();
    string num[] = { "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN", "SIXTEEN", "SEVENTEEN", "EIGHTEEN", "NINETEEN", "TWENTY", "TWENTYONE", "TWENTYTWO", "TWENTYTHREE", "TWENTYFOUR", "TWENTYFIVE", "TWENTYSIX", "TWENTYSEVEN", "TWENTYEIGHT", "TWENTYNINE", "THIRTY", "THIRTYONE", "THIRTYTWO", "THIRTYTHREE", "THIRTYFOUR", "THIRTYFIVE", "THIRTYSIX", "THIRTYSEVEN", "THIRTYEIGHT", "THIRTYNINE", "FORTY", "FORTYONE", "FORTYTWO", "FORTYTHREE", "FORTYFOUR", "FORTYFIVE", "FORTYSIX", "FORTYSEVEN", "FORTYEIGHT", "FORTYNINE", "FIFTY", "FIFTYONE", "FIFTYTWO", "FIFTYTHREE", "FIFTYFOUR", "FIFTYFIVE", "FIFTYSIX", "FIFTYSEVEN", "FIFTYEIGHT", "FIFTYNINE", };

    set<string> myset;
    for(multimap<string, string>::iterator it = mm.begin(), end = mm.end(); it != end; it = mm.upper_bound(it->first)) {
        myset.insert(it->first);
    }

    set<string>::iterator mbegin = myset.begin(), mysetit;
    //int setlen = myset.size();
    unsigned int phraselen = 0;
    unsigned int words = 0;
    string phrase[50];
    string phrasestr = "";
    unsigned int attempts = 0, skips=0;
    string word, lword;
    unsigned int fails = 0;

    printf("crypt: %s(%i)\n", crypt_c, crypt_len);

    for(int hour = 1; hour < 12; hour++)
        for(int minute = 0; minute < 60; minute++)
            for(int second = 0; second < 60; second++) {
                /*
                phrasestr = format_1(hour, minute, second); //FIVE O CLOCK TWO MINUTE(S) AND TWENTY SEVEN SECOND(S)
                phrase = format_2(hour, minute, second); //FIFTY FIVE SECONDS AND THREE MINUTES AFTER SEVEN
                phrase = format_3(hour, minute, second); //MEASUREMENT THREE IS AT NINE O THREE AND THIRTY SECONDS
                phrase = format_4(hour, minute, second); //THREE MINUTES FIFTY TWO SECONDS PAST THREE O CLOCK
                phrase = format_5(hour, minute, second); //TWO MINUTES AND SEVEN SECONDS PAST EIGHT PM
                phrase = format_6(hour, minute, second); //TWO MINUTES AND SEVEN SECONDS PAST EIGHT
                phrase = format_7(hour, minute, second); //SIX O CLOCK AND FOUR MINUTES AND THIRTEEN SECONDS
                phrase = format_8(hour, minute, second); //THIRTEEN SECONDS PAST SEVEN O CLOCK SHARP
                */
                words = 0;
                phraselen = 0;
                word = num[hour]; phrase[words] = word; ++words; phraselen += word.length();
                word = "O"; phrase[words] = word; ++words; phraselen += word.length();
                word = "CLOCK"; phrase[words] = word; ++words; phraselen += word.length();
                if(minute > 20 && (minute%10) != 0) {
                    word = num[minute - minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[minute]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "MINUTES"; phrase[words] = word; ++words; phraselen += word.length();
                word = "AND"; phrase[words] = word; ++words; phraselen += word.length();
                if(second > 20 && (second%10) != 0) {
                    word = num[second - second%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[second%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[second]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "SECONDS"; phrase[words] = word; ++words; phraselen += word.length();
                phrasestr = pad_phrase(phrase, words, phraselen);
                if(phrasestr == "")
                    continue;
//printf("phrase: %s(%i)\n" "MD5:    %s\n" "OTP:    %s\n", phrasestr.c_str(), phrasestr.length(), MD5_c, get_key_for_crypt_phrase(crypt, phrasestr).c_str());
                checkphrase(phrasestr);
            }
    fprintf(stderr,"[ii] Format Checks Finished, Starting Random Guessing\n");
    while(MD5.compare(md5(get_key_for_crypt_phrase(crypt, phrasestr))) != 0) {
        phraselen = 0;
        words = 0;
        fails = 0;
        /*mysetit = mbegin;
        advance(mysetit, std::rand() % setlen);
        word = *mysetit;*/
        word = "FIVE";
        phrase[words]= word;
        phraselen = word.length();
        while((phraselen+words) < crypt_len) {
            ++words;
            lword = word;
            word = random_element(mm, lword);
            if(word.length() == 0  ||(phraselen + words + word.length()) > crypt_len) {
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
        phrasestr = pad_phrase(phrase, words, phraselen);
        if(++attempts %10000 == 0) {
            fprintf(stderr, "Hashes: %i, Skips: %i, str: %s(%i)\n", attempts, skips, phrasestr.c_str(), phrasestr.length());
        }
    }
    std::cout << phrasestr << std::endl;
}


void test() {
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
    int clen = crypt.length();
    int wlen = sizeof(WORDLIST) / sizeof(WORDLIST[0]);

    //printf("crypt: %s(%i)\n", crypt.c_str(), clen);
    //printf("words: (%i)\n", wlen);
    int count = 100000;
    while(count --> 0) {
        crypt = md5(crypt);
    }
    //printf("%s\n", md5(crypt).c_str());
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
            s = get_key_for_search(crypt.substr(i, word_len), WORDLIST[j]);
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

