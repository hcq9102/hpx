//  deprecated_include_check implementation  --------------------------------//

//  Copyright Beman Dawes   2002.
//  Copyright Gennaro Prota 2006.
//  Copyright Hartmut Kaiser 2016.
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>
#include <hpx/util/to_string.hpp>

#include <algorithm>

#include "boost/regex.hpp"
#include "deprecated_include_check.hpp"
#include "function_hyper.hpp"

namespace boost { namespace inspect {
    deprecated_includes const names[] = {{"boost/move/move\\.hpp", "utility"},
        {"boost/atomic/atomic\\.hpp", "boost/atomic.hpp"},
        {"boost/thread/locks.hpp", "mutex or shared_mutex"},
        {"boost/type_traits\\.hpp", "type_traits"},
        {"boost/type_traits/([^\\s]*)\\.hpp", "type_traits"},
        {"boost/unordered_map\\.hpp", "unordered_map"},
        {"boost/unordered_set\\.hpp", "unordered_set"},
        {"boost/utility/enable_if\\.hpp", "type_traits"},
        {"boost/detail/atomic_count\\.hpp",
            "hpx/thread_support/atomic_count.hpp"},
        {"boost/function\\.hpp", "hpx/util/function.hpp"},
        {"boost/shared_ptr\\.hpp", "memory"},
        {"boost/intrusive_ptr\\.hpp", "hpx/modules/memory.hpp"},
        {"boost/make_shared\\.hpp", "memory"},
        {"boost/enable_shared_from_this\\.hpp", "memory"},
        {"boost/bind\\.hpp", "hpx/util/bind.hpp"},
        {"boost/(chrono/)?chrono\\.hpp", "chrono"},
        {"boost/(core/)?ref\\.hpp", "functional"},
        {"boost/cstdint\\.hpp", "cstdint"},
        {"boost/thread/barrier\\.hpp", "hpx/util/barrier.hpp"},
        {"boost/exception_ptr\\.hpp", "exception"},
        {"boost/range/iterator_range\\.hpp",
            "hpx/iterator_support/iterator_range.hpp"},
        {"hpx/hpx_fwd\\.hpp", "nothing (remove unconditionally)"},
        {"boost/preprocessor/cat\\.hpp", "hpx/util/detail/pp/cat.hpp"},
        {"boost/preprocessor/stringize\\.hpp",
            "hpx/util/detail/pp/stringize.hpp"},
        {"boost/atomic\\.hpp", "atomic"}, {"boost/nondet_random.hpp", "random"},
        {"boost/random/([^\\s]*)\\.hpp", "random"},
        {"boost/format\\.hpp", "hpx/util/format.hpp"},
        {"boost/regex.hpp", "regex"},
        {"boost/program_options([^\\s]*)\\.hpp", "hpx/program_options\\2.hpp"},
        {"boost/filesystem([^\\s]*)\\.hpp", "hpx/modules/filesystem.hpp"},
        {"boost/lexical_cast\\.hpp",
            "hpx/util/((from_string)|(to_string)).hpp"},
        {"boost/system([^\\s]*)\\.hpp", "system_error"}, {nullptr, nullptr}};

    //  deprecated_include_check constructor  -------------------------------//

    deprecated_include_check::deprecated_include_check()
      : m_errors(0)
    {
        // C/C++ source code...
        register_signature(".c");
        register_signature(".cpp");
        register_signature(".cu");
        register_signature(".cxx");
        register_signature(".h");
        register_signature(".hpp");
        register_signature(".hxx");
        register_signature(".inc");
        register_signature(".ipp");

        for (deprecated_includes const* includes_it = &names[0];
             includes_it->include_regex != nullptr; ++includes_it)
        {
            std::string rx = std::string(R"(^\s*#\s*include\s*<()") +
                includes_it->include_regex + ")>\\s*$" + "|" +
                R"(^\s*#\s*include\s*"()" + includes_it->include_regex +
                ")\"\\s*$";

            regex_data.push_back(
                deprecated_includes_regex_data(includes_it, rx));
        }
    }

    //  inspect ( C++ source files )  ---------------------------------------//

    void deprecated_include_check::inspect(const string& library_name,
        const path& full_path,     // example: c:/foo/boost/filesystem/path.hpp
        const string& contents)    // contents of file to be inspected
    {
        std::string::size_type p = contents.find("hpxinspect:"
                                                 "nodeprecatedinclude");
        if (p != string::npos)
        {
            // ignore this directive here (it is handled below) if it is followed
            // by a ':'
            if (p == contents.size() - 30 ||
                (contents.size() > p + 30 && contents[p + 30] != ':'))
            {
                return;
            }
        }

        std::set<std::string> found_includes;

        // check for all given includes
        for (deprecated_includes_regex_data const& d : regex_data)
        {
            boost::sregex_iterator cur(
                contents.begin(), contents.end(), d.pattern),
                end;
            for (/**/; cur != end; ++cur)
            {
                auto m = *cur;
                if (m[1].matched || m[2].matched)
                {
                    int idx = (m[1].matched ? 1 : 2);

                    // avoid errors to be reported twice
                    std::string found_include(m[1].first, m[1].second);
                    if (found_includes.find(found_include) ==
                        found_includes.end())
                    {
                        std::string tag("hpxinspect:"
                                        "nodeprecatedinclude:" +
                            found_include);
                        if (contents.find(tag) != string::npos)
                            continue;

                        // name was found
                        found_includes.insert(found_include);

                        // include is missing
                        auto it = contents.begin();
                        auto match_it = m[idx].first;
                        auto line_start = it;

                        string::size_type line_number = 1;
                        for (/**/; it != match_it; ++it)
                        {
                            if (string::traits_type::eq(*it, '\n'))
                            {
                                ++line_number;
                                line_start = it + 1;    // could be end()
                            }
                        }

                        ++m_errors;
                        error(library_name, full_path,
                            string(name()) + " deprecated #include (" +
                                found_include + ") on line " +
                                linelink(full_path,
                                    hpx::util::to_string(line_number)) +
                                " use " + m.format(d.data->use_instead) +
                                " instead");
                    }
                }
            }
        }
    }

}}    // namespace boost::inspect
