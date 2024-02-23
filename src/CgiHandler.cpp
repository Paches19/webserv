/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 12:40:51 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/23 19:06:51 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

//*******************************************************************
// Constructores y destructor de la clase canónica
//*******************************************************************
CgiHandler::CgiHandler(void) { }
CgiHandler::~CgiHandler(void) { }
CgiHandler::CgiHandler(HttpRequest &request, const Location &config, VirtualServers &server)
{
	_body = request.getBody();
	this->_initEnv(request, config, server);
}
CgiHandler::CgiHandler(CgiHandler const	&src)
{
	if (this != &src)
	{
		this->_body = src._body;
		this->_env = src._env;
	}
	return ;
}
CgiHandler&	CgiHandler::operator=(CgiHandler const	&src)
{
	if (this != &src)
	{
		this->_body = src._body;
		this->_env = src._env;
	}
	return *this;
}

//*******************************************************************
// Métodos de la clase
//*******************************************************************
std::string ft_itoa(int n)
{
	std::string str;
	std::stringstream ss;
	ss << n;
	ss >> str;
	return str;
}

void	CgiHandler::_initEnv(HttpRequest &request, const Location &config, VirtualServers &server)
{
	std::map<std::string, std::string>	headers = request.getHeaders();
	// Extract user information from Authorization header
	// Assuming it's a Basic Authentication header
    // Extract user information from the Authorization header and set REMOTE_IDENT and REMOTE_USER
    // You need to implement a function to extract user information based on the authentication scheme
	if (headers.find("Authorization") != headers.end() && !headers["Authorization"].empty()) 
    	this->_env["AUTH_TYPE"] = "Basic"; 

	// Set the remote information
	this->_env["REMOTE_ADDR"] =  inet_ntoa(server.getIpAddress());
	this->_env["REMOTE_PORT"] = ft_itoa(server.getPort());
	this->_env["REMOTE_IDENT"] = headers["Authorization"];
	this->_env["REMOTE_USER"] = headers["Authorization"];

	// Set the server information
	if (headers.find("Host") != headers.end())
		this->_env["SERVER_NAME"] = headers["Host"].substr(0, headers["Host"].find(":"));
	else
		this->_env["SERVER_NAME"] = this->_env["REMOTE_ADDR"];
	this->_env["SERVER_PORT"] = ft_itoa(server.getPort());
	this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	this->_env["SERVER_SOFTWARE"] = "Webserver/1.0";
	
	std::string url = request.getURL();
	// Set the request information
	this->_env["SCRIPT_NAME"] =  url.substr(0, url.find("?"));
	this->_env["SCRIPT_FILENAME"] =  url.substr(0, url.find("?"));
	this->_env["REQUEST_METHOD"] = request.getMethod();
	this->_env["REQUEST_URI"] = url;

	// Set the content information
	this->_env["CONTENT_LENGTH"] = ft_itoa(request.getMethod() == "GET" ? 0 : this->_body.length());
	this->_env["CONTENT_TYPE"] = headers["Content-Type"];

	// Set the path information
	//this->_env["PATH_INFO"] = config.getRootLocation() + config.getPath();
	this->_env["PATH_INFO"] = url.substr(url.find("cgi-bin") + 7, url.find("?") - 8);
	this->_env["PATH_TRANSLATED"] = url.substr(0, url.find("?"));

	// Set the query information
	this->_env["REDIRECT_STATUS"] = "200";
	this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	this->_env["QUERY_STRING"] = url.substr(url.find("?") + 1);	

	const std::vector<std::string>& cgiPathVector = config.getCgiPath();
	for (std::vector<std::string>::const_iterator it = cgiPathVector.begin(); it != cgiPathVector.end(); ++it)
   		this->_env.insert(std::make_pair(server.getRoot() + *it, std::string()));
}

char	**CgiHandler::_getEnvAsCstrArray() const {
	char	**env = new char*[this->_env.size() + 1];

	int	j = 0;
	for (std::map<std::string, std::string>::const_iterator i = this->_env.begin(); i != this->_env.end(); i++) {
		std::string	element;
		if (j == 0)
			element = i->first;
		else
			element = i->first + "=" + i->second;
		env[j] = new char[element.size() + 1];
		env[j] = strcpy(env[j], (const char*)element.c_str());
		j++;
	}
	env[j] = NULL;
	return env;
}

std::string CgiHandler::executeCgi(std::string const scriptName, std::string const pathCGI)
{
	pid_t		pid;
	int			saveStdin;
	int			saveStdout;
	char		**env;
	std::string	newBody;

	try {
		env = this->_getEnvAsCstrArray();
	}
	catch (std::bad_alloc &e) {
		std::cerr << RED << e.what() << RESET << std::endl;
	}

	// SAVING STDIN AND STDOUT IN ORDER TO TURN THEM BACK TO NORMAL LATER
	saveStdin = dup(STDIN_FILENO);
	saveStdout = dup(STDOUT_FILENO);

	FILE	*fIn = tmpfile();
	FILE	*fOut = tmpfile();
	long	fdIn = fileno(fIn);
	long	fdOut = fileno(fOut);
	int		ret = 1;

	write(fdIn, _body.c_str(), _body.size());
	lseek(fdIn, 0, SEEK_SET);

	pid = fork();

	if (pid == -1)
	{
		std::cerr << RED << "Fork crashed." << RESET << std::endl;
		return ("Status: 500\r\n\r\n");
	}
	else if (!pid) // Child process
	{
		dup2(fdIn, STDIN_FILENO);
		dup2(fdOut, STDOUT_FILENO);

		const char* argv[] = { pathCGI.c_str(), NULL, NULL };
		std::cout << "scriptName: " << scriptName << std::endl;
		std::cout << "	pathCGI: " << pathCGI << " <br>" << std::endl;
		size_t i = -1;
		while (env[++i])
		{
			std::cout << "	env[" << i << "]: " << env[i] << " <br>" << std::endl;
		}
		execve(scriptName.c_str(), const_cast<char* const*>(argv), env);
		// If execve fails, it will return here and print an error message
		std::cerr << RED << "Execve crashed." << RESET << std::endl;
		write(STDOUT_FILENO, "Status: 500\r\n\r\n", 15);
	}
	else // Parent process
	{
		char	buffer[CGI_BUFSIZE] = {0};

		waitpid(-1, NULL, 0);
		lseek(fdOut, 0, SEEK_SET);
		ret = 1;
		while (ret > 0)
		{
			memset(buffer, 0, CGI_BUFSIZE);
			ret = read(fdOut, buffer, CGI_BUFSIZE - 1);
			//std::cout << "buffer : " << buffer << std::endl;
			newBody += buffer;
		}
	}

	// TURNING STDIN AND STDOUT BACK TO NORMAL
	dup2(saveStdin, STDIN_FILENO);
	dup2(saveStdout, STDOUT_FILENO);

	fclose(fIn);
	fclose(fOut);
	close(fdIn);
	close(fdOut);
	close(saveStdin);
	close(saveStdout);
	for (size_t i = 0; env[i]; i++)
		delete[] env[i];
	delete[] env;
	if (!pid)
		exit(0);

	return (newBody);
}
