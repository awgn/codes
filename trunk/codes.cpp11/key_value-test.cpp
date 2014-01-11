#include <variant.hpp>
#include <show.hpp>
#include <key_value.hpp>

#include <list>
#include <string>
#include <set>
#include <map>
#include <memory>

namespace key
{
    DECLARE_KEY(unsigned_int);
    DECLARE_KEY(integers);
    DECLARE_KEY(booleans);
    DECLARE_KEY(strings);
    DECLARE_KEY(intset);

    DECLARE_KEY(associative);
    DECLARE_KEY(simple_pair);
    DECLARE_KEY(simple_tuple);

    DECLARE_KEY(ptr_int);
    DECLARE_KEY(shared_int);
    DECLARE_KEY(unique_int);

    DECLARE_KEY(lit_string);
    DECLARE_KEY(int_matrix);

    DECLARE_KEY(doc);

    DECLARE_KEY(int_first);
    DECLARE_KEY(int_second);

    DECLARE_KEY(v_int_string);
}

using namespace more::key_value;

typedef document<

    options<true, '#', '='>,

    key_pair<key::int_first,  int>,
    key_pair<key::int_second, int>

   > int_pair;


typedef document<

    options<false, '#', '='>,

    key_pair<key::unsigned_int, unsigned int>,
    key_pair<key::integers,     std::vector<int>>,
    key_pair<key::booleans,     std::vector<bool>>,
    key_pair<key::strings,      std::list<std::string>>,
    key_pair<key::simple_pair,  std::pair<double, double>>,
    key_pair<key::simple_tuple, std::tuple<bool, int, double, std::string>>,
    key_pair<key::associative,  std::map<std::string, int>>,
    key_pair<key::intset,       std::set<int>>,
    key_pair<key::ptr_int,      int *>,
    key_pair<key::shared_int,   std::shared_ptr<int>>,
    key_pair<key::unique_int,   std::unique_ptr<int>>,
    key_pair<key::lit_string,   const char *>,
    key_pair<key::int_matrix,   std::vector<std::vector<int>>>,
    key_pair<key::doc,          std::vector<int_pair>>,
    key_pair<key::v_int_string, more::variant<int, std::string>>

    > this_document;


template <typename K, typename Par>
void show_value(Par const &doc)
    {
        std::cout << "-> " << K::str() << " = " << show (doc.template get<K>()) << std::endl;
    }

template <typename K, typename Par>
void show_pointer(Par const &doc)
    {
        auto &p = doc.template get<K>();

        std::cout << "-> " << K::str() << " = ";
        if (p)
            std::cout << show (*p) << '(' << show(p) << ')'<< std::endl;
        else 
            std::cout << "nullptr" << std::endl;
    }

int
main(int, char *[])
{
    this_document doc;

    if (!parse("key_value_test.txt", doc))
        exit(1);

    show_value<key::unsigned_int>(doc);
    show_value<key::integers>(doc);
    show_value<key::booleans>(doc);
    show_value<key::strings>(doc);
    show_value<key::simple_pair>(doc);
    show_value<key::simple_tuple>(doc);
    show_value<key::associative>(doc);
    show_value<key::intset>(doc);
    show_value<key::lit_string>(doc);
    show_value<key::int_matrix>(doc);

    show_value<key::v_int_string>(doc);

    show_value<key::doc>(doc);

    show_pointer<key::ptr_int>(doc);
    show_pointer<key::shared_int>(doc);
    show_pointer<key::unique_int>(doc);

    return 0;
}

