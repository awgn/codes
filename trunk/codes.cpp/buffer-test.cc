/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <buffer.hh>
#include <colorful.hh>

#include <iterator>
#include <vector>

using namespace more;

typedef colorful< TYPELIST(ecma::bold) > bold;
typedef colorful< TYPELIST(ecma::bold,ecma::fg_red) > red_bold;
typedef colorful< TYPELIST(ecma::reset) > rst;

template <typename T>
void dump_buff(const char *name, const more::buffer<T> &x, bool content = true)
{
    std::cout << bold() << "*** " << name << ": " << rst() << "size()=" << x.size() << " max_size()=" << x.max_size() << " capacity()=" << x.capacity()
              << " reverse_capacity()=" << x.reverse_capacity() << " empty()=" << std::boolalpha << x.empty() << std::endl;

    if (content) {
        std::cout << "    {";
        std::copy(x.begin(), x.end(), std::ostream_iterator<T>(std::cout, ","));
        std::cout << "}" << std::endl;
    }
}


struct object
{
    object() { std::cout << __PRETTY_FUNCTION__  << std::endl;}
    object(const object &) { std::cout << __PRETTY_FUNCTION__  << std::endl;}
    ~object() { std::cout << __PRETTY_FUNCTION__  << std::endl;}
};


int main(int, char *[])
{
    more::buffer<int> abc(10);
    dump_buff("abc", abc);

    std::cout << bold() << "push_back(1,2,3,4)" << rst() << std::endl; 
    
    dump_buff("abc", abc);
    abc.push_back(1); abc.push_back(2); abc.push_back(3); abc.push_back(4);
    dump_buff("abc", abc);

    more::buffer<int> xxx (abc); 
    
    std::cout << bold() << "xxx(abc)" << rst() << std::endl;
    dump_buff("xxx",xxx);

    std::cout << bold() << "yyy(3); yyy = abc:" << rst() << std::endl;
    more::buffer<int> yyy(3); 

    dump_buff("yyy",yyy);
    yyy = abc;
    dump_buff("yyy",yyy);


    std::cout << bold() << "abc[0]=" << abc[0] << rst() << std::endl;
    dump_buff("abc", abc);


    // discard()
    std::cout << bold() << "abc.discard(2)" << rst() << std::endl;
    abc.discard(2);

    std::cout << "abc[0]=" << abc[0] << std::endl;
    dump_buff("abc", abc);

    // commit()
    std::cout << bold() << "abc.commit(2):" << rst() << std::endl;
    more::buffer<int>::iterator t = abc.end();

    * t++ = 5;
    * t++ = 6;

    abc.commit(2);
    dump_buff("abc", abc);

    // clear()
    std::cout << bold() << "abc.clear():" << rst() << std::endl;
    dump_buff("abc", abc);
    abc.clear();
    dump_buff("abc", abc);

    // reset()
    std::cout << bold() << "abc.reset():" << rst() << std::endl;
    dump_buff("abc", abc);
    abc.reset();
    dump_buff("abc", abc);


    // swap()
    std::cout << bold() <<  "abc.swap(xxx):" << rst() << std::endl;    
    
    dump_buff("abc",abc);
    dump_buff("xxx",xxx);
    abc.swap(xxx);
    dump_buff("abc",abc);
    dump_buff("xxx",xxx);

    // push()
    std::cout << bold() << "xxx.push_back(0,1,2)" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.push_back(0);
    xxx.push_back(1);
    xxx.push_back(2);
    dump_buff("xxx",xxx);

    // push_front(): error 
    std::cout << bold() << "xxx.push_front(-1): error" << rst() << std::endl;
    try {
        dump_buff("xxx",xxx);
        xxx.push_front(-1);
    }
    catch(std::exception &e) {
        std::cout << red_bold() << "*** catched exception: " << rst() << e.what() << " [ok]" << std::endl;
        dump_buff("xxx",xxx);
    }

    // pop_back()
    std::cout << bold() << "xxx.pop_back()" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.pop_back();
    dump_buff("xxx",xxx);
  
    // pop_front()
    std::cout << bold() << "xxx.pop_front()" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.pop_front();
    dump_buff("xxx",xxx);
 
    // push_front(): ok
    std::cout << bold() << "xxx.push_front(-1)" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.push_front(-1);
    dump_buff("xxx",xxx);
   

    // erase() test
    xxx.push_back(2);
    xxx.push_back(3);
    xxx.push_back(4);

    std::cout << bold() << "xxx.erase( xxx.begin(), xxx.begin()+1 )" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.erase( xxx.begin(), xxx.begin()+1 );
    dump_buff("xxx",xxx);

    std::cout << bold() << "xxx.erase( xxx.begin()+2, xxx.begin()+4 )" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.erase( xxx.begin()+2, xxx.begin()+ 4 );
    dump_buff("xxx",xxx);

    // operator=
    std::cout << bold() << "operator== :" << rst() << std::endl;
    std::cout << "    xxx==xxx: " << std::boolalpha << (xxx == xxx) << std::endl;
    std::cout << "    xxx==abc: " << std::boolalpha << (xxx == abc) << std::endl;

    // operator <
    std::cout << bold() << "operator< :" << rst() << std::endl;
    std::cout << "    xxx<xxx: " << std::boolalpha << (xxx < xxx) << std::endl;
    std::cout << "    xxx<abc: " << std::boolalpha << (xxx < abc) << std::endl;

    // shift operators...
    std::cout << bold() << "__shift_begin:" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.__shift_begin();
    dump_buff("xxx",xxx);

    std::cout << bold() << "__shift_end:" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.__shift_end();
    dump_buff("xxx",xxx);

    std::cout << bold() << "__shift_center:" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.__shift_center();
    dump_buff("xxx",xxx);

    // insert .......

    std::vector<int> v;
    v.push_back(-2);
    v.push_back(-1);
    v.push_back(0);
   
    std::cout << bold() << "\ninsert FRONT: enough reverse_capacity:" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.insert(xxx.begin(), v.begin(), v.end());
    dump_buff("xxx",xxx);

    std::cout << bold() << "\ninsert FRONT: insufficient reverse_capacity (shift):" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.insert(xxx.begin(), v.begin(), v.end());
    dump_buff("xxx",xxx);

    std::cout << bold() << "\ninsert FRONT: insufficient capacity (failure):" << rst() << std::endl;
    try {
        dump_buff("xxx",xxx);
        xxx.insert(xxx.begin(), v.begin(), v.end());
    } catch(std::exception &e) {
        std::cout << red_bold() << "*** catched exception: " << rst() << e.what() << " [ok]" << std::endl;
        dump_buff("xxx",xxx);
    }

    xxx.reset();
    xxx.insert(xxx.begin(), v.begin(), v.end());
    xxx.__shift_center();

    std::vector<int> pad(5,5);
    
    std::cout << bold() << "\ninsert CENTER: enough capacity:" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.insert(xxx.begin()+1, pad.begin(), pad.end());
    dump_buff("xxx",xxx);

    std::cout << bold() << "\ninsert CENTER: insufficient capacity: (failure)" << rst() << std::endl;
    try {
        dump_buff("xxx",xxx);
        xxx.insert(xxx.begin()+1, pad.begin(), pad.end());
    } catch(std::exception &e) {
        std::cout << red_bold() << "*** catched exception: " << rst() << e.what() << " [ok]" << std::endl;
        dump_buff("xxx",xxx);
    }

    xxx.reset();
    xxx.insert(xxx.begin(), v.begin(), v.end());

    std::cout << bold() << "\ninsert BACK: enough capacity:" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.insert(xxx.end(), pad.begin(), pad.end());
    dump_buff("xxx",xxx);

    xxx.reset();
    xxx.insert(xxx.begin(), v.begin(), v.end());
    xxx.__shift_end();

    std::cout << bold() << "\ninsert BACK:  enough capacity:" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.insert(xxx.end(), pad.begin(), pad.end());
    dump_buff("xxx",xxx);

    std::cout << bold() << "\ninsert BACK: insufficient capacity: (failure)" << rst() << std::endl;
    try {
        dump_buff("xxx",xxx);
        xxx.insert(xxx.end(), pad.begin(), pad.end());
    } catch(std::exception &e) {
        std::cout << red_bold() << "*** catched exception: " << rst() << e.what() << " [ok]" << std::endl;
        dump_buff("xxx",xxx);
    }

    xxx.reset();
    xxx.insert(xxx.begin(), v.begin(), v.end());
    xxx.__shift_end();

    std::cout << bold() << "\ninsert CENTER: enough capacity:" << rst() << std::endl;
    dump_buff("xxx",xxx);
    xxx.insert(xxx.begin()+1, pad.begin(), pad.end());
    dump_buff("xxx",xxx);

}
