/* Copyright (c) 2012 BDT Media Automation GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * confParser.h
 *
 *  Created on: Jul 3, 2012
 *      Author: chento
 */

#ifndef __CONF_PARSER_H__
#define __CONF_PARSER_H__

namespace txtutils
{
    class conf_info_t;
    class conf_parser
    {
    public:
        static conf_parser* get_inst()
        {
            static conf_parser* inst = NULL;
            if (NULL == inst)
            {
                inst = new conf_parser;
            }
            return inst;
        }

		 int init(const char* path, const char* params, void* reserved);

		 int add_entry(const char* file);

		 int add_text_entry(const char* txt);

		 const char* get_value(const char* sec_name, const char* key) const;
		 int get_num_value(const char* sec_name, const char* key) const;

		 void set_value(const char* sec_name, const char* key, const char* value);

        ~conf_parser();

    protected:
        conf_parser();

	private:
        conf_info_t*     conf_info_p_;
    };
}


#define __GET_CONF_PARSER_INST()            txtutils::conf_parser::get_inst()
#define CONF_PARSER_SIMPLE_INIT(conf_file)  __GET_CONF_PARSER_INST()->init(conf_file, NULL, NULL)
#define CONF_PARSER_GET_VAL(sec, key)       __GET_CONF_PARSER_INST()->get_value(sec, key)
#define CONF_PARSER_GET_NUM_VAL(sec, key)   __GET_CONF_PARSER_INST()->get_num_value(sec, key)
#define CONF_PARSER_SET_VAL(sec, key, val)  __GET_CONF_PARSER_INST()->set_value(sec, key, val)

#define CONF_PARSER_ADD_FILE(file)          __GET_CONF_PARSER_INST()->add_entry(file)
#define CONF_PARSER_ADD_TEXT(txt)           __GET_CONF_PARSER_INST()->add_text_e

#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <stdio.h>

namespace txtutils
{
	typedef std::pair<std::string, std::string> str_str_pair;
    typedef std::map<std::string, std::string>  str_str_map;
    typedef str_str_map::const_iterator         str_str_map_citer;

	class conf_info_t
    {
    public:
		static const char* make_inside_key(const char* sec_name, const char* key, std::string& inside_key)
        {
            const std::string MIX_STR = "___#@#___";
            inside_key = std::string(sec_name) + MIX_STR + key;
            return inside_key.c_str();
        }

		const char* get_value(const char* sec_name, const char* key) const
        {
            std::string key_inside;
            make_inside_key(sec_name, key, key_inside);
            str_str_map_citer it = kv_map_.find(key_inside);

            return (kv_map_.end() == it)? "" : it->second.c_str();
        }

        int get_num_value(const char* sec_name, const char* key) const
        {
            std::string key_inside;
            make_inside_key(sec_name, key, key_inside);
            str_str_map_citer it = kv_map_.find(key_inside);

            std::string tmp = (kv_map_.end() == it)? "" : it->second.c_str();
            return atoi(tmp.c_str() );
        }

		void set_value(const char* sec_name, const char* key, const char* value)
        {
            std::string key_inside;
            make_inside_key(sec_name, key, key_inside);
            str_str_map_citer it = kv_map_.find(key_inside);
            kv_map_[key_inside] = value;
        }

        int add_cfg(const char* conf_file);
        int add_txt(const char* txt);

    protected:
        str_str_map         kv_map_;
    };

	struct conf_txt_line_parser
    {
    public:
        conf_txt_line_parser(str_str_map& kv_map) : kv_map_(kv_map) {}

