/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Olaf Krzikalla 2004-2007.
// (C) Copyright Ion Gazta�aga  2006.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#include <boost/intrusive/ilist.hpp>
#include <boost/intrusive/detail/pointer_to_other.hpp>
#include "itestvalue.hpp"
#include "smart_ptr.hpp"
#include <vector>
#include <boost/intrusive/value_traits_type.hpp>

// Boost.Test
#include "boost/test/included/test_exec_monitor.hpp"

using namespace boost::intrusive;

template<class ValueTraits>
struct test_list 
{
   typedef typename ValueTraits::value_type value_type;
   static void test_all(std::vector<value_type>& values);
   static void test_front_back(std::vector<value_type>& values);
   static void test_sort(std::vector<value_type>& values);
   static void test_insert(std::vector<value_type>& values);
   static void test_swap(std::vector<value_type>& values);
};

template<class ValueTraits>
void test_list<ValueTraits>::test_all(std::vector<typename ValueTraits::value_type>& values)
{
   test_front_back(values);
   test_sort(values);
   test_insert(values);
   test_swap(values);
}

//test: push_front, pop_front, push_back, pop_back, front, back, size, empty:
template<class ValueTraits>
void test_list<ValueTraits>
   ::test_front_back(std::vector<typename ValueTraits::value_type>& values)
{
   typedef boost::intrusive::ilist
      <ValueTraits
      ,ValueTraits::value_type::constant_time_size, std::size_t 
      > list_type;
   list_type testlist;
   BOOST_CHECK (testlist.empty());

   testlist.push_back (values[0]);
   BOOST_CHECK (testlist.size() == 1);
   BOOST_CHECK (&testlist.front() == &values[0]);
   BOOST_CHECK (&testlist.back() == &values[0]);

   testlist.push_front (values[1]);
   BOOST_CHECK (testlist.size() == 2);
   BOOST_CHECK (&testlist.front() == &values[1]);
   BOOST_CHECK (&testlist.back() == &values[0]);

   testlist.pop_back();
   BOOST_CHECK (testlist.size() == 1);
   const list_type &const_testlist = testlist;
   BOOST_CHECK (&const_testlist.front() == &values[1]);
   BOOST_CHECK (&const_testlist.back() == &values[1]);

   testlist.pop_front();
   BOOST_CHECK (testlist.empty());
}  


//test: constructor, iterator, reverse_iterator, sort, reverse:
template<class ValueTraits>
void test_list<ValueTraits>
   ::test_sort(std::vector<typename ValueTraits::value_type>& values)
{
   typedef typename ValueTraits::value_type testvalue_t;
   boost::test_tools::output_test_stream test_seq;
   typedef boost::intrusive::ilist
      <ValueTraits
      ,ValueTraits::value_type::constant_time_size, std::size_t 
      > list_type;
   list_type testlist(values.begin(), values.end());

   std::copy (testlist.begin(), testlist.end(), 
               std::ostream_iterator<testvalue_t> (test_seq));
   BOOST_CHECK (test_seq.is_equal ("12345"));

   testlist.sort (even_odd());
   std::copy (testlist.rbegin(), testlist.rend(), 
               std::ostream_iterator<testvalue_t> (test_seq));
   BOOST_CHECK (test_seq.is_equal ("53142"));

   testlist.reverse();
   //test postincrement:
   for (BOOST_DEDUCED_TYPENAME list_type::iterator i = 
         testlist.begin(), e = testlist.end(); i != e; i++) 
      test_seq << *i;
   BOOST_CHECK (test_seq.is_equal ("53142"));
}  
  
//test: assign, insert, const_iterator, const_reverse_iterator, erase, current:
template<class ValueTraits>
void test_list<ValueTraits>
   ::test_insert(std::vector<typename ValueTraits::value_type>& values)
{
   typedef typename ValueTraits::value_type testvalue_t;
   boost::test_tools::output_test_stream test_seq;
   typedef boost::intrusive::ilist
      <ValueTraits
      ,ValueTraits::value_type::constant_time_size, std::size_t 
      > list_type;
   list_type testlist;
   testlist.assign (&values[0] + 2, &values[0] + 5);

   const list_type& const_testlist = testlist;
   std::copy (const_testlist.begin(), const_testlist.end(), 
               std::ostream_iterator<testvalue_t> (test_seq));
   BOOST_CHECK (test_seq.is_equal ("345"));

   BOOST_DEDUCED_TYPENAME list_type::iterator i = ++testlist.begin();
   BOOST_CHECK (i->value_ == 4);

   testlist.insert (i, values[0]);
   std::copy (const_testlist.rbegin(), const_testlist.rend(), 
               std::ostream_iterator<testvalue_t> (test_seq));
   BOOST_CHECK (test_seq.is_equal ("5413"));

   i = testlist.current (values[4]);
   BOOST_CHECK (&*i == &values[4]);

   i = testlist.erase (i);
   BOOST_CHECK (i == testlist.end());

   //test postincrement:
   for (BOOST_DEDUCED_TYPENAME list_type::const_iterator i = 
         const_testlist.begin(), e = const_testlist.end(); i != e; i++) 
      test_seq << *i;
   BOOST_CHECK (test_seq.is_equal ("314"));
}  

