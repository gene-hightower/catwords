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

#include <array>
#include <iostream>
#include <string>
#include <string_view>

#if __has_include(<tsl/hopscotch_set.h>)
// See <https://github.com/Tessil/hopscotch-map>
#include <tsl/hopscotch_set.h>
using word_set_t = tsl::hopscotch_set<std::string_view>;
#else
#include <unordered_set>
using word_set_t = std::unordered_set<std::string_view>;
#endif

#include <boost/iostreams/device/mapped_file.hpp>

#include <range/v3/all.hpp>

class Words {
public:
  explicit Words(std::string fn);

  static constexpr auto longest_N = 2;

  auto count_of_cat_words() const { return n_cat_words_; }
  auto longest_N_cat_words() const { return longest_cat_words_; }

private:
  inline bool is_word_(std::string_view word) const;
  bool is_cat_word_(std::string_view word) const;

  boost::iostreams::mapped_file_source word_file_;
  word_set_t words_;
  std::string_view::size_type shortest_word_len_;
  std::array<std::string_view, longest_N> longest_cat_words_;
  int n_cat_words_;
};

Words::Words(std::string fn)
{
  word_file_.open(fn);

  auto const word_file_view{
      std::string_view{word_file_.data(), word_file_.size()}};

  // clang-format off
  auto words = word_file_view
             | ranges::view::split('\n')
             | ranges::view::transform([](auto&& rng) {
                                         return std::string_view(&*rng.begin(),
                                                                 ranges::distance(rng));
                                       });
  // clang-format on

  words_.reserve(word_file_.size() / 10); // divide by something close to mean
                                          // word size

  shortest_word_len_ = std::numeric_limits<std::string_view::size_type>::max();
  ranges::for_each(words, [this](auto &&word) {
    words_.emplace(word);
    shortest_word_len_ = std::min(shortest_word_len_, word.length());
  });

  n_cat_words_ = ranges::count_if(words_, [this](auto &&word) {
    if (is_cat_word_(word)) {
      if (auto longest =
              std::find_if(begin(longest_cat_words_), end(longest_cat_words_),
                           [len = word.length()](auto &&cat_word) {
                             return len > cat_word.length();
                           });
          longest != end(longest_cat_words_)) {
        std::rotate(longest, end(longest_cat_words_) - 1,
                    end(longest_cat_words_));
        *longest = word;
      }
      return true;
    }
    return false;
  });
}

inline bool Words::is_word_(std::string_view word) const
{
  return words_.find(word) != words_.end();
}

bool Words::is_cat_word_(std::string_view word) const
{
  for (auto p = shortest_word_len_; p <= (word.length() - shortest_word_len_);
       ++p) {
    auto first = word.substr(0, p);
    auto rest = word.substr(p);
    if (is_word_(first) && (is_word_(rest) || is_cat_word_(rest)))
      return true;
  }
  return false;
}

int main(int argc, char *argv[])
{
  for (int arg = 1; arg < argc; ++arg) {
    Words words(argv[arg]);

    auto const longest = words.longest_N_cat_words();

    std::cout << "longest word " << longest[0] << " length "
              << longest[0].size() << '\n';
    std::cout << "second longest word " << longest[1] << " length "
              << longest[1].size() << '\n';
    std::cout << "count " << words.count_of_cat_words() << '\n';
  }
}
