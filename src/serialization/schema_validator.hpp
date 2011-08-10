#ifndef SCHEMA_VALIDATOR_HPP
#define SCHEMA_VALIDATOR_HPP

#include "serialization/validator.hpp"
#include "tools/schema/tag.hpp"

#include "config.hpp"
#include "config_cache.hpp"
#include "serialization/parser.hpp"



#include "boost/regex.hpp"

#include <iostream>
#include <queue>
#include <string>
#include <stack>

/** @file
 *  One of the realizations of serialization/validator.hpp abstract validator.
 */
namespace schema_validation{
/**
 * Realization of serialization/validator.hpp abstract validator.
 * Based on stack. Uses some stacks to store different info.
 */
class schema_validator : public abstract_validator{
public:
	schema_validator();
	virtual ~schema_validator();
	/**
	 * Initializes validator from file.
	 * Throws abstract_validator::error if any error.
	 */
	schema_validator(const std::string & filename);
	/**
	 * Prints schema tree to stream
	 * Usually used to testing, whether schema config was read and expanded
	 */
	virtual void open_tag(const std::string & name,int start_line=0,
						   const std::string &file="");
	virtual void close_tag();
	virtual bool validate(const config & cfg, const std::string & name,
							   int start_line, const std::string &file);

private:
// types section
	// Just some magic to ensure zero initialization.
	struct counter{
		int cnt;
		counter(): cnt(0){}
	};
	/**
	 * Counters are mapped by tag name
	 */
	typedef std::map<std::string,counter> cnt_map;

	/**
	 * And counter maps are organize in stack.
	 */
	typedef std::stack<cnt_map> cnt_stack;

	enum message_type{WRONG_TAG,EXTRA_TAG,MISSING_TAG,
					EXTRA_KEY,MISSING_KEY,WRONG_VALUE};
	//error_cache
	/**
	  * Messages are cached.
	  * The reason is next: in file where [tag]...[/tag][+tag]...[/tag]
	  * config object will be validated each [/tag]. So message can be as wrong
	  * (when [+tag] section contains missed elements) as duplicated.
	  *
	  * Messages are mapped by config*. That ensures uniqueness.
	  * Also message-maps are organized in stack to avoid memory leaks.
	  */
	struct message_info{
		message_type type;
		std::string file;
		int line;
		std::string tag;
		std::string key;
		std::string value;
		message_info(message_type t,
				   const std::string& file,
				   int line = 0,
				   const std::string& tag = "",
				   const std::string& key = "",
				   const std::string& value = "")
					   :type(t),file(file),line(line),tag(tag),key(key),
					   value(value){}
	};
	typedef std::deque<message_info> message_list;
	typedef std::map<const config *, message_list> message_map;

	void print(message_info &);
	/**
	 * Reads config from input.
	 */
	bool read_config_file(const std::string & filename);
	/**
	 * Shows, if validator is intialized with schema file;
	 */
	bool config_read_;

	/**
	 * Root of schema information
	 */
	class_tag root_;

	std::stack<const class_tag *> stack_;
	/**
	 * Contains number of children
	 */
	cnt_stack counter_;
	/**
	 * Caches error messages.
	 */
	std::stack<message_map> cache_;
	/**
	 * Type validators.
	 */
	 std::map<std::string,boost::regex> types_;
};
}//namespace schema_validation{

#endif // SCHEMA_VALIDATOR_HPP
