/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adpachec <adpachec@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/16 11:49:23 by adpachec          #+#    #+#             */
/*   Updated: 2024/02/16 12:00:05 by adpachec         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerUtils.hpp"

std::string getMimeType(const std::string& filePath)
{
	size_t dotPos = filePath.rfind('.');
	std::map<std::string, std::string> mimeTypes;

	mimeTypes[".html"] = "text/html";
	mimeTypes[".css"]  = "text/css";
	mimeTypes[".txt"]  = "text/plain";
	mimeTypes[".csv"]  = "text/csv";
	mimeTypes[".htm"]  = "text/html";
	
	mimeTypes[".jpg"]  = "image/jpeg";
	mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".png"]  = "image/png";
	mimeTypes[".gif"]  = "image/gif";
	mimeTypes[".svg"]  = "image/svg+xml";
	mimeTypes[".ico"]  = "image/x-icon";

	mimeTypes[".pdf"]  = "application/pdf";
	mimeTypes[".zip"]  = "application/zip";
	mimeTypes[".tar"]  = "application/x-tar";
	mimeTypes[".gz"]   = "application/gzip";
	mimeTypes[".js"]   = "application/javascript";
	mimeTypes[".json"] = "application/json";
	mimeTypes[".xml"]  = "application/xml";
	mimeTypes[".doc"]  = "application/msword";

	mimeTypes[".mp3"]  = "audio/mpeg";
	mimeTypes[".mp4"]  = "video/mp4";
	mimeTypes[".avi"]  = "video/x-msvideo";
	mimeTypes[".mpeg"] = "video/mpeg";
	mimeTypes[".webm"] = "video/webm";

	if (dotPos != std::string::npos)
	{
		std::string ext = filePath.substr(dotPos);
		if (mimeTypes.count(ext))
			return mimeTypes[ext];
	}
	return "text/plain"; // Tipo MIME por defecto si no se reconoce la extensión
}

bool isValidPath(const std::string& basePath, const std::string& path)
{
	// Prevenir Path Traversal verificando la presencia de ".."
	if (path.find("..") != std::string::npos)
		return false;

	std::string fullPath = path;

	// Asegurarse de que el path no salga del directorio base
	if (fullPath.find(basePath) != 0)
		return false; // El path resultante no está dentro del basePath

	return true; // La ruta es válida y está permitida
}

bool isCGIScript(const std::string& resourcePath)
{
    // Determine if the resource is a CGI script based on some criteria
    // For example, check the file extension or any other condition
    // Return true if it's a CGI script, false otherwise

    // Example: Check if the file extension is ".cgi"
	if (resourcePath.empty())
		return false;
    if (resourcePath.size() >= 4)
	{
		if ((resourcePath.substr(resourcePath.size() - 3) == ".py")  ||
			(resourcePath.substr(resourcePath.size() - 3) == ".sh"))
			{
				std::cout << "    CGI script detected" << std::endl;
				return true;
			}
	}
	return false;
}

std::string getFilename(HttpRequest request, std::string resourcePath)
{
	std::string filename;
	size_t lastSlashPos = request.getURL().find_last_of('/');
	
	if (lastSlashPos != std::string::npos)
		filename = request.getURL().substr(lastSlashPos + 1);
	if (!filename.empty() )
	{
		if (resourcePath.size() < filename.size() ||
			resourcePath.substr(resourcePath.size() - filename.size()) != filename)
				resourcePath += "/" + filename;
	}

	if (resourcePath.size() >= 2 && resourcePath[0] == '/')
		resourcePath = resourcePath.substr(1);
	return resourcePath;

	// Concatenar el nombre del archivo al resourcePath
	//std::string resourcePath = resourcePath + "/" + filename;
	//if (resourcePath.size() >= 2)
	//	resourcePath = &resourcePath[1];
	//return resourcePath;
}

std::string generateDirectoryIndex(const std::string& directoryPath)
{
	std::stringstream html;
	html << "<html>\n<head>\n<title>Index of " << directoryPath << "</title>\n</head>\n";
	html << "<body>\n<h1>Index of " << directoryPath << "</h1>\n";
	html << "<ul>\n";

	DIR* dir = opendir(directoryPath.c_str());
	if (dir != NULL)
	{
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			// Filtra "." y ".."
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			// Construye el enlace al archivo/directorio
			html << "<li><a href=\"" << directoryPath + "/"+entry->d_name << "\">" << entry->d_name << "</a></li>\n";
		}
		closedir(dir);
	}
	else
		html << "<p>Error opening directory.</p>\n";

	html << "</ul>\n</body>\n</html>";
	return html.str();
}

std::string buildResourcePathForPost(HttpRequest& request,
	const Location& location, VirtualServers& server)
{
	std::string requestURL = request.getURL();

	// Eliminar parámetros de consulta
	size_t queryPos = requestURL.find('?');
	if (queryPos != std::string::npos)
		requestURL = requestURL.substr(0, queryPos);

	// Prevenir Path Transversal
	if (requestURL.find("..") != std::string::npos)
		return "";

	std::string basePath = location.getRootLocation().empty() ? server.getRoot() : location.getRootLocation();

	if (basePath != "/" && !basePath.empty() && basePath[basePath.length() - 1] == '/')
        basePath.erase(basePath.length() - 1);

	if (!requestURL.empty() && requestURL[0] != '/' && basePath != "/")
        requestURL = "/" + requestURL;
	
	std::string resourcePath = basePath + requestURL;

	// Prevenir la creación de archivos fuera del directorio raíz
	if (!request.startsWith(resourcePath, basePath))
		return "";

	return resourcePath;
}

std::string adjustPathForDirectory(const std::string& requestURL, const std::string& basePath,
										const Location& location, VirtualServers& server)
{
	if (ConfigFile::fileExistsAndReadable(requestURL))
			return requestURL;

	std::string fullPath = basePath;
	if (requestURL != "/")
		fullPath += requestURL;
	
	if (ConfigFile::checkPath(fullPath) == IS_DIR)
	{
		if (location.getAutoindex())
			return fullPath;
		std::string indexFile;
		if (location.getIndexLocation().empty())
		{
			indexFile = server.getIndex();
			fullPath = server.getRoot() + indexFile;
		}
		else
		{
			indexFile = location.getIndexLocation();
			fullPath += indexFile;
		}
		if (ConfigFile::fileExistsAndReadable(fullPath))
			return fullPath;
	}
	return fullPath;
}

std::string buildResourcePath(HttpRequest& request,
	const Location& location, VirtualServers& server)
{
	// Extraer la URL de la solicitud
	std::string requestURL = request.getURL();

	// Eliminar cualquier parámetro de consulta de la URL
	size_t queryPos = requestURL.find('?');
	if (queryPos != std::string::npos)
		requestURL = requestURL.substr(0, queryPos);

	std::string basePath = location.getRootLocation().empty() ? server.getRoot() : location.getRootLocation();

	// Ajustar la ruta del recurso para manejo de directorios
	std::string resourcePath =
		adjustPathForDirectory(requestURL, basePath, location, server);
	if (!location.getAlias().empty())
		resourcePath.replace(0, location.getPath().length(), location.getAlias());
	return resourcePath;
}

std::string bodyReturn(const std::string cad, const std::string& url, int statusCode)
{
	HttpResponse r;
	std::stringstream ss;
	ss << statusCode;

	std::string body = "<html>\n<head>\n<title>";
	body += ss.str() + " " +r.getStatusMessage(statusCode) + "</title>\n</head>\n";
	body += "<body>\n<h1>" + ss.str() + " " + r.getStatusMessage(statusCode) + "</h1>\n";
	if (url != "")
		body += "<p><h2><font color=\"green\">Redirecting to <a href=\"" + url + "\">" + url + "</a></font></h2></p>\n";
	else
		body += "<p><h2><font color=\"red\">" + cad + "</font></h2></p>\n";
	body += "</body>\n</html>";
	return body;
}

bool checkOpenPorts(std::vector<Socket*> _serverSockets, VirtualServers server)
{
	if (_serverSockets.empty())
		return true;
	for (size_t i = 0; i < _serverSockets.size(); ++i)
	{
		if (htons(_serverSockets[i]->getSocketAddr().sin_port) == server.getPort())
			return false;
	}
	return true;
}
