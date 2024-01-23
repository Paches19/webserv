/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:41:07 by adpachec          #+#    #+#             */
/*   Updated: 2024/01/22 12:41:07 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "WebServer.hpp"

class Location
{
	private:
		std::string					_path;
		std::string					_root;
		bool						_autoindex;
		std::string					_index;
		std::vector<short>			_methods; // GET, POST, DELETE, PUT, HEAD
		std::string					_return;
		std::string					_alias;
		std::vector<std::string>	_cgi_path;
		std::vector<std::string>	_cgi_ext;
		unsigned long				_client_max_body_size;

		Location();
		int _checkLocation(Location &location) const;

	public:
		std::map<std::string, std::string> _ext_path;
		Location &operator=(const Location &rhs);
		Location(const Location &other);
		~Location();
		Location(std::string &path, std::vector<std::string> &paramtr, std::string &r);
		// Setters
		void setPath(std::string parametr);
		void setRootLocation(std::string parametr);
		void setMethods(std::vector<std::string> methods);
		void setAutoindex(std::string parametr);
		void setIndexLocation(std::string parametr);
		void setReturn(std::string parametr);
		void setAlias(std::string parametr);
		void setCgiPath(std::vector<std::string> path);
		void setCgiExtension(std::vector<std::string> extension);
		void setMaxBodySize(std::string parametr);
		void setMaxBodySize(unsigned long parametr);
		// Getters
		const std::string &getPath() const;
		const std::string &getRootLocation() const;
		const std::vector<short> &getMethods() const;
		const bool &getAutoindex() const;
		const std::string &getIndexLocation() const;
		const std::string &getReturn() const;
		const std::string &getAlias() const;
		const std::vector<std::string> &getCgiPath() const;
		const std::vector<std::string> &getCgiExtension() const;
		const std::map<std::string, std::string> &getExtensionPath() const;
		const unsigned long &getMaxBodySize() const;
		std::string getPrintMethods() const;
		static void checkToken(std::string &parametr);
		static int ft_stoi(std::string str);
		
		class ErrorException : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorException(std::string message) throw()
				{
					_message = "CONFIG LOCATION ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorException() throw() {}
		};
};

#endif // LOCATION_HPP