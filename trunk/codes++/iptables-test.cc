/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#define DEFAULT_ACTUATOR DummyActuator

#include "iptables.hh"

using namespace ipt;

char MYTARGET[]="MYTARGET";

int main(int argc, char *argv[])
{
    iptables<table::filter, chain::INPUT>::policy<target::ACCEPT>();
    iptables<table::filter, chain::INPUT>::zero();
    iptables<table::filter>::zero();
    iptables<table::filter, chain::INPUT>::flush();
    iptables<table::filter>::flush();

    iptables<table::filter, chain::INPUT>::list();
    iptables<table::filter>::list();

    iptables<table::filter, chain::INPUT>::append<target::ACCEPT>("-p tcp -i eth0");
    iptables<table::filter, chain::INPUT>::append<MYTARGET>("-p udp");

    iptables<table::filter, chain::INPUT>::del<target::ACCEPT>("-p tcp -i eth0");
    iptables<table::filter, chain::INPUT>::del<MYTARGET>("-p udp");

    iptables<table::filter, chain::INPUT>::insert<target::ACCEPT>("-p tcp -i eth0");
    iptables<table::filter, chain::INPUT>::insert<target::ACCEPT>("-p tcp -i eth0", 12);
    iptables<table::filter, chain::INPUT>::insert<MYTARGET>("-p udp");

    iptables<table::filter, chain::INPUT>::replace<target::ACCEPT>("-p tcp -i eth0", 10);
    iptables<table::filter, chain::INPUT>::replace<MYTARGET>("-p tcp -i eth0", 10);

    return 0;
}
