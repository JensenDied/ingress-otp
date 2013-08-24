#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <set>

#include <time.h>
#include <map>
#include <vector>
#include <algorithm>
#include "md5.h"
#include "permutation_base.h"

using std::advance;
using std::iterator;
using std::multimap;
using std::pair;
using std::set;
using std::string;
using std::vector;

// Ideas
// Random strings based on valid keyspace for letters
// Random assembling of words and padding and xor'ing against the encrypted_string and hashing the result
// Markov chain words and the above.
multimap <string, string> mm_init();
string get_key_for_encrypted_string_phrase(const string phrase);

static unsigned long long attempts;
static unsigned int skips;
static char *keybuff;
static char *padbuff;
static string encrypted_string;
static const char *encrypted_string_c;
static unsigned int encrypted_string_len;
static MD5 OTP_md5;
static unsigned int phraselen;
static unsigned int words;
static string *phrase;
static vector <string> num;
static string phrasestr;
static vector< vector <vector  <int> > > base_permutations;

inline void checkphrase(const char *str) {
    if(++attempts %500000 == 0) {
        fprintf(stderr, "Hashes: %llu, Skips: %i, str: %s\n", attempts, skips, padbuff);
    }
    if(OTP_md5 == MD5(get_key_for_encrypted_string_phrase(str))) {
        printf("Found at Hash(%llu)\n"
                "phrase: %s\n"
                "MD5:    %s\n"
                "OTP:    %s\n", attempts, str, OTP_md5.hexdigest().c_str(), keybuff);
        exit(0);
    }
}


vector<vector<int>> get_permutation_base(int val) {
    int max = 30;
    if(val > max) {
        fprintf(stderr, "[WW] Requested permutation val(%i) not supported\n", val);
        val = 1;
    }
    return base_permutations[val];
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
    int number = sizeof(elements) / sizeof(elements[0]);
    return elements[std::rand()%number];
}

string get_key_for_encrypted_string_phrase(const string phrase) {
    //C:WJYDJZ
    //K:EFWPWW
    //S:SECOND
    //string rot;
    char c;
    for(unsigned int i = 0; i<encrypted_string_len; ++i) {
        c = encrypted_string[i] - (phrase[i] - 'A');
        if(c < 'A')
            c+=26;
        keybuff[i]= c;
    }
    return keybuff;
}

void pad_check_phrase(const string phrase[], const unsigned int words, const unsigned int plen) {
    int budget = encrypted_string_len - plen - words+1; //only required to fill spaces between words
    if(budget < 0) {
        ++skips;
        return;
    }
    unsigned int i, j, k, l;
    int base_s = words+1;
    //if(budget > base_s)
        //base_s = budget;
    vector<int> base(base_s);
    for(vector<int> vv : get_permutation_base(budget)) {
        std::fill(base.begin(), base.end(),0);
        std::copy(vv.begin(), vv.begin() + std::min(vv.size(), base.size()) , base.begin());
        if(base[words] != 0) {
            ++skips; // Skip permutations with more elements than words
            continue;
        }
        std::sort(base.begin(), base.end());
        do {
            memset(padbuff, 'X', encrypted_string_len);
            k = 0;
            // i word counter / padarr pos
            // k padbuff position
            // l = phrase[i] length
            // j = misc
            for(i = 0; i < words; ++i) {
                k += base[i];
                l = phrase[i].length();
                for(j = 0; j < l; ++j, ++k) {
                    padbuff[k] = phrase[i][j];
                }
                if(i+1 < words) { // Are there going to be more words?
                    ++k;
                }
            }
            k += base[words];
            /* // This seems to have been resolved
            if(k != encrypted_string_len) {
                fprintf(stderr, "[ww] Possible buffer permutation issue, budget: %i, k: %i\n", budget, k);
                for(unsigned int a = 0; a < base.size(); ++a) {
                    fprintf(stderr, "[%i]", base[a]);
                }
                fprintf(stderr, "\n");
            } */
            checkphrase(padbuff);
        } while(std::next_permutation(base.begin(), base.end()));
    }
}

