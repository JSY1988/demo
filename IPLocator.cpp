﻿#include "IPLocator.h"
#include <sstream>
#include <iostream>
using namespace std;

IPLocator::IPLocator(const string &ipdb_name)
{
    unsigned char buf[8];
    ipdb.open(ipdb_name.c_str(), ios::binary);
    if (!ipdb)
    {
        cerr << "文件打开失败" << ipdb_name << endl;
        return;
    }
    ipdb.read((char *)buf, 8);
    first_index = IPLocator::bytes2integer(buf, 4);
    last_index = IPLocator::bytes2integer(buf + 4, 4);
    index_count = (last_index - first_index) / 7 + 1;
}

IPLocator::~IPLocator()
{
    ipdb.close();
}

string IPLocator::getVersion()
{
    string version = this->getIpAddr(0xffffff00);
    std::ostringstream oss;
    oss << this->index_count;
    string total_item(oss.str());
    version = version + " ?????????" + total_item + "??";
    return version;
}

string IPLocator::getIpAddr(const string &ip)
{
    return this->getIpAddr(this->getIpFromString(ip));
}

string IPLocator::getIpAddr(unsigned int ip)
{
    unsigned int M, L = 0, R = this->index_count;
    string addr;

    while (L < R - 1)
    {
        M = (L + R) / 2;
        this->setIpRange(M);
        if (ip == this->cur_start_ip)
        {
            L = M;
            break;
        }
        if (ip > this->cur_start_ip)
            L = M;
        else
            R = M;
    }
    this->setIpRange(L);
    if (ip >= this->cur_start_ip && ip <= this->cur_end_ip)
        addr = this->getAddr(this->cur_start_ip_offset);
    else
        addr = "NULL";
    return addr;
}

string IPLocator::getIpRange(const string &range)
{
    return this->getIpRange(this->getIpFromString(range));
}

string IPLocator::getIpRange(unsigned int range)
{
    this->getIpAddr(range);
    return this->getIpString(this->cur_start_ip) + " - " + this->getIpString(this->cur_end_ip);
}

string IPLocator::getAddr(streamsize offset)
{
    unsigned char byte;
    unsigned char buf[4];
    unsigned int country_offset;
    string country_addr, area_addr;

    this->readFromFile(offset + 4, buf, 4);
    byte = buf[0];
    if (0x01 == byte)
    {
        country_offset = IPLocator::bytes2integer(buf + 1, 3);
        this->readFromFile(country_offset, buf, 4);
        byte = buf[0];
        if (0x02 == byte)
        {
            country_addr = this->readStringFromFile(IPLocator::bytes2integer(buf + 1, 3));
            area_addr = this->getAreaAddr(country_offset + 4);
        }
        else
        {
            country_addr = this->readStringFromFile(country_offset);
            area_addr = this->getAreaAddr(country_offset + country_addr.length() + 1);
        }
    }
    else if (0x02 == byte)
    {
        this->readFromFile(offset + 4 + 1, buf, 3);
        country_offset = IPLocator::bytes2integer(buf, 3);
        country_addr = this->readStringFromFile(country_offset);
        area_addr = this->getAreaAddr(offset + 4 + 4);
    }
    else
    {
        country_addr = this->readStringFromFile(offset + 4);
        area_addr = this->getAreaAddr(offset + 4 + country_addr.length() + 1);
    }

    return country_addr + " " + area_addr;
}

string IPLocator::getAreaAddr(streamsize offset)
{
    unsigned char byte;
    unsigned char buf[4];
    unsigned int p = 0;
    string area_addr;

    this->readFromFile(offset, buf, 4);
    byte = buf[0];
    if (0x01 == byte || 0x02 == byte)
    {
        p = IPLocator::bytes2integer(buf + 1, 3);
        if (p)
            area_addr = this->readStringFromFile(p);
        else
            area_addr = "";
    }
    else
        area_addr = this->readStringFromFile(offset);
    return area_addr;
}

void IPLocator::setIpRange(unsigned int rec_no)
{
    unsigned char buf[7];
    unsigned int offset = first_index + rec_no * 7;
    this->readFromFile(offset, buf, 7);
    this->cur_start_ip = IPLocator::bytes2integer(buf, 4);
    this->cur_start_ip_offset = IPLocator::bytes2integer(buf + 4, 3);
    this->readFromFile(this->cur_start_ip_offset, buf, 4);
    this->cur_end_ip = IPLocator::bytes2integer(buf, 4);
}

void IPLocator::readFromFile(streamsize offset, unsigned char *buf, int len)
{
    ipdb.seekg(offset);
    ipdb.read((char *)buf, len);
}

string IPLocator::readStringFromFile(streamsize offset)
{
    char ch;
    string str;
    ipdb.seekg(offset);
    ipdb.get(ch);
    while (ch)
    {
        str += ch;
        ipdb.get(ch);
    }
    return str;
}

unsigned int IPLocator::getIpFromString(const string &ip)
{
    char *result = NULL;
    unsigned int ret = 0;
    char *s = strdup(ip.c_str());
    result = strtok(s, ".");
    while (result)
    {
        ret <<= 8;
        ret |= (unsigned int)atoi(result);
        result = strtok(NULL, ".");
    }
    free(s);
    return ret;
}

string IPLocator::getIpString(unsigned int ip)
{
    char buf[256];
    sprintf(buf, "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
    string ipstr(buf);
    return ipstr;
}

unsigned int IPLocator::bytes2integer(unsigned char *ip, int count)
{
    int i;
    unsigned int ret;

    if (count < 1 || count > 4)
        return 0;
    ret = ip[0];
    for (i = 0; i < count; i++)
        ret |= ((unsigned int)ip[i]) << (8 * i);
    return ret;
}