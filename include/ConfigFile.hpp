/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:40:51 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:49:55 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

# include "WebServer.hpp"
# include "ConfigFile.hpp"

class ConfigFile
{
	private:
		std::string	_path;

	public:
		ConfigFile();
		ConfigFile(std::string const path);
		ConfigFile(const ConfigFile &other);
		~ConfigFile();
		ConfigFile &operator=(const ConfigFile &rhs);
		
		std::string	getPath();
		static int	getTypePath(std::string const path);

		static int	checkPath(std::string const path);
		static int	checkFile(std::string const path, int mode);
		static std::string	readFile(std::string path);
		static int 	isFileExistAndReadable(std::string const path, std::string const index);
		static bool isDirectory(const std::string& path);
		static bool fileExistsAndReadable(const std::string& filePath);
};

#endif // CONFIGFILE_HPP