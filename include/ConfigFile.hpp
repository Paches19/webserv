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

class ConfigFile
{
	private:
		std::string		_path;
		ConfigFile();
		ConfigFile(const ConfigFile &other);
		ConfigFile &operator=(const ConfigFile &rhs);

	public:		
		ConfigFile(std::string const path);
		~ConfigFile();

		static int checkPath(std::string const path);
		static int checkFile(std::string const path, int mode);
		std::string	readFile(std::string path);
		static int isFileExistAndReadable(std::string const path, std::string const index);
		std::string getPath();
		static int getTypePath(std::string const path);
};

#endif // CONFIGFILE_HPP