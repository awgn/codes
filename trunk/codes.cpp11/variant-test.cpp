#include <variant.hpp>
#include <yats.hpp>

#include <sstream>

using namespace yats;


Context(variant)
{
    Test(simple)
    {
        more::variant<int, std::string> x;
        AssertThrow(x.get<int>());
        AssertThrow(x.get<std::string>());
    }


    Test(set_get)
    {
        more::variant<int, std::string> x;

        x.set<std::string>("hello");
        Assert(x.get<std::string>(), is_equal_to(std::string("hello")));

        x.set<int>(42);
        Assert(x.get<int>(), is_equal_to(42));
    }


    Test(copy_ctor)
    {
        more::variant<int, std::string> x;
        more::variant<int, std::string> y(x);

        Assert(x.empty(), is_true());
        Assert(y.empty(), is_true());
    }


    Test(copy_ctor2)
    {
        more::variant<int, std::string> x(std::string("hello world"));
        more::variant<int, std::string> y(x);

        Assert(x.get<std::string>() == y.get<std::string>());
    }


    Test(move_ctor)
    {
        more::variant<int, std::string> x;
        more::variant<int, std::string> y(std::move(x));

        Assert(x.empty(), is_true());
        Assert(y.empty(), is_true());
    }


    Test(move_ctor2)
    {
        more::variant<int, std::string> x(std::string("hello world"));
        more::variant<int, std::string> y(std::move(x));

        Assert(x.empty());
        Assert(y.get<std::string>() == std::string("hello world"));
    }


    Test(universal_ctor)
    {
        more::variant<int, std::string> x(42);
        more::variant<int, std::string> y(std::string("hello world"));

        Assert(x.get<int>(), is_equal_to(42));
        Assert(y.get<std::string>(), is_equal_to(std::string("hello world")));
    }


    Test(assigment)
    {
        more::variant<int, std::string> x;

        x = std::string("hello");
        Assert(x.get<std::string>(), is_equal_to(std::string("hello")));

        x = 42;
        Assert(x.get<int>(), is_equal_to(42));
    }


    Test(assign_op)
    {
        more::variant<int, std::string> x;
        more::variant<int, std::string> y;

        y = x;

        Assert(x.empty(), is_true());
        Assert(y.empty(), is_true());
    }


    Test(assign_op2)
    {
        more::variant<int, std::string> x(42);
        more::variant<int, std::string> y;

        y = x;

        Assert(x.get<int>(), is_equal_to(42));
        Assert(y.get<int>(), is_equal_to(42));
    }


    Test(assign_op3)
    {
        more::variant<int, std::string> x;
        more::variant<int, std::string> y(42);

        y = x;

        Assert(x.empty(), is_true());
        Assert(y.empty(), is_true());
    }


    Test(assign_op4)
    {
        more::variant<int, std::string> x(std::string("hello world"));
        more::variant<int, std::string> y(42);

        y = x;

        Assert(x.get<std::string>(), is_equal_to(std::string("hello world")));
        Assert(y.get<std::string>(), is_equal_to(std::string("hello world")));
    }


    Test(assign_op5)
    {
        more::variant<int, std::string> x(std::string("hello"));
        more::variant<int, std::string> y(std::string("world"));

        y = x;

        Assert(x.get<std::string>(), is_equal_to(std::string("hello")));
        Assert(y.get<std::string>(), is_equal_to(std::string("hello")));
    }


    Test(move_assign_op)
    {
        more::variant<int, std::string> x;
        more::variant<int, std::string> y;

        y = std::move(x);

        Assert(x.empty(), is_true());
        Assert(y.empty(), is_true());
    }


    Test(move_assign_op2)
    {
        more::variant<int, std::string> x(42);
        more::variant<int, std::string> y;

        y = std::move(x);

        Assert(x.empty(), is_true());
        Assert(y.get<int>(), is_equal_to(42));
    }


    Test(move_assign_op3)
    {
        more::variant<int, std::string> x;
        more::variant<int, std::string> y(42);

        y = std::move(x);

        Assert(x.empty(), is_true());
        Assert(y.empty(), is_true());
    }


    Test(move_assign_op4)
    {
        more::variant<int, std::string> x(std::string("hello world"));
        more::variant<int, std::string> y(42);

        y = std::move(x);

        Assert(x.empty(), is_true());
        Assert(y.get<std::string>(), is_equal_to(std::string("hello world")));
    }


    Test(move_assign_op5)
    {
        more::variant<int, std::string> x(std::string("hello"));
        more::variant<int, std::string> y(std::string("world"));

        y = std::move(x);

        Assert(x.empty(), is_true());
        Assert(y.get<std::string>(), is_equal_to(std::string("hello")));
    }


    Test(op_equal)
    {
        more::variant<int, std::string> x0;
        more::variant<int, std::string> y0;
        Assert(x0 == y0);

        more::variant<int, std::string> x1(42);
        more::variant<int, std::string> y1;
        Assert(x1 != y1);

        more::variant<int, std::string> x2;
        more::variant<int, std::string> y2(std::string("hello"));
        Assert(x2 != y2);

        more::variant<int, std::string> x3(42);
        more::variant<int, std::string> y3(std::string("hello"));
        Assert(x3 != y3);

        more::variant<int, std::string> x4(std::string("hello"));
        more::variant<int, std::string> y4(std::string("world"));
        Assert(x4 != y4);

        more::variant<int, std::string> x5(std::string("hello world"));
        more::variant<int, std::string> y5(std::string("hello world"));
        Assert(x5 == y5);

    }


    struct size_of
    {
        template <typename T>
        size_t operator()(T &elem)
        {
            return sizeof(elem);
        }
    };

    Test(visitor)
    {
        more::variant<int, std::string> x(42);
        more::variant<int, std::string> y(std::string("hello world"));

        auto sx = more::visitor(size_of(), x);
        auto sy = more::visitor(size_of(), y);

        Assert(sx, is_equal_to(sizeof(int)));
        Assert(sy, is_equal_to(sizeof(std::string)));
    }


    Test(stream)
    {
        more::variant<int, std::string> x(42);
        more::variant<int, std::string> y(std::string("hello world"));

        std::ostringstream out;

        out << x;

        Assert(out.str(), is_equal_to(std::string("42")));

        out.str("");

        out << y;

        Assert(out.str(), is_equal_to(std::string("hello world")));
    }

    // struct err
    // {
    //     err(err &&) = delete;
    // };

    // Test(error)
    // {
    //     more::variant<int, err> e;
    // }

    Test(hash)
    {
        int a = 42; std::string b("hello world");

        auto ha = std::hash<int>()(a);
        auto hb = std::hash<std::string>()(b);

        more::variant<int, std::string> v(a);

        auto hv = std::hash<more::variant<int, std::string>>()(v);

        Assert( ha, is_equal_to(hv) );

        v = b;

        hv = std::hash<more::variant<int, std::string>>()(v);

        Assert( hb, is_equal_to(hv) );
    }
}


int
main(int argc, char * argv[])
{
    return yats::run(argc, argv);
}
