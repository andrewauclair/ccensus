#ifndef LOC_UTILS_H
#define LOC_UTILS_H

#include <string_view>
#include <string>

inline bool is_blank(std::string_view string)
{
	for (auto ch : string)
	{
		if (ch != ' ' && ch != '\t') return false;
	}
	return true;
}

inline bool is_single_comment(std::string_view string)
{
	char prev_ch = ' ';

	for (auto ch : string)
	{
		if (ch == '/' && prev_ch == '/') return true;

		prev_ch = ch;
	}
	return false;
}

inline bool starts_with(std::string_view str, std::string_view start)
{
    if (str.length() >= start.length())
    {
        return str.compare(0, start.length(), start) == 0;
    }
    return false;
}

inline bool ends_with(std::string_view str, std::string_view ending)
{
    if (str.length() >= ending.length())
    {
        return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
    }
    return false;
}

inline bool contains(std::string_view str, std::string_view search)
{
	return str.find(search) != std::string_view::npos;
}

enum class Block_Comment_Type
{
	None, // there is no block comment
	Inline_Comment, // there is a block comment, but it's contained within this line
	Opening // there is a opening block comment and the closing block comment is on a future line
};

inline Block_Comment_Type is_opening_block_comment(std::string_view string)
{
	bool in_quotes = false;
	char prev_ch = ' ';
	Block_Comment_Type type = Block_Comment_Type::None;

	for (auto ch : string)
	{
		if (type == Block_Comment_Type::None && !in_quotes && (ch == '\'' || ch == '"')) in_quotes = true;
		else if (in_quotes && (ch == '\'' || ch == '"') && prev_ch != '\\') in_quotes = false;
		else if (!in_quotes && prev_ch == '/' && ch == '*') type = Block_Comment_Type::Opening;
		else if (!in_quotes && prev_ch == '*' && ch == '/') type = Block_Comment_Type::Inline_Comment;

		prev_ch = ch;
	}

	return type;
}

// this function will find a closing block comment of the form */ in the string
// only called when we know we're inside of a block comment
inline bool is_closing_block_comment(std::string_view string)
{
	char prev_ch = ' ';

	for (auto ch : string)
	{
		if (prev_ch == '*' && ch == '/') return true;

		prev_ch = ch;
	}

	return false;
}

inline bool is_code_and_comment(std::string_view string)
{
	auto type = is_opening_block_comment(string);
	if (!is_single_comment(string) && type == Block_Comment_Type::None && !is_closing_block_comment(string))
	{
		return false;
	}

	bool non_whitespace = false;
	bool ignore = false;
	char prev_ch = ' ';

	for (auto ch : string)
	{
		if (ch != ' ' && ch != '\t' && ch != '/' && ch != '*' && !ignore)
		{
			non_whitespace = true;
		}

		if (!ignore && prev_ch == '/' && ch == '/')
		{
			return non_whitespace;
		}

		if (!ignore && prev_ch == '/' && ch == '*' && type == Block_Comment_Type::Opening)
		{
			return non_whitespace;
		}
		
		if (!ignore && prev_ch == '/' && ch == '*')
		{
			ignore = true;
		}
		else if (ignore && prev_ch == '*' && ch == '/')
		{
			ignore = false;
		}
		
		prev_ch = ch;
	}
	return non_whitespace;
}

#endif