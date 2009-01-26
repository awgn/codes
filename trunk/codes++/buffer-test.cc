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

#include <vector>

template <typename T>
void dump_buff(const char *name, const more::buffer<T> &x, bool content = true)
{
    std::cout << "*** " << name << ": " << "size()=" << x.size() << " max_size()=" << x.max_size() << " capacity()=" << x.capacity()
              << " reverse_capacity()=" << x.reverse_capacity() << " empty()=" << std::boolalpha << x.empty() << std::endl;

    if (content) {
        std::cout << " {";
        typename more::buffer<T>::const_iterator it = x.begin();
        for(; it != x.end(); ++it) {
            std::cout << *it << ",";
        }
        std::cout << "}";
    }

    std::cout << std::endl;
}


struct object
{
    object() { std::cout << __PRETTY_FUNCTION__  << std::endl;}
    object(const object &) { std::cout << __PRETTY_FUNCTION__  << std::endl;}
    ~object() { std::cout << __PRETTY_FUNCTION__  << std::endl;}
};


int main()
{
    more::buffer<int> abc(10);

    dump_buff("abc", abc);

    std::cout << "    push_back(1,2,3,4)" << std::endl; abc.push_back(1); abc.push_back(2); abc.push_back(3); abc.push_back(4);
    dump_buff("abc", abc);

    more::buffer<int> xxx (abc); std::cout << "xxx(abc)\n";

    dump_buff("xxx",xxx);
    more::buffer<int> yyy(3); std::cout << "yyy = abc: (truncated)\n";

    yyy = abc;
    dump_buff("yyy",yyy);


    std::cout << "abc[0]=" << abc[0] << std::endl;
    dump_buff("abc", abc);

    // discard()
    std::cout << "abc.discard(2)\n";
    abc.discard(2);

    std::cout << "abc[0]=" << abc[0] << std::endl;
    dump_buff("abc", abc);

    // commit()
    more::buffer<int>::iterator t = abc.end();

    * t++ = 5;
    * t++ = 6;

    std::cout << "abc.commit(2)\n";
    abc.commit(2);
    dump_buff("abc", abc);

    // clear()
    std::cout << "abc.clear()\n";
    abc.clear();
    dump_buff("abc", abc);

    // reset()
    std::cout << "abc.reset()\n";
    abc.reset();
    dump_buff("abc", abc);

    // swap()
    std::cout << "abc.swap(xxx)\n";
    abc.swap(xxx);
    dump_buff("abc",abc);
    dump_buff("xxx",xxx);

    // push()
    std::cout << "xxx.push_back(0,1,2)\n";
    xxx.push_back(0);
    xxx.push_back(1);
    xxx.push_back(2);
    dump_buff("xxx",xxx);

    // push_front(): error 
    std::cout << "xxx.push_front(-1): error\n";
    xxx.push_front(-1);
    dump_buff("xxx",xxx);

    // pop_back()
    std::cout << "xxx.pop_back()\n";
    xxx.pop_back();
    dump_buff("xxx",xxx);
  
    // pop_front()
    std::cout << "xxx.pop_front()\n";
    xxx.pop_front();
    dump_buff("xxx",xxx);
 
    // push_front(): ok
    std::cout << "xxx.push_front(-1)\n";
    xxx.push_front(-1);
    dump_buff("xxx",xxx);
   
    // erase() test

    xxx.push_back(2);
    xxx.push_back(3);
    xxx.push_back(4);

    std::cout << "** erase test\n";
    dump_buff("xxx",xxx);
   
    std::cout << "xxx.erase( xxx.begin(), xxx.begin()+1 )\n";
    xxx.erase( xxx.begin(), xxx.begin()+1 );
    dump_buff("xxx",xxx);

    std::cout << "xxx.erase( xxx.begin()+2, xxx.begin()+4 )\n";
    xxx.erase( xxx.begin()+2, xxx.begin()+ 4 );
    dump_buff("xxx",xxx);

    // operator=
    std::cout << "operator== :\n";
    std::cout << "    xxx==xxx: " << std::boolalpha << (xxx == xxx) << std::endl;
    std::cout << "    xxx==abc: " << std::boolalpha << (xxx == abc) << std::endl;

    // operator <
    std::cout << "operator< :\n";
    std::cout << "    xxx<xxx: " << std::boolalpha << (xxx < xxx) << std::endl;
    std::cout << "    xxx<abc: " << std::boolalpha << (xxx < abc) << std::endl;


    // shift operators...

    std::cout << "__shift_begin:\n";
    xxx.__shift_begin();
    dump_buff("xxx",xxx);

    std::cout << "__shift_end:\n";
    xxx.__shift_end();
    dump_buff("xxx",xxx);

    std::cout << "__shift_center:\n";
    xxx.__shift_center();
    dump_buff("xxx",xxx);

    // insert .......

    std::vector<int> v;
    v.push_back(-2);
    v.push_back(-1);
    v.push_back(0);
   
    std::cout << "\ninsert FRONT: enough reverse_capability:\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.begin(), v.begin(), v.end()) << std::endl;
    dump_buff("xxx",xxx);

    std::cout << "insert FRONT: insufficient reverse_capability:\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.begin(), v.begin(), v.end()) << std::endl;
    dump_buff("xxx",xxx);

    std::cout << "insert FRONT: insufficient capability (failure):\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.begin(), v.begin(), v.end()) << std::endl;
    dump_buff("xxx",xxx);

    xxx.reset();
    xxx.insert(xxx.begin(), v.begin(), v.end());
    std::cout << "RESET:";
    xxx.__shift_center();
    dump_buff("xxx",xxx);

    std::vector<int> pad(5,5);
    
    std::cout << "\ninsert CENTER: enough capability:\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.begin()+1, pad.begin(), pad.end()) << std::endl;
    dump_buff("xxx",xxx);

    std::cout << "\ninsert CENTER: insufficient capability:\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.begin()+1, pad.begin(), pad.end()) << std::endl;
    dump_buff("xxx",xxx);

    xxx.reset();
    xxx.insert(xxx.begin(), v.begin(), v.end());
    std::cout << "RESET:";
    dump_buff("xxx",xxx);

    std::cout << "\ninsert BACK: enough capability:\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.end(), pad.begin(), pad.end()) << std::endl;
    dump_buff("xxx",xxx);

    xxx.reset();
    xxx.insert(xxx.begin(), v.begin(), v.end());
    xxx.__shift_end();

    std::cout << "RESET:";
    dump_buff("xxx",xxx);

    std::cout << "insert BACK:  enough capability:\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.end(), pad.begin(), pad.end()) << std::endl;
    dump_buff("xxx",xxx);

    std::cout << "insert BACK: insufficient capability:\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.end(), pad.begin(), pad.end()) << std::endl;
    dump_buff("xxx",xxx);

    xxx.reset();
    xxx.insert(xxx.begin(), v.begin(), v.end());
    xxx.__shift_end();

    std::cout << "RESET:";
    dump_buff("xxx",xxx);

    std::cout << "\ninsert CENTER: enough capability:\n";
    std::cout << "   ret: " << std::boolalpha << xxx.insert(xxx.begin()+1, pad.begin(), pad.end()) << std::endl;
    dump_buff("xxx",xxx);

    // std::cout << "with non-POD types..\n";
    // more::buffer<object> o(2);

}