void nopad_check_phrase(const string phrase[], const unsigned int words, const unsigned int plen) {
    int budget = encrypted_string_len - plen; 
    if(budget < 0) {
        ++skips;
        return;
    }
    unsigned int i, j, k, l;
    int base_s = words+1;
    //if(budget > base_s)
        //base_s = budget;
    vector<int> base(base_s);
    for(vector<int> vv : get_permutation_base(budget)) {
        std::fill(base.begin(), base.end(),0);
        std::copy(vv.begin(), vv.begin() + std::min(vv.size(), base.size()) , base.begin());
        if(base[words] != 0) {
            ++skips; // Skip permutations with more elements than words
            continue;
        }
        std::sort(base.begin(), base.end());
        do {
            memset(padbuff, 'X', encrypted_string_len);
            k = 0;
            // i word counter / padarr pos
            // k padbuff position
            // l = phrase[i] length
            // j = misc
            for(i = 0; i < words; ++i) {
                k += base[i];
                l = phrase[i].length();
                for(j = 0; j < l; ++j, ++k) {
                    padbuff[k] = phrase[i][j];
                }
            }
            k += base[words];
            /* // This seems to have been resolved
            if(k != encrypted_string_len) {
                fprintf(stderr, "[ww] Possible buffer permutation issue, budget: %i, k: %i\n", budget, k);
                for(unsigned int a = 0; a < base.size(); ++a) {
                    fprintf(stderr, "[%i]", base[a]);
                }
                fprintf(stderr, "\n");
            } */
            checkphrase(padbuff);
        } while(std::next_permutation(base.begin(), base.end()));
    }
}

