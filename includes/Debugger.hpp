#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "webserv.hpp"

class Debugger
{
    public:
        template <typename Container>
        static void printVec(string varName, vector <Container> &v)
        {
            cout << varName << ": " << endl << "----";
            for (size_t i = 0; i < v.size(); i++)
                cout << v[i] << "  ";
            cout << endl;
        }

        template <typename type1, typename type2>
        static void printMap(string varName, map <type1, type2> &m)
        {
            cout << varName << ": " << endl << "----";
            for (size_t i = 0; i < m.size(); i++)
                cout << m[i].first << " -> " << m[i].second << "  ";
            cout << endl;
        }

        static void printLocationNode(const LocationNode &locNode, int indent = 2)
        {
            string spaces(indent, ' ');
            
            cout << spaces << "🔹 Location Path: " << locNode.path << endl;
            cout << spaces << "  Root: " << locNode.root << endl;
            
            cout << spaces << "  Allowed Methods: ";
            for (set<string>::const_iterator it = locNode.methods.begin(); it != locNode.methods.end(); ++it)
                cout << *it << " ";
            cout << endl;
            
            cout << spaces << "  AutoIndex: " << (locNode.autoIndex ? "ON" : "OFF") << endl;
            
            cout << spaces << "  Index Files: ";
            for (size_t i = 0; i < locNode.index.size(); i++)
                cout << locNode.index[i] << " ";
            cout << endl;
            
            if (locNode.redirect.first != 0)
                cout << spaces << "  Redirect: " << locNode.redirect.first << " -> " << locNode.redirect.second << endl;
            
            if (!locNode.upload_path.empty())
                cout << spaces << "  Upload Path: " << locNode.upload_path << endl;
            
            if (!locNode.cgi_exts.empty())
            {
                cout << spaces << "  CGI Extensions:" << endl;
                for (map<string, string>::const_iterator it = locNode.cgi_exts.begin(); it != locNode.cgi_exts.end(); ++it)
                    cout << spaces << "    " << it->first << " -> " << it->second << endl;
            }
            
            if (!locNode.headers.empty())
            {
                cout << spaces << "  Headers:" << endl;
                for (size_t i = 0; i < locNode.headers.size(); i++)
                    cout << spaces << "    " << locNode.headers[i] << endl;
            }
        }
        
        static void printServerNode(const ServerNode &servNode)
        {
            cout << "\n🌐 SERVER NODE" << endl;
            cout << "===========================================" << endl;
            cout << "Host: " << servNode.host << endl;
            cout << "Port: " << servNode.port << endl;
            cout << "Root: " << servNode.root << endl;
            
            cout << "Server Names: ";
            for (set<string>::const_iterator it = servNode.serverNames.begin(); it != servNode.serverNames.end(); ++it)
                cout << *it << " ";
            cout << endl;
            
            cout << "Client Max Body Size: " << servNode.clientMaxBodySize << " megabytes" << endl;
            
            // Print error pages
            if (!servNode.errorNodes.empty())
            {
                typedef map<unsigned short, string> errorCodePageMapType;
                 map<unsigned short, string> errorCodePageMap = servNode.errorNodes;
                cout << "\n📄 Error Pages:" << endl;
                for (errorCodePageMapType::iterator it = errorCodePageMap.begin(); it != errorCodePageMap.end(); it++)
                {
                    cout << "  Error Code: " << it->first << endl;
                    cout << "  Error Page: " << it->second << endl;
                }
                cout << endl;
            }
            
            // Print locations
            if (!servNode.locationNodes.empty())
            {
                cout << "\n📂 Locations (" << servNode.locationNodes.size() << "):" << endl;
                for (size_t i = 0; i < servNode.locationNodes.size(); i++)
                {
                    cout << "\n  [" << i + 1 << "] Location" << endl;
                    printLocationNode(servNode.locationNodes[i]);
                }
            }
            cout << "===========================================" << endl;
        }
    
        // Main method to print all server configurations
        static void printServerConfig(const vector<ServerNode> &servNodes)
        {
            cout << "\n\n🖥️  WEBSERV CONFIGURATION" << endl;
            cout << "==========================================" << endl;
            cout << "Total Server Blocks: " << servNodes.size() << endl;
            
            for (size_t i = 0; i < servNodes.size(); i++)
            {
                cout << "\n📌 Server Block [" << i + 1 << "]" << endl;
                printServerNode(servNodes[i]);
            }
            
            // Print a summary of all servers by host:port
            cout << "\n📊 SERVER SUMMARY BY HOST:PORT" << endl;
            cout << "==========================================" << endl;
            map<string, vector<size_t> > hostPortMap;
            
            for (size_t i = 0; i < servNodes.size(); i++)
            {
                string hostPort = servNodes[i].host + ":" + to_string(servNodes[i].port);
                hostPortMap[hostPort].push_back(i);
            }
            
            for (map<string, vector<size_t> >::const_iterator it = hostPortMap.begin(); it != hostPortMap.end(); ++it)
            {
                cout << "Host:Port " << it->first << " has " << it->second.size() << " server(s)" << endl;
                cout << "  Default server: Server Block [" << it->second[0] + 1 << "]" << endl;
                
                if (it->second.size() > 1)
                {
                    cout << "  Additional servers: ";
                    for (size_t i = 1; i < it->second.size(); i++)
                        cout << "Server Block [" << it->second[i] + 1 << "] ";
                    cout << endl;
                }
            }
            cout << "==========================================" << endl;
        }

        // Helper function for to_string since it might not be available in all C++ versions
        template <typename T>
        static string to_string(T value)
        {
            ostringstream os;
            os << value;
            return os.str();
        }

};


#endif