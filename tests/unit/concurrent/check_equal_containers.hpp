//  Copyright (c) 2016 John Biddiscombe
//  Copyright (c) 2016 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// (C) Copyright Ion Gaztanaga 2004-2013.

#ifndef HPX_CONTAINER_TEST_CHECK_EQUAL_CONTAINER_HPP
#define HPX_CONTAINER_TEST_CHECK_EQUAL_CONTAINER_HPP

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace test {

template <typename T>
struct is_pair
  : std::false_type
{};

template <typename T1, typename T2>
struct is_pair<std::pair<T1, T2> >
  : std::true_type
{};

template <class T1, class T2>
bool CheckEqual(const T1 &t1, const T2 &t2,
    typename std::enable_if<
        !test::is_pair<T1>::value && !test::is_pair<T2>::value
    >::type * = 0)
{
    return t1 == t2;
}

template <class T1, class T2, class C1, class C2>
bool CheckEqualIt(const T1 &i1, const T2 &i2, const C1 &c1, const C2 &c2)
{
    bool c1end = i1 == c1.end();
    bool c2end = i2 == c2.end();
    if (c1end != c2end)
    {
        return false;
    }
    else if (c1end)
    {
        return true;
    }
    else
    {
        return CheckEqual(*i1, *i2);
    }
}

template <class Pair1, class Pair2>
bool CheckEqual(const Pair1 &pair1, const Pair2 &pair2,
    typename std::enable_if<
        test::is_pair<Pair1>::value && test::is_pair<Pair2>::value
    >::type * = 0)
{
    return CheckEqual(pair1.first, pair2.first) &&
        CheckEqual(pair1.second, pair2.second);
}

// Function to check if both containers are equal
template <class ContA, class ContB>
bool CheckEqualContainers(const ContA &cont_a, const ContB &cont_b)
{
    if (cont_a.size() != cont_b.size())
        return false;

    typename ContA::const_iterator itcont_a(cont_a.begin()),
        itcont_a_end(cont_a.end());
    typename ContB::const_iterator itcont_b(cont_b.begin()),
        itcont_b_end(cont_b.end());
    ;
    typename ContB::size_type dist =
        (typename ContB::size_type) std::distance(itcont_a, itcont_a_end);
    if (dist != cont_a.size())
    {
        return false;
    }
    typename ContA::size_type dist2 =
        (typename ContA::size_type) std::distance(itcont_b, itcont_b_end);
    if (dist2 != cont_b.size())
    {
        return false;
    }
    std::size_t i = 0;
    for (; itcont_a != itcont_a_end; ++itcont_a, ++itcont_b, ++i)
    {
        if (!CheckEqual(*itcont_a, *itcont_b))
            return false;
    }
    return true;
}

template <class MyBoostCont, class MyStdCont>
bool CheckEqualPairContainers(
    const MyBoostCont &boostcont, const MyStdCont &stdcont)
{
    if (boostcont.size() != stdcont.size())
        return false;

    typedef typename MyBoostCont::key_type key_type;
    typedef typename MyBoostCont::mapped_type mapped_type;

    typename MyBoostCont::const_iterator itboost(boostcont.begin()),
        itboostend(boostcont.end());
    typename MyStdCont::const_iterator itstd(stdcont.begin());
    for (; itboost != itboostend; ++itboost, ++itstd)
    {
        if (itboost->first != key_type(itstd->first))
            return false;

        if (itboost->second != mapped_type(itstd->second))
            return false;
    }
    return true;
}

}    //namespace test{

#endif
