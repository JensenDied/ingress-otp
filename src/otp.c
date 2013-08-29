#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <map>
#include <vector>
#include <algorithm>

#include "md5.h"
#include "permutation_base.h"

// This code looks a lot worse than it is for a few reasons, although it's still not great.
// i moved a lot of local variables to global for speed reasons
// i didn't get around to making this a otp solver class where the globals are members
// it's reasonably fast (single threaded) and supports arguments to shard the work
//   it does not itself know how complex work is so the sharding is the responsibility of the user
// TOODS:
//  convert the associated sum_n.py to c++ and replace permutation_base.c
//  convert global scope to a class / instance
//  mpi support?
//  more formats and variations
//
using std::iterator;
using std::multimap;
using std::pair;
using std::string;
using std::vector;

multimap <string, string> mm_init();
char * get_key_for_encrypted_string_and_padbuff();

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
static int start_hour, end_hour, start_second, start_minute, end_second, end_minute, measurement;

//hashes and compares padbuff with our known OTP md5
inline void check_padbuff()
{
    if(++attempts %500000 == 0) {
        fprintf(stderr, "Hashes: %llu, Skips: %i, str: %s\n", attempts, skips, padbuff);
    }
    if(OTP_md5 == MD5(get_key_for_encrypted_string_and_padbuff(), encrypted_string_len)) {
        printf("Found at Hash(%llu)\n"
                "phrase: %s\n"
                "MD5:    %s\n"
                "OTP:    %s\n", attempts, padbuff, OTP_md5.hexdigest().c_str(), keybuff);
        exit(0);
    }
}

// retrieves our permutations base
// TODO: Dynamically generate on first request
//  sort and save for next_permutation, (sum_n.py, or the vector.push_back returns in the opposite order)
vector<vector<int>> get_permutation_base(int val)
{
    int max = 32;
    if(val > max) {
        fprintf(stderr, "[WW] Requested permutation val(%i) not supported\n", val);
        val = 1;
    }
    return base_permutations[val];
}

//Alpha Rot-N function
char * get_key_for_encrypted_string_and_padbuff()
{
    //C:WJYDJZ
    //K:EFWPWW
    //S:SECOND
    char c;
    for(unsigned int i = 0; i<encrypted_string_len; ++i) {
        c = encrypted_string_c[i] - (padbuff[i] - 'A');
        if(c < 'A')
            c+=26;
        keybuff[i]= c;
    }
    return keybuff;
}

// iterates and permutes all padding combinations for the passed phrase, then checks
void pad_check_phrase(const string phrase[], const unsigned int words, const unsigned int plen)
{
    int budget = encrypted_string_len - plen; 
    if(budget < 0) {
        ++skips;
        return;
    }
    unsigned int i, j, k;
    // There is probably a way to further optimize this so that it only needs to be done on the first invocation of a certain size.
    int base_s = words+1;
    vector<int> base(base_s);
    const char * phrasebuff;
    for(vector<int> vv : get_permutation_base(budget)) {
        std::fill(base.begin(), base.end(),0);
        std::copy(vv.begin(), vv.begin() + std::min(vv.size(), base.size()) , base.begin());
        if(base[words] != 0) {
            ++skips; // Skip permutations with more elements than words
            continue;
        }
        //TODO: Remove when get_permutation_base() returns in the correct order
        std::sort(base.begin(), base.end());
        do {
            // Toggle this if you just want to get a count of how many interations need to be done for a keyspace
            //++attempts; continue
            //The last byte is 0 from initializeation, Xfill the padding so we can skip over it when adding words
            memset(padbuff, 'X', encrypted_string_len);
            k = 0;
            // i word counter / padarr pos
            // k padbuff position
            // j = misc
            for(i = 0; i < words; ++i) {
                k += base[i];
                phrasebuff = phrase[i].c_str();
                for(j = 0; phrasebuff[j]; ++j, ++k) {
                    padbuff[k] = phrasebuff[j];
                }
            }
            k += base[words];
            /* // This seems to have been resolved after i stopped hand generating these
            if(k != encrypted_string_len) {
                fprintf(stderr, "[ww] Possible buffer permutation issue, budget: %i, k: %i\n", budget, k);
                for(unsigned int a = 0; a < base.size(); ++a) {
                    fprintf(stderr, "[%i]", base[a]);
                }
                fprintf(stderr, "\n");
            } */
            check_padbuff();
        } while(std::next_permutation(base.begin(), base.end()));
    }
}


void format_1() // 17.2B
{
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
    for(minute = start_minute; minute <= end_minute; minute++) {
        for(second = start_second; second < end_second; second++) { // 60-69 is the alternate forty / fourty
            if((minute == 5 && second > 0) || (minute > end_minute || (minute >= end_minute && second > end_second)))
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
                pad_check_phrase(phrase, words, phraselen);
            }
        }
    }
}

void format_2()
{
//&0  %h O CLOCK     %m MINUTES     %s SECONDS #Elide seconds at 0
//&1  %h O CLOCK AND %m MINUTES     %s SECONDS
//&2  %h O CLOCK     %m MINUTES AND %s SECONDS
//&3  %h O CLOCK AND %m MINUTES AND %s SECONDS
//&4  %h O CLOCK     %m MINUTES     %s SEC
//&5  %h O CLOCK AND %m MINUTES     %s SEC
//&6  %h O CLOCK     %m MINUTES AND %s SEC
//&7  %h O CLOCK AND %m MINUTES AND %s SEC
//8-15: Reverse Minutes/ Seconds
    int hour, minute, second, modval;
    string word;
    for(hour = start_hour; hour <= end_hour; hour++) {
        int hour_len = num[hour].length();
        phrase[0] = num[hour];
        phrase[1] = "O";
        phrase[2] = "CLOCK";
        for(minute = start_minute; minute <= end_minute; minute++) {
            for(second = start_second; second < end_second; second++) {
                if((minute == 5 && second > 0) || (minute > end_minute || (minute >= end_minute && second > end_second)))
                    break;
                for(modval = 0; modval < 16; ++modval) {
                    phraselen = hour_len + 6;
                    words = 3;
                    if(modval&1) { // First AND
                        phrase[words] = "AND";
                        ++words;
                        phraselen += 3;
                    }
                    if(modval&8) { // Seconds First
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
                        if(second > 0 ) {
                            if(second > 20 && (second%10) != 0) {
                                word = num[second - second%10]; phrase[words] = word; ++words; phraselen += word.length();
                                word = num[second%10]; phrase[words] = word; ++words; phraselen += word.length();
                            } else {
                                word = num[second]; phrase[words] = word; ++words; phraselen += word.length();
                            }
                            if(modval &4) {
                                phrase[words] = "SEC";
                                phraselen += 3;
                            } else {
                                if(second == 1) {
                                    phrase[words] = "SECOND";
                                    phraselen += 6;
                                } else {
                                    phrase[words] = "SECONDS";
                                    phraselen += 7;
                                }
                            }
                            ++words;
                        }
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
                        if(modval &4) {
                            phrase[words] = "SEC";
                            phraselen += 3;
                        } else {
                            if(second == 1) {
                                phrase[words] = "SECOND";
                                phraselen += 6;
                            } else {
                                phrase[words] = "SECONDS";
                                phraselen += 7;
                            }
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
                    pad_check_phrase(phrase, words, phraselen);
                }
            }
        }
    }
}

void format_3() {
//Original:      THIRTEEN  SECONDS   PAST    SEVEN  O  CLOCK   SHARP
//%h O %m AND %s SECONDS
//MEASUREMENT %n  IS  AT  %h  O %m AND %s SECONDS
}

// usage, simple enough
int usage(int argc, char **argv)
{
    fprintf(stderr, "Usage:\n\t%s encrypted_string md5 [options]\n"
        "\n\tThis program accepts the following optional arguments\n"
        "\t[-f format=0]\n"
        "\t[-sh start_hour=1] [-eh end_hour=start_hour]\n"
        "\t[-sm start_minute=0] [-em end_minute=5]\n"
        "\t[-ss start_second=0] [-es end_second=70] (60 is a spelling variation of fourty)\n"
        "\t[-m measurement=]\n"
        , argv[0]);
    return 1;
}

// mostly data init and arg handling until it gets down to the switch on formats
int main(int argc, char **argv)
{
    if(argc < 3) {
        exit(usage(argc, argv));
    }
    encrypted_string = argv[1];
    encrypted_string_c = encrypted_string.c_str();
    encrypted_string_len= encrypted_string.length();
    OTP_md5 = MD5(argv[2], strlen(argv[2]), true);
    start_hour = 1;
    start_second = 0;
    start_minute = 0;
    end_hour = 1;
    end_second = 60;
    end_minute = 5;
    measurement = 0;
    int n_format = 0;
    if(OTP_md5.hexdigest().length() != 32) {
        fprintf(stderr, "Malformed MD5 string: %s\n", OTP_md5.hexdigest().c_str());
        exit(1);
    }
    if(argc >= 4) {
        n_format = atoi(argv[3]);
        if(n_format < 0) {
            fprintf(stderr, "Format must be a positive number, defaulting to all\n");
            n_format = 0;
        }
    }
    int argpos = 3;
    while(argpos < argc) {
        if(strcmp("-f", argv[argpos]) == 0) {
            n_format = atoi(argv[++argpos]);
            if(n_format < 0) n_format = 0;
            if(n_format > 3) n_format = 3;
        } else if(strcmp("-sh", argv[argpos]) == 0) {
            start_hour = atoi(argv[++argpos]);
            if(start_hour < 1) start_hour = 1;
            if(start_hour > 12) start_hour = 12;
            if(start_hour > end_hour) end_hour = start_hour;
        } else if(strcmp("-eh", argv[argpos]) == 0) {
            end_hour = atoi(argv[++argpos]);
            if(end_hour < 1) end_hour = 1;
            if(end_hour > 12) end_hour = 12;
            if(end_hour < start_hour) start_hour = end_hour;
        } else if(strcmp("-sm", argv[argpos]) == 0) {
            start_minute = atoi(argv[++argpos]);
            if(start_minute < 0) start_minute = 0;
            if(start_minute > 70) start_minute = 70; // Extra timefram for misspelled fourty
        } else if(strcmp("-em", argv[argpos]) == 0) {
            end_minute = atoi(argv[++argpos]);
            if(end_minute < 0) end_minute = 0;
            if(end_minute > 70) end_minute = 70; // Extra timefram for misspelled fourty
        } else if(strcmp("-ss", argv[argpos]) == 0) {
            start_second = atoi(argv[++argpos]);
            if(start_second < 0) start_second = 0;
            if(start_second > 70) start_second = 70; // Extra timefram for misspelled fourty
        } else if(strcmp("-es", argv[argpos]) == 0) {
            end_second = atoi(argv[++argpos]);
            if(end_second < 0) end_second = 0;
            if(end_second > 70) end_second = 70; // Extra timefram for misspelled fourty
        } else if(strcmp("-m", argv[argpos]) == 0) {
            measurement = atoi(argv[++argpos]);
            if(measurement < 0) measurement = 0;
            if(measurement > 5) measurement = 5;
        } else {
            exit(usage(argc, argv));
        }
        ++argpos;
    }
    printf("MD5(OTP).hexdigest(): %s(%zi)\n", OTP_md5.hexdigest().c_str(), OTP_md5.hexdigest().length());
    keybuff = new char[encrypted_string_len+1];
    memset(keybuff, 0, encrypted_string_len);
    padbuff = new char[encrypted_string_len+1];
    memset(padbuff, 'X', encrypted_string_len);
    //multimap<string,string> mm = mm_init();
    base_permutations = *pb_init();
    // These numbers will largely only be used 1-9 and 10,20,30,40,50. Added variation of fourty/forty as sixty
    num = vector <string>({ "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN", "SIXTEEN", "SEVENTEEN", "EIGHTEEN", "NINETEEN", "TWENTY", "TWENTYONE", "TWENTYTWO", "TWENTYTHREE", "TWENTYFOUR", "TWENTYFIVE", "TWENTYSIX", "TWENTYSEVEN", "TWENTYEIGHT", "TWENTYNINE", "THIRTY", "THIRTYONE", "THIRTYTWO", "THIRTYTHREE", "THIRTYFOUR", "THIRTYFIVE", "THIRTYSIX", "THIRTYSEVEN", "THIRTYEIGHT", "THIRTYNINE", "FORTY", "FORTYONE", "FORTYTWO", "FORTYTHREE", "FORTYFOUR", "FORTYFIVE", "FORTYSIX", "FORTYSEVEN", "FORTYEIGHT", "FORTYNINE", "FIFTY", "FIFTYONE", "FIFTYTWO", "FIFTYTHREE", "FIFTYFOUR", "FIFTYFIVE", "FIFTYSIX", "FIFTYSEVEN", "FIFTYEIGHT", "FIFTYNINE", "FOURTY", "FOURTYONE", "FOURTYTWO", "FOURTYTHREE", "FOURTYFOUR", "FOURTYFIVE", "FOURTYSIX", "FOURTYSEVEN", "FOURTYEIGHT", "FOURTYNINE" });

    phraselen = 0;
    words = 0;
    phrase = new string[50];
    phrasestr = "";
    attempts = 0;
    skips=0;

    printf("encrypted_string: %s(%i)format(%i)\n", encrypted_string_c, encrypted_string_len, n_format);

    switch(n_format) {
        case 1:
            format_1();
            break;
        case 2:
            format_2();
            break;
        case 3:
            format_3();
            break;
        default:
            format_1();
            format_2();
            format_3();
    }
    fprintf(stderr, "[ii] Not found in %llu attempts with known formats\n", attempts);
    exit(1);
}


// This was going to be used for a markov chain approach before the regularity of niantic formats was apparent
multimap <string, string> mm_init()
{
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

