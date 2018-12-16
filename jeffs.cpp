/*

The challenge

We have provided a file called “wordsforproblem.txt” which contains a
sorted list of approximately 173,000 words.  The words are listed one
word per line, do not contain spaces, and are all lowercase.

Your task is to write a program that reads the file and provides the following:

❖ the longest concatenated word (that is,the longest word that is
  comprised entirely of shorter words in the file)

❖ the 2nd longest concatenated word

❖ the total count of all the concatenated words in the file

For example, if the file contained:
cat
cats
catsdogcats
dog
dogcatsdog
hippopotamuses
rat
ratcatdogcat

the longest concatenated word would be 'ratcatdogcat' with 12
characters.  ‘hippopotamuses’ is a longer word, however it is not
comprised entirely of shorter words in the list. The 2nd longest
concatenated word is ‘catsdogcats’ with 11 characters.  The total
number of concatenated words is 3.  Note that ‘cats’ is not a
concatenated word because there is no word ‘s’ in the list.

*/

#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>

using std::cin;
using std::cout;
using std::endl;
using std::get;
using std::ifstream;
using std::istream_iterator;
using std::istringstream;
using std::make_pair;
using std::pair;
using std::queue;
using std::size_t;
using std::string;
using std::tuple;
using std::unordered_set;

// generate a lookup table for our target alphabet
constexpr char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
unsigned char LUT[256]{};
int setupLUT()
{
  unsigned char v{};
  for (auto x : alphabet)
    LUT[(size_t)x] = v++;
  return v;
}
int ignored = setupLUT();

inline unsigned char index(char c) { return LUT[(size_t)c]; }

struct concatenated_word_challenge {
  // result type, longest word, second longest word, total count
  using result = tuple<string, string, size_t>;

  // range constructor
  template <class Itr> concatenated_word_challenge(Itr b, Itr e)
  {
    for (; b != e; ++b) {
      add_word(*b);
    }
  }

  // process the input data and generate the result
  result operator()() { return process_work_queue(); }

private:
  using bag_of_words = unordered_set<string>;
  using suffix_queue = queue<pair<string, string>>;

  // non copyable/movable or assignable
  concatenated_word_challenge(const concatenated_word_challenge &) = delete;
  concatenated_word_challenge(concatenated_word_challenge &&) = delete;
  concatenated_word_challenge &
  operator=(const concatenated_word_challenge &) = delete;
  concatenated_word_challenge &
  operator=(concatenated_word_challenge &&) = delete;

  // add the word to the trie, if we encounter an existing term along
  // the way enqueue the existing word plus the remainder of the
  // target word as a prefix suffix pair in our work queue

  void add_word(const string &word)
  {
    auto p = &root;
    for (auto i = 0u; i < word.size(); ++i) {
      if (!p->children[index(word[i])]) {
        p->children[index(word[i])] = new trie_node;
      }

      if (p->terminal) {
        // work queue item, prefix the portion of the target word that matches
        // an existing
        // trie entry, suffix the remainder of the target word
        Q.push(make_pair(word.substr(0, i), word.substr(i)));
      }
      p = p->children[index(word[i])];
    }
    p->terminal = true;
  }

  // process the entire work queue and return the longest word
  // comprised of concatenated terms, the second longest word and the
  // total number of words comprised of concatenated terms

  result process_work_queue()
  {
    string first_longest;   // longest word
    string second_longest;  // second longest word
    bag_of_words all_words; // all words comprised of concatenated terms seen

    while (Q.size()) {
      auto &q = Q.front();
      const auto &prefix = q.first;
      const auto &suffix = q.second;

      // if the suffix is found in the trie then this is a potential
      // word in our result set, it is comprised of a concatentation
      // of existing trie entries

      if (exists_in_trie(suffix)) {
        auto word = prefix + suffix;

        // add the new term only once to our bag of words that are
        // concatenations
        all_words.insert(word);

        // if new word is bigger than our existing saved off results, update our
        // saved results
        if (word.size() > first_longest.size()) {
          second_longest = first_longest;
          first_longest = word;
        }
        // avoid suplicate of first word clobbering second longest word
        else if (word.size() > second_longest.size() && word != first_longest) {
          second_longest = word;
        }
      }

      // for each of the words contained in the suffix which exist in
      // the trie form a new work queue item with a new prefix term
      // that is the concatenation of the existing prefix and the new
      // term, and a suffix which is the old suffix less the newly
      // appended prefix term

      for (const auto &pr : all_prefixes_of(suffix)) {
        Q.push(make_pair(prefix + pr, suffix.substr(pr.size())));
      }

      Q.pop();
    }
    return make_tuple(first_longest, second_longest, all_words.size());
  }

  // check if word exists in the trie,
  bool exists_in_trie(const string &word)
  {
    auto p = &root;
    for (auto c : word) {
      // word is not found
      if (!p->children[index(c)]) {
        return false;
      }

      p = p->children[index(c)];
    }
    // found if our end position is a word marker
    return p->terminal;
  }

  // find all prefix strings for a word which exist in the trie
  bag_of_words all_prefixes_of(const string &word)
  {
    auto p = &root;
    bag_of_words prefixes;
    string prefix;
    // iterate through the trie using the target word
    for (auto c : word) {

      // if the character doesnt exist in the trie along this path,
      // there are no more prefixes to add
      if (!p->children[index(c)]) {
        break;
      }

      p = p->children[index(c)];

      // append the new character to our running prefix
      prefix += c;

      // if the running prefix has encountered a new word in the trie,
      // add it to our bag of prefixes
      if (p->terminal) {
        prefixes.insert(prefix);
      }
    }
    return prefixes;
  }

  struct trie_node {
    trie_node *children[26]{};
    bool terminal{};
    ~trie_node()
    {
      for (auto x : children)
        delete x;
    }
  };

  trie_node root;
  suffix_queue Q;
};

// process the given range of input strings and print out our resuts
template <class Itr> void run(Itr b, Itr e)
{
  auto result = concatenated_word_challenge{b, e}();

  cout << "longest word " << get<0>(result) << " length "
       << get<0>(result).size() << endl;
  cout << "second longest word " << get<1>(result) << " length "
       << get<1>(result).size() << endl;
  cout << "count " << get<2>(result) << endl;
}

// small sanity test
void test()
{
  istringstream SS{
      "cat cats catsdogcats dog dogcatsdog hippopotamuses rat ratcatdogcat"};
  run(istream_iterator<string>{SS}, istream_iterator<string>{});
}

// if run with an argument of --test, perform simple sanity test above
// if run with a single argument attempt to open a file given by the argument as
// an input word list
// if run without arguments generate the input word list from stdin
int main(int argc, char *argv[])
{
  using std::operator""s;

  if (argc == 2 && argv[1] == "--test"s) {
    test();
  }
  else if (argc == 2) {
    ifstream F{argv[1]};
    run(istream_iterator<string>{F}, istream_iterator<string>{});
  }
  else {
    run(istream_iterator<string>{cin}, istream_iterator<string>{});
  }
}