//test: insert (seq-version), swap, splice, erase (seq-version):
template<class ValueTraits>
void test_list<ValueTraits>
   ::test_swap(std::vector<typename ValueTraits::value_type>& values)
{
   typedef typename ValueTraits::value_type testvalue_t;
   boost::test_tools::output_test_stream test_seq;
   typedef boost::intrusive::ilist
      <ValueTraits
      ,ValueTraits::value_type::constant_time_size, std::size_t 
      > list_type;

   {
      list_type testlist1 (&values[0], &values[0] + 2);
      list_type testlist2;
      testlist2.insert (testlist2.end(), &values[0] + 2, &values[0] + 5);
      testlist1.swap (testlist2);
      std::copy (testlist1.begin(), testlist1.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("345"));
      std::copy (testlist2.begin(), testlist2.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("12"));

      testlist2.splice (++testlist2.begin(), testlist1);
      std::copy (testlist2.begin(), testlist2.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("13452"));
      BOOST_CHECK (testlist1.empty());

      testlist1.splice (testlist1.end(), testlist2, ++(++testlist2.begin()));
      std::copy (testlist1.begin(), testlist1.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("4"));
      std::copy (testlist2.begin(), testlist2.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("1352"));

      testlist1.splice (testlist1.end(), testlist2, 
                        testlist2.begin(), ----testlist2.end());
      std::copy (testlist1.begin(), testlist1.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("413"));
      std::copy (testlist2.begin(), testlist2.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("52"));

      testlist1.erase (testlist1.current(values[0]), testlist1.end());
      BOOST_CHECK (testlist1.size() == 1);
      BOOST_CHECK (&testlist1.front() == &values[3]);
   }
   {
      boost::test_tools::output_test_stream test_seq;
      list_type testlist1 (&values[0], &values[0] + 2);
      list_type testlist2 (&values[0] + 3, &values[0] + 5);

      values[0].swap_nodes(values[2]);
      std::copy (testlist1.begin(), testlist1.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("32"));

      values[2].swap_nodes(values[4]);
      std::copy (testlist1.begin(), testlist1.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("52"));
      std::copy (testlist2.begin(), testlist2.end(), 
                  std::ostream_iterator<testvalue_t> (test_seq));
      BOOST_CHECK (test_seq.is_equal ("43"));
   }
}  

template<class VoidPointer, bool constant_time_size>
class test_main_template
{
   public:
   int operator()()
   {
      typedef testvalue<VoidPointer, constant_time_size> testvalue_t;
      std::vector<testvalue_t> data (5);
      for (int i = 0; i < 5; ++i)
         data[i].value_ = i + 1; 

      test_list <typename testvalue_t::list_base_hook::template
         value_traits<testvalue_t> >::test_all(data);

      test_list <typename testvalue_t::list_member_hook::template
            value_traits<&testvalue_t::list_node_> >::test_all(data);


//   test_list <boost::intrusive::ilist_ptr_to_member
//      < testvalue_t        , typename testvalue_t::pointer
//      , &testvalue_t::list_prev_, &testvalue_t::list_next_> >::test_all(data);

      return 0;
   }
};

template<class VoidPointer>
class test_main_template<VoidPointer, false>
{
   public:
   int operator()()
   {
      typedef testvalue<VoidPointer, false> testvalue_t;
      std::vector<testvalue_t> data (5);
      for (int i = 0; i < 5; ++i)
         data[i].value_ = i + 1; 

      test_list <typename testvalue_t::list_base_hook::template
         value_traits<testvalue_t> >::test_all(data);

      test_list <typename testvalue_t::list_member_hook::template
            value_traits<&testvalue_t::list_node_> >::test_all(data);

//      test_list <boost::intrusive::ilist_ptr_to_member
//         < testvalue_t        , typename testvalue_t::pointer
//         , &testvalue_t::list_prev_, &testvalue_t::list_next_> >::test_all(data);

      test_list <typename testvalue_t::list_auto_base_hook::template
         value_traits<testvalue_t> >::test_all(data);

      test_list <typename testvalue_t::list_auto_member_hook::template
            value_traits<&testvalue_t::list_auto_node_> >::test_all(data);
      return 0;
   }
};

//Explicit instantiations of non-counted classes
template class boost::intrusive::ilist<list_base_raw>;
template class boost::intrusive::ilist<list_member_raw>;
template class boost::intrusive::ilist<list_auto_base_raw>;
template class boost::intrusive::ilist<list_auto_member_raw>;
//template class boost::intrusive::ilist<list_ptr_to_member_raw>;
template class boost::intrusive::ilist<list_base_smart>;
template class boost::intrusive::ilist<list_member_smart>;
template class boost::intrusive::ilist<list_auto_base_smart>;
template class boost::intrusive::ilist<list_auto_member_smart>;
//template class boost::intrusive::ilist<list_ptr_to_member_smart>;

//Explicit instantiation of counted classes
template class boost::intrusive::ilist<list_base_raw_t, true>;
template class boost::intrusive::ilist<list_member_raw_t, true>;
//template class boost::intrusive::ilist<list_ptr_to_member_raw_t, true>;
template class boost::intrusive::ilist<list_base_smart_t, true>;
template class boost::intrusive::ilist<list_member_smart_t, true>;
//template class boost::intrusive::ilist<list_ptr_to_member_smart_t, true>;

int test_main( int, char* [] ) 
{
   test_main_template<void*, false>()();
   test_main_template<boost::intrusive::smart_ptr<void>, false>()();
   test_main_template<void*, true>()();
   test_main_template<boost::intrusive::smart_ptr<void>, true>()();
   return 0;
}