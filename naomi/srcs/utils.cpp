#include <fstream>
#include <string>
#include <map>
#include <unistd.h>
#include "utils.hpp"


/**
 * send along string + pos
//! needs const ref
 */

std::string	getLine(std::string	str, size_t start) {

	std::string	line;
	size_t		end;

	end = str.find("\r\n", start);
	if (end != std::string::npos)
		line = str.substr(start, end - start);
	return (line);
}

// std::streampos	getFileSize(int	fd) {
	
// 	std::streampos	size;

// 	fd.seekg(0, std::ios::end); // Set the file pointer within the input stream. We want to start at 0 (cause we want to use this to get the length of the file) and set it to the end of the file - which is represented by std::ios::end (which is an enumerator))
// 	size = fd.tellg(); // Tellg will return the position of the file pointer. And we just set it to the end so the location of the file pointer will be the last element, ergo the total filesize.
// 	fd.seekg(0, std::ios::beg); // Set that bad boy to the start of the input stream again
// 	entityBody.resize(size); // Give entityBody enough space that we can read straight into entityBody. This is what ChatGPT did. I'm gonna check later if actually necessary or if can also read into std::string and append that to entityBody. But I think maybe that's not possible because the whole problem with serving images was that I was storing the binary data into std::string but wait entityBody is also std::string. Maybe so we don't have to read line by line?
// 	fd.read(&entityBody[0], size);
// 	fd.close();
// }

/**
 * @brief Splits string into two and returns a map of the two new strings. 
 * The string will be split at location. Location will be the last element of
 * the first substring. After that, look for the first whitespace char. IF it exists,
 * the second substring will start at that location. If it doesn't exist, the second substring
 * will be empty.
 * 
 * @param input 
 * @param location 
 * @return std::map<int, std::string> 
 */

// std::map<int, std::string>	split_string(std::string input, size_t location) {

// 	std::map<int, std::string>	ret;
// 	size_t	start;


// 	ret[0] = input.substr(0, location - 1); // im doing this cause i want to pass along the : as the location, so that i can find the first instance of not whitespace (the value) and check if its npos (if its npos it means there is no entry). It's too messy to try and implement the find_first_of(all the fucking alphanumerical characters)
// 	start = input.find_first_of(' ', location);
// 	ret[1] =
	
// }
