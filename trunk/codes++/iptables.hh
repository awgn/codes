/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef IPTABLES_HH
#define IPTABLES_HH

#include <iostream>
#include <string>
#include <cstdlib>

template <bool val> struct CTassert;
template <>
struct CTassert<true>{};

template <int v>
struct int2type {
    enum { value = v };
};

namespace ipt {

    struct table {
        enum val { filter=1, nat, mangle, raw
        };
    };
    static const char *table_str[] = {
        NULL, "filter", "nat", "mangle", "raw"
    };

    struct chain {
        enum val { NONE, INPUT, FORWARD, OUTPUT, PREROUTING, POSTROUTING, };
    };
    static const char *chain_str[16] = {
        "", "INPUT", "FORWARD", "OUTPUT", "PREROUTING", "POSTROUTING", };

    struct target {
        enum val {
            ACCEPT=1, DROP, QUEUE, RETURN, CLASSIFY, CLUSTERIP, CONNMARK, CONNSECMARK, DNAT, 
            DSCP, ECN, IPV4OPTSSTRIP, LOG, MARK, MASQUERADE, MIRROR, NETMAP, NFQUEUE, NOTRACK, 
            REDIRECT, REJECT, SAME, SECMARK, SET, SNAT, TARPIT, TCPMSS, TOS, TTL, ULOG
        };
    };
    static const char * target_str[] = {
        NULL, "ACCEPT", "DROP", "QUEUE", "RETURN", "CLASSIFY", "CLUSTERIP", "CONNMARK", "CONNSECMARK", "DNAT",
        "DSCP", "ECN", "IPV4OPTSSTRIP", "LOG", "MARK", "MASQUERADE", "MIRROR", "NETMAP", "NFQUEUE", "NOTRACK",
        "REDIRECT", "REJECT", "SAME", "SECMARK", "SET", "SNAT", "TARPIT", "TCPMSS", "TOS", "TTL", "ULOG"
    };

    struct command {
        enum val {
            APPEND=1, DELETE, INSERT, REPLACE, LIST, FLUSH, ZERO, 
            NEW_CHAIN, DELETE_CHAIN, POLICY, RENAME_CHAIN
        };
    };
    static const char command_char[] = { '\0', 'A', 'D', 'I', 'R', 'L', 'F', 'Z', 'N', 'X', 'P', 'E' };

    // Dummy Actuator Policy...
    //
    struct DummyActuator
    {
        static int exec(command::val c, 
                        const char *table, const char *chain, const char *target,
                        const char *rule,  const char *opt, int rulenum=0)
        {
            char com[1024];
            char num[21];

            sprintf(num, "%d",rulenum);
            sprintf(com, "/sbin/iptables -t %s -%c %s %s %s %s %s %s", table,
                                                       command_char[c], chain,
                                                       ( rulenum ? num : "" ), rule, 
                                                       ( c == command::LIST  || 
                                                         c == command::FLUSH ||
                                                         c == command::ZERO  ||
                                                         c == command::POLICY ? "" : "-j" ), target, opt 
                                                       );
            std::cout << "DummyActuator .:[" << com << "]:.\n";
            return 0;
        }

    };

    // Simple Actuator Policy...
    //
    struct SimpleActuator
    {

        static int exec(command::val c,
                        const char *table, const char *chain, const char *target,
                        const char *rule,  const char *opt, int rulenum=0)
        {
            char com[1024];
            char num[21];

            sprintf(num, "%d",rulenum);
            sprintf(com, "/sbin/iptables -t %s -%c %s %s %s %s %s %s", table,
                                                       command_char[c], chain,
                                                       ( rulenum ? num : "" ), rule, 
                                                       ( c == command::LIST  ||
                                                         c == command::FLUSH ||
                                                         c == command::ZERO  ||
                                                         c == command::POLICY ? "" : "-j" ), target, opt
                                                       );
            int ret = system(com);
            if (ret < 0 || WEXITSTATUS(ret) != 0  )
                std::clog << "system: WEXITSTATUS=" << WEXITSTATUS(ret) << std::endl;

            return WEXITSTATUS(ret);
        }
    };

#ifndef DEFAULT_ACTUATOR
#define DEFAULT_ACTUATOR SimpleActuator
#endif

    template <table::val TABLE, chain::val CHAIN = chain::NONE, typename ACTUATOR = DEFAULT_ACTUATOR >
    class iptables {

        // tables check...
        //
        static void CTtable_check(int2type<table::filter>) {
            CTassert< CHAIN == chain::INPUT         ||
                      CHAIN == chain::OUTPUT        ||
                      CHAIN == chain::FORWARD       
                     > chain_for_table_filter __attribute__((unused));
        }

        static void CTtable_check(int2type<table::nat>) {
            CTassert< CHAIN == chain::PREROUTING    ||
                      CHAIN == chain::OUTPUT        ||
                      CHAIN == chain::POSTROUTING   
                    > chain_for_table_nat __attribute__((unused));
        }

        static void CTtable_check(int2type<table::mangle>) {
            CTassert< CHAIN == chain::PREROUTING    ||
                      CHAIN == chain::OUTPUT        ||
                      CHAIN == chain::INPUT         ||
                      CHAIN == chain::FORWARD       ||
                      CHAIN == chain::POSTROUTING   
                    > chain_for_table_mangle __attribute__((unused));
        }

        static void CTtable_check(int2type<table::raw>) {
            CTassert< CHAIN == chain::PREROUTING   ||
                      CHAIN == chain::OUTPUT       
                    >  chain_for_table_raw __attribute__((unused));
        }

        // targets check...
        //
        static void CTtarget_check(int2type<target::ACCEPT>)  {}
        static void CTtarget_check(int2type<target::DROP>)    {}
        static void CTtarget_check(int2type<target::RETURN>)  {}
        static void CTtarget_check(int2type<target::QUEUE>)   {}
        static void CTtarget_check(int2type<target::NFQUEUE>) {}

        static void CTtarget_check(int2type<target::REJECT>) 
        {
            CTassert< CHAIN == chain::INPUT  || 
                      CHAIN == chain::OUTPUT ||
                      CHAIN == chain::FORWARD > table_for_target __attribute__((unused));
        }

        static void CTtarget_check(int2type<target::CLASSIFY>) 
        {
            CTassert< TABLE == table::mangle > table_for_target __attribute__((unused));
            CTassert< CHAIN == chain::POSTROUTING > chain_for_target __attribute__((unused));
        }

        static void CTtarget_check(int2type<target::CONNSECMARK>) 
        {
            CTassert< TABLE == table::mangle > table_for_target __attribute__((unused));
        }    
        static void CTtarget_check(int2type<target::DNAT>) 
        {
            CTassert< TABLE == table::nat > table_for_target __attribute__((unused));
            CTassert< CHAIN == chain::PREROUTING || 
                      CHAIN == chain::OUTPUT     
                    > chain_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::SNAT>) 
        {
            CTassert< TABLE == table::nat > table_for_target __attribute__((unused));
            CTassert< CHAIN == chain::POSTROUTING 
                    > chain_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::MASQUERADE>) 
        {
            CTassert< TABLE == table::nat > table_for_target __attribute__((unused));
            CTassert< CHAIN == chain::POSTROUTING  
                    > chain_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::NETMAP>) 
        {
            CTassert< TABLE == table::nat > table_for_target __attribute__((unused));
            CTassert< CHAIN == chain::PREROUTING    || 
                      CHAIN == chain::POSTROUTING   || 
                      CHAIN == chain::OUTPUT 
                    > chain_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::REDIRECT>) 
        {
            CTassert< TABLE == table::nat > table_for_target __attribute__((unused));
            CTassert< CHAIN == chain::PREROUTING    || 
                      CHAIN == chain::OUTPUT  
                    > chain_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::SAME>) 
        {
            CTassert< TABLE == table::nat > table_for_target __attribute__((unused));
            CTassert< CHAIN == chain::PREROUTING    || 
                      CHAIN == chain::POSTROUTING  
                    > chain_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::DSCP>) 
        {
            CTassert< TABLE == table::mangle > table_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::ECN>) 
        {
            CTassert< TABLE == table::mangle > table_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::MARK>) 
        {
            CTassert< TABLE == table::mangle > table_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::NOTRACK>) 
        {
            CTassert< TABLE == table::raw > table_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::SECMARK>) 
        {
            CTassert< TABLE == table::mangle > table_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::TOS>) 
        {
            CTassert< TABLE == table::mangle > table_for_target __attribute__((unused));
        }
        static void CTtarget_check(int2type<target::TTL>) 
        {
            CTassert< TABLE == table::mangle > table_for_target __attribute__((unused));
        }
        static void CTtarget_check(...) {
        }