        void operator() (const std::string &line)
        {
            size_t first    = 0;
            size_t last     = 0;

            static const size_t npos                = std::string::npos;
            static const std::string BLANK_STR      = " \t\n";
            static const std::string COMMENT_STR    = "#;";

			static const std::string VAL_END_TAGS[] =
            {
                " #",   " ;",
                "\t#",  "\t;",
            };


			first   = line.find_first_of(COMMENT_STR);
            last    = line.find_first_of("=[");
            if (first < last) return;

			first   = line.find('[');
            last    = line.find(']');
            if (npos != first && npos != last)
            {
                if (first+1 != last)
                    sec_ = line.substr(first+1, last-first-1);
                return;
            }

			str_str_pair key_val_pair;

			first = line.find('=');
            if (npos == first)  return;
            std::string key_part = line.substr(0, first);
            std::string val_part = line.substr(first + 1);

			first = key_part.find_first_not_of(BLANK_STR);
            last  = key_part.find_last_not_of(BLANK_STR);
            if (npos == first || npos == last)  return;
            key_val_pair.first = key_part.substr(first, last+1-first);

			first = val_part.find_first_not_of(BLANK_STR);
            if (npos == first) return;
            size_t flag_size = sizeof(VAL_END_TAGS)/sizeof(VAL_END_TAGS[0]);
            size_t ix = 0;
            do
            {
                last = val_part.find(VAL_END_TAGS[ix], first);
                ++ix;
            } while(ix < flag_size && npos == last);
            if (npos == last) last = val_part.length();
            val_part = val_part.substr(first, last-first);

            last = val_part.find_last_not_of(BLANK_STR);
            if (npos == last) return;
            key_val_pair.second = val_part.substr(0, last+1);

            conf_info_t::make_inside_key(sec_.c_str(), key_val_pair.first.c_str(), key_val_pair.first);
            kv_map_[key_val_pair.first] = key_val_pair.second;
            return;
        }
    private:
        str_str_map&    kv_map_;
        std::string     sec_;
    };


	inline int conf_info_t::add_cfg(const char* conf_file)
    {
        const int MAX_LINE_SIZE = 1024;
        char buf[MAX_LINE_SIZE + 1] = {0};
        FILE* fp = fopen(conf_file, "r");
        if (NULL == fp)
        {
            return -1;
        }

		std::vector<std::string> org_txt;
        while (fgets(buf, MAX_LINE_SIZE, fp) != NULL)
        {
            org_txt.push_back(buf);
        }
        fclose(fp);

		std::for_each(org_txt.begin(), org_txt.end(), conf_txt_line_parser(kv_map_) );
        return 0;
    }

	inline int conf_info_t::add_txt(const char* txt_buff)
    {
        std::vector<std::string> org_txt;
        const char* psz_start = txt_buff;
        const char* psz_find  = txt_buff;

        while (*psz_find != 0)
        {
            while ( *psz_find != 0 && *psz_find != '\r' && *psz_find != '\n')
                ++psz_find;
            org_txt.push_back(std::string(psz_start, psz_find - psz_start) );

			while ( *psz_find != 0 && (*psz_find == '\r' || *psz_find == '\n') )
                ++psz_find;
            psz_start = psz_find;
        }

		std::for_each(org_txt.begin(), org_txt.end(), conf_txt_line_parser(kv_map_) );
        return 0;
    }

	inline conf_parser::conf_parser()
    {
        conf_info_p_ = new conf_info_t;
    }
    inline conf_parser::~conf_parser()
    {
        delete conf_info_p_;
    }

    inline int conf_parser::init(const char* path, const char* params, void* reserved)
    {
        return add_entry(path);
	}

	inline int conf_parser::add_entry(const char* file)
    {
        return conf_info_p_->add_cfg(file);
    }
    inline int conf_parser::add_text_entry(const char* txt)
    {
        return conf_info_p_->add_txt(txt);
    }

    inline const char* conf_parser::get_value(const char* sec_name, const char* key) const
    {
        if (NULL == conf_info_p_)    return NULL;
        return conf_info_p_->get_value(sec_name, key);
    }
    inline int conf_parser::get_num_value(const char* sec_name, const char* key) const
    {
        if (NULL == conf_info_p_)    return -1;
        return conf_info_p_->get_num_value(sec_name, key);
    }

	inline void conf_parser::set_value(const char* sec_name, const char* key, const char* value)
    {
        if (NULL == conf_info_p_)    return;
        return conf_info_p_->set_value(sec_name, key, value);
    }
}

#endif //__CONF_PARSER_H__