// FIVE O CLOCK TWO  MINUTE(S) AND TWENTY SEVEN SECOND(S)
void format_1(int hour, int max_hour) {
    for(; hour < max_hour; hour++)
        for(int minute = 0; minute <= 5; minute++)
            for(int second = 0; second < 60; second++) {
                if(minute == 5 and second >0)
                    break;
                words = 0;
                phraselen = 0;
                string word = num[hour]; phrase[words] = word; ++words; phraselen += word.length();
                word = "O"; phrase[words] = word; ++words; phraselen += word.length();
                word = "CLOCK"; phrase[words] = word; ++words; phraselen += word.length();
                if(minute > 20 && (minute%10) != 0) {
                    word = num[minute - minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[minute]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "MINUTES";
                if(minute == 1)
                    word = "MINUTE";
                phrase[words] = word; ++words; phraselen += word.length();

                word = "AND"; phrase[words] = word; ++words; phraselen += word.length();
                if(second > 20 && (second%10) != 0) {
                    word = num[second - second%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[second%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[second]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "SECONDS"; phrase[words] = word; ++words; phraselen += word.length();
                pad_check_phrase(phrase, words, phraselen);
            }
}

// ONE  O CLOCK FOUR MINUTE(S)     FIFTY  EIGHT SECOND(S)
void format_2(int hour, int max_hour) {
    for(; hour < max_hour; hour++)
        for(int minute = 0; minute <= 5; minute++)
            for(int second = 0; second < 60; second++) {
                if(minute == 5 and second >0)
                    break;
                words = 0;
                phraselen = 0;
                string word = num[hour]; phrase[words] = word; ++words; phraselen += word.length();
                word = "O"; phrase[words] = word; ++words; phraselen += word.length();
                word = "CLOCK"; phrase[words] = word; ++words; phraselen += word.length();
                if(minute > 20 && (minute%10) != 0) {
                    word = num[minute - minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[minute]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "MINUTES";
                if(minute == 1)
                    word = "MINUTE";
                phrase[words] = word; ++words; phraselen += word.length();
                if(second > 20 && (second%10) != 0) {
                    word = num[second - second%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[second%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[second]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "SECONDS"; phrase[words] = word; ++words; phraselen += word.length();
                pad_check_phrase(phrase, words, phraselen);
            }
}

//SIX O CLOCK AND FOUR MINUTES AND THIRTEEN SECONDS
void format_3(int hour, int max_hour) {
    for(; hour < max_hour; hour++)
        for(int minute = 0; minute <= 5; minute++)
            for(int second = 0; second < 60; second++) {
                if(minute == 5 and second >0)
                    break;
                words = 0;
                phraselen = 0;
                string word = num[hour]; phrase[words] = word; ++words; phraselen += word.length();
                word = "O"; phrase[words] = word; ++words; phraselen += word.length();
                word = "CLOCK"; phrase[words] = word; ++words; phraselen += word.length();
                word = "AND"; phrase[words] = word; ++words; phraselen += word.length();
                if(minute > 20 && (minute%10) != 0) {
                    word = num[minute - minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[minute]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "MINUTES";
                if(minute == 1)
                    word = "MINUTE";
                phrase[words] = word; ++words; phraselen += word.length();
                word = "AND"; phrase[words] = word; ++words; phraselen += word.length();
                if(second > 20 && (second%10) != 0) {
                    word = num[second - second%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[second%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[second]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "SECONDS"; phrase[words] = word; ++words; phraselen += word.length();
                pad_check_phrase(phrase, words, phraselen);
            }
}

//      SEVENOCLOCK AND THREE MINUTES     AND THIRTYFIVE SEC
void format_12(int hour, int max_hour) {
    for(; hour < max_hour; hour++)
        for(int minute = 0; minute <= 5; minute++)
            for(int second = 0; second < 60; second++) {
                if(minute == 5 and second >0)
                    break;
                words = 0;
                phraselen = 0;
                string word = num[hour]; phrase[words] = word; ++words; phraselen += word.length();
                word = "O"; phrase[words] = word; ++words; phraselen += word.length();
                word = "CLOCK"; phrase[words] = word; ++words; phraselen += word.length();
                word = "AND"; phrase[words] = word; ++words; phraselen += word.length();
                if(minute > 20 && (minute%10) != 0) {
                    word = num[minute - minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[minute]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "MINUTES";
                if(minute == 1)
                    word = "MINUTE";
                phrase[words] = word; ++words; phraselen += word.length();
                word = "AND"; phrase[words] = word; ++words; phraselen += word.length();
                if(second > 20 && (second%10) != 0) {
                    word = num[second - second%10]; phrase[words] = word; ++words; phraselen += word.length();
                    word = num[second%10]; phrase[words] = word; ++words; phraselen += word.length();
                } else {
                    word = num[second]; phrase[words] = word; ++words; phraselen += word.length();
                }
                word = "SEC"; phrase[words] = word; ++words; phraselen += word.length();
                nopad_check_phrase(phrase, words, phraselen);
            }
}

void format_chi() {
//%m MINUTES %s SECONDS PAST THE HOUR
//%m MINUTES %s SECONDS AFTER THE HOUR
//%m MINUTES AND %s SECONDS PAST THE HOUR
//%m MINUTES AND %s SECONDS AFTER THE HOUR
//%m MINUTES %s SECONDS PAST THE HOUR SHARP
//%m MINUTES %s SECONDS AFTER THE HOUR SHARP
//%m MINUTES AND %s SECONDS PAST THE HOUR SHARP
//%m MINUTES AND %s SECONDS AFTER THE HOUR SHARP
//%s SECONDS %m MINUTES PAST THE HOUR
//%s SECONDS %m MINUTES AFTER THE HOUR
//%s SECONDS AND %m MINUTES PAST THE HOUR
//%s SECONDS AND %m MINUTES AFTER THE HOUR
//%s SECONDS %m MINUTES PAST THE HOUR SHARP
//%s SECONDS %m MINUTES AFTER THE HOUR SHARP
//%s SECONDS AND %m MINUTES PAST THE HOUR SHARP
//%s SECONDS AND %m MINUTES AFTER THE HOUR SHARP
    int minute, second, modval;
    string word;
    for(minute = 0; minute <= 5; minute++) {
        for(second = 0; second < 70; second++) { // 60-69 is the alternate forty / fourty
            if(minute == 5 and second >0)
                break;
            for(modval = 0; modval < 16; ++modval) {
                words = 0;
                phraselen = 0;
                if(modval&8) { // Minutes First
                    if(minute > 20 && (minute%10) != 0) {
                        word = num[minute - minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                        word = num[minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                    } else {
                        word = num[minute]; phrase[words] = word; ++words; phraselen += word.length();
                    }
                    if(minute == 1) {
                        phrase[words] = "MINUTE";
                        phraselen += 6;
                    } else {
                        phrase[words] = "MINUTES";
                        phraselen += 7;
                    }
                    ++words;
                } else {
                    if(second > 20 && (second%10) != 0) {
                        word = num[second - second%10]; phrase[words] = word; ++words; phraselen += word.length();
                        word = num[second%10]; phrase[words] = word; ++words; phraselen += word.length();
                    } else {
                        word = num[second]; phrase[words] = word; ++words; phraselen += word.length();
                    }
                    if(second == 1) {
                        phrase[words] = "SECOND";
                        phraselen += 6;
                    } else {
                        phrase[words] = "SECONDS";
                        phraselen += 7;
                    }
                    ++words;
                }
                if(modval&2) {
                    phrase[words] = "AND";
                    ++words;
                    phraselen += 3;
                }
                if(modval&8) { // Seconds Second
                    if(second > 20 && (second%10) != 0) {
                        word = num[second - second%10]; phrase[words] = word; ++words; phraselen += word.length();
                        word = num[second%10]; phrase[words] = word; ++words; phraselen += word.length();
                    } else {
                        word = num[second]; phrase[words] = word; ++words; phraselen += word.length();
                    }
                    if(second == 1) {
                        phrase[words] = "SECOND";
                        phraselen += 6;
                    } else {
                        phrase[words] = "SECONDS";
                        phraselen += 7;
                    }
                    ++words;
                } else {
                    if(minute > 20 && (minute%10) != 0) {
                        word = num[minute - minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                        word = num[minute%10]; phrase[words] = word; ++words; phraselen += word.length();
                    } else {
                        word = num[minute]; phrase[words] = word; ++words; phraselen += word.length();
                    }
                    if(minute == 1) {
                        phrase[words] = "MINUTE";
                        phraselen += 6;
                    } else {
                        phrase[words] = "MINUTES";
                        phraselen += 7;
                    }
                    ++words;
                }
                if(modval&1) { // 0: PAST, 1: AFTER
                    phrase[words] = "AFTER";
                    phraselen += 5;
                } else {
                    phrase[words] = "PAST";
                    phraselen += 4;
                }
                ++words;
                phrase[words] = "THE";
                phraselen += 3;
                ++words;
                phrase[words] = "HOUR";
                phraselen += 4;
                ++words;
                if(modval&4) { // Ends in Sharp
                    phrase[words] = "SHARP";
                    phraselen += 5;
                    ++words;
                }
                nopad_check_phrase(phrase, words, phraselen);
            }
        }
    }
}
int main(int argc, char **argv) {
    if(argc < 3 || argc > 5) {
        fprintf(stderr, "Usage:\n\t%s encrypted_string md5 [format] [hour]\n", argv[0]);
        exit(1);
    }
    srand ( time(NULL) );
    encrypted_string = argv[1];
    encrypted_string_c = encrypted_string.c_str();
    encrypted_string_len= encrypted_string.length();
    OTP_md5 = MD5(argv[2], 1);
    if(OTP_md5.hexdigest().length() != 32) {
        fprintf(stderr, "Malformed MD5 string: %s\n", OTP_md5.hexdigest().c_str());
        exit(1);
    }
    printf("MD5(OTP).hexdigest(): %s(%zi)\n", OTP_md5.hexdigest().c_str(), OTP_md5.hexdigest().length());
    int n_format = 0;
    if(argc >= 4) {
        n_format = atoi(argv[3]);
        if(n_format > 13) {
            fprintf(stderr, "Max formats: 13\n");
            exit(1);
        }
        if(n_format < 0)
            n_format = 0;
    }
    int hour = 1;
    int max_hour = 12;
    if(argc >= 5) {
        hour = atoi(argv[4]);
        max_hour = hour+1;
    }
    keybuff = new char[encrypted_string_len+1];
    memset(keybuff, 0, encrypted_string_len);
    padbuff = new char[encrypted_string_len+1];
    memset(padbuff, 'X', encrypted_string_len);
    multimap<string,string> mm = mm_init();
    base_permutations = *pb_init();
    // These numbers will largely only be used 1-9 and 10,20,30,40,50. Added variation of fourty/forty as sixty
    num = vector <string>({ "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN", "SIXTEEN", "SEVENTEEN", "EIGHTEEN", "NINETEEN", "TWENTY", "TWENTYONE", "TWENTYTWO", "TWENTYTHREE", "TWENTYFOUR", "TWENTYFIVE", "TWENTYSIX", "TWENTYSEVEN", "TWENTYEIGHT", "TWENTYNINE", "THIRTY", "THIRTYONE", "THIRTYTWO", "THIRTYTHREE", "THIRTYFOUR", "THIRTYFIVE", "THIRTYSIX", "THIRTYSEVEN", "THIRTYEIGHT", "THIRTYNINE", "FORTY", "FORTYONE", "FORTYTWO", "FORTYTHREE", "FORTYFOUR", "FORTYFIVE", "FORTYSIX", "FORTYSEVEN", "FORTYEIGHT", "FORTYNINE", "FIFTY", "FIFTYONE", "FIFTYTWO", "FIFTYTHREE", "FIFTYFOUR", "FIFTYFIVE", "FIFTYSIX", "FIFTYSEVEN", "FIFTYEIGHT", "FIFTYNINE", "FOURTY", "FOURTYONE", "FOURTYTWO", "FOURTYTHREE", "FOURTYFOUR", "FOURTYFIVE", "FOURTYSIX", "FOURTYSEVEN", "FOURTYEIGHT", "FOURTYNINE" });

    set<string> myset;
    for(multimap<string, string>::iterator it = mm.begin(), end = mm.end(); it != end; it = mm.upper_bound(it->first)) {
        myset.insert(it->first);
    }

    //set<string>::iterator mbegin = myset.begin(), mysetit;
    //int setlen = myset.size();
    phraselen = 0;
    words = 0;
    phrase = new string[50];
    phrasestr = "";
    attempts = 0;
    skips=0;
    string word, lword;

    printf("encrypted_string: %s(%i)format(%i)\n", encrypted_string_c, encrypted_string_len, n_format);

    switch(n_format) {
        case 1:     // XOXX*.*CLOCKXX*.*MINUTE.*XANDXX*.*XSECOND
            format_1(hour, max_hour); // FIVE O CLOCK TWO  MINUTE(S) AND TWENTY SEVEN SECOND(S)
            break;
        case 2:
            format_2(hour, max_hour); // ONE  O CLOCK FOUR MINUTE(S)     FIFTY  EIGHT SECOND(S)
            break;
        case 3:
            format_3(hour, max_hour); //SIX O CLOCK AND FOUR MINUTES AND THIRTEEN SECONDS
        case 12:
            format_12(hour, max_hour); //      SEVENOCLOCK AND THREE MINUTES     AND THIRTYFIVE SEC
            break;
        case 13:
            format_chi();
            break;
        default:
            format_1(hour, max_hour); // FIVE O CLOCK TWO  MINUTE(S) AND TWENTY SEVEN SECOND(S)
            format_2(hour, max_hour); // ONE  O CLOCK FOUR MINUTE(S)     FIFTY  EIGHT SECOND(S)
            format_12(hour, max_hour); //      SEVENOCLOCK AND THREE MINUTES     AND THIRTYFIVE SEC
    }
    /*
    format_4(hour); //THREE MINUTES FIFTY TWO SECONDS PAST THREE O CLOCK
    format_5(hour); //TWO MINUTES AND SEVEN SECONDS PAST EIGHT PM
    format_6(hour); //TWO MINUTES AND SEVEN SECONDS PAST EIGHT
    format_7(hour); //MEASUREMENT THREE IS AT NINE O THREE AND THIRTY SECONDS
    format_8(hour); //THIRTEEN SECONDS PAST SEVEN O CLOCK SHARP
    format_9(hour); //FOURTY SEVEN SECONDS AND TWO MINUTES PAST TWO PM
    format_10(hour); // THREE O FOUR AND TWO SECONDS
    format_11(hour); //FIFTY FIVE SECONDS AND THREE MINUTES AFTER SEVEN
    format_13(hour); // TWO MINUTES AND FIFTY TWO SECONDS AFTER  THE HOUR
    */
    fprintf(stderr, "[ii] Not found in %llu attempts with known formats\n", attempts);
    exit(1);
}


multimap <string, string> mm_init() {
    // http://www.cplusplus.com/reference/map/multimap/equal_range/
    // Plan: iterate through all keys, hashing final string.
    //  padding: after every stage, all variants to str length
    //  next(terms[], next, max)
    multimap <string, string> mm;
    for(string str : { "SECONDS", "MINUTES", "AND", "O", "IS" }) {
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
        mm.insert(pair<const string, string>("AFTER", str));
        mm.insert(pair<const string, string>("PAST", str));
    }
    for(string str : {
            "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN",
            "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN",  "SIXTEEN",  "SEVENTEEN",
            "EIGHTEEN", "NINETEEN", "TWENTY",   "THIRTY",   "FOURTY",   "FIFTY",    /*"SIXTY",*/
        }) mm.insert(pair<const string, string>("AT", str));
    mm.insert(pair<const string, string>("IS", "AT"));
    mm.insert(pair<const string, string>("SHARP", ""));
    mm.insert(pair<const string, string>("PM", ""));
    for(string str : { "ONE", "TWO", "THREE", "FOUR" })
        mm.insert(pair<const string, string>("MEASUREMENT", str));
    for(string str : {
             "AND", "AFTER", "PAST",
             "ONE",  "TWO",  "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN",
             "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN",  "SIXTEEN",  "SEVENTEEN",
             "EIGHTEEN", "NINETEEN", "TWENTY",   "THIRTY",   "FOURTY",   "FIFTY",    /*"SIXTY", */
        }) mm.insert(pair<const string, string>("MINUTES", str));
    for(string str : { "AND", "PAST", "AFTER", })
        mm.insert(pair<const string, string>("SECONDS", str));

    return mm;
}