    public:

        template <target::val TARGET>
        static int append(const char *rule, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            CTtarget_check(int2type<TARGET>());
            return ACTUATOR::exec(command::APPEND, table_str[TABLE], chain_str[CHAIN], target_str[TARGET], rule, opt);
        }
        template <const char *TARGET>
        static int append(const char *rule, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            return ACTUATOR::exec(command::APPEND, table_str[TABLE], chain_str[CHAIN], TARGET, rule, opt);
        }

        template <target::val TARGET>
        static int del(const char *rule, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            CTtarget_check(int2type<TARGET>());
            return ACTUATOR::exec(command::DELETE, table_str[TABLE], chain_str[CHAIN], target_str[TARGET], rule, opt);
        }
        template <const char * TARGET>
        static int del(const char *rule, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            return ACTUATOR::exec(command::DELETE, table_str[TABLE], chain_str[CHAIN], TARGET, rule, opt);
        }

        template <target::val TARGET>
        static int insert(const char *rule, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            CTtarget_check(int2type<TARGET>());
            return ACTUATOR::exec(command::INSERT, table_str[TABLE], chain_str[CHAIN], target_str[TARGET], rule, opt);
        }        
        template <const char *TARGET>
        static int insert(const char *rule, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            return ACTUATOR::exec(command::INSERT, table_str[TABLE], chain_str[CHAIN], TARGET, rule, opt);
        }

        template <target::val TARGET>
        static int insert(const char *rule, unsigned int rulenum, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            CTtarget_check(int2type<TARGET>());
            return ACTUATOR::exec(command::INSERT, table_str[TABLE], chain_str[CHAIN], target_str[TARGET], rule, opt, rulenum);
        }

        template <target::val TARGET>
        static int replace(const char *rule, unsigned int rulenum, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            CTtarget_check(int2type<TARGET>());
            return ACTUATOR::exec(command::REPLACE, table_str[TABLE], chain_str[CHAIN], target_str[TARGET], rule, opt, rulenum);
        }
        template <const char * TARGET>
        static int replace(const char *rule, unsigned int rulenum, const char * opt="") {
            CTtable_check(int2type<TABLE>());
            return ACTUATOR::exec(command::REPLACE, table_str[TABLE], chain_str[CHAIN], TARGET, rule, opt, rulenum);
        }

        static int list(const char *opt="")
        {
            return ACTUATOR::exec(command::LIST, table_str[TABLE], chain_str[CHAIN], "", "", opt);
        }

        static int flush(const char *opt="")
        {
            return ACTUATOR::exec(command::FLUSH, table_str[TABLE], chain_str[CHAIN], "", "", opt);
        }

        static int zero(const char *opt="")
        {
            return ACTUATOR::exec(command::ZERO, table_str[TABLE], chain_str[CHAIN], "", "", opt);
        }

        template <target::val TARGET>
        static int policy(const char *opt="") {
            CTtable_check(int2type<TABLE>());
            CTtarget_check(int2type<TARGET>());
            CTassert<CHAIN != chain::NONE> chain __attribute((unused));
            CTassert< TARGET == target::ACCEPT ||
                      TARGET == target::DROP   ||
                      TARGET == target::QUEUE  ||
                      TARGET == target::RETURN 
                    > policy_target __attribute__((unused));

            return ACTUATOR::exec(command::POLICY, table_str[TABLE], chain_str[CHAIN], target_str[TARGET], "", opt ); 
        }

    };
}

#endif /* IPTABLES_HH */
