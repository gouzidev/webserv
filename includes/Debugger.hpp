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
            typedef map <type1, type2> mapType;
            typename mapType::iterator it;
            cout << "--------------------------------------------------------------------------" << endl;;
            cout << varName << ": " << endl << "----";
            for (it = m.begin(); it != m.end(); it++)
                cout << it->first << " -> " << it->second << endl;
            cout << "--------------------------------------------------------------------------" << endl;;
            cout << endl;
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
            
            if (!locNode.uploadDir.empty())
                cout << spaces << "  Upload Path: " << locNode.uploadDir << endl;
            
            if (!locNode.cgiExts.empty())
            {
                cout << spaces << "  CGI Extensions:" << endl;
                for (map<string, string>::const_iterator it = locNode.cgiExts.begin(); it != locNode.cgiExts.end(); ++it)
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
            cout << "Host: " << servNode.hostStr << endl;
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
    
        // ✅ NEW: Print Host:Port to ServerNode mapping
        static void printHostServMap(const map<string, ServerNode> &hostServMap)
        {
            cout << "\n🗺️  HOST:PORT TO SERVER MAPPING" << endl;
            cout << "===========================================" << endl;
            cout << "Total mappings: " << hostServMap.size() << endl;
            
            for (map<string, ServerNode>::const_iterator it = hostServMap.begin(); it != hostServMap.end(); ++it)
            {
                cout << "\n🔑 Key: \"" << it->first << "\"" << endl;
                cout << "   └─ Maps to server with:" << endl;
                cout << "      • Host: " << it->second.hostStr << endl;
                cout << "      • Port: " << it->second.port << endl;
                cout << "      • Root: " << it->second.root << endl;
                cout << "      • Server Names: ";
                for (set<string>::const_iterator nameIt = it->second.serverNames.begin(); 
                     nameIt != it->second.serverNames.end(); ++nameIt)
                    cout << *nameIt << " ";
                cout << endl;
            }
            cout << "===========================================" << endl;
        }

        // ✅ NEW: Print ServerName:Port to ServerNode mapping  
        static void printServNameServMap(const map<string, ServerNode> &servNameServMap)
        {
            cout << "\n🏷️  SERVERNAME:PORT TO SERVER MAPPING" << endl;
            cout << "===========================================" << endl;
            cout << "Total mappings: " << servNameServMap.size() << endl;
            
            for (map<string, ServerNode>::const_iterator it = servNameServMap.begin(); it != servNameServMap.end(); ++it)
            {
                cout << "\n🔑 Key: \"" << it->first << "\"" << endl;
                cout << "   └─ Maps to server with:" << endl;
                cout << "      • Host: " << it->second.hostStr << endl;
                cout << "      • Port: " << it->second.port << endl;
                cout << "      • Root: " << it->second.root << endl;
            }
            cout << "===========================================" << endl;
        }

        // ✅ NEW: Print validation results and port uniqueness
        static void printValidationResults(const vector<ServerNode> &servNodes)
        {
            cout << "\n✅ VALIDATION RESULTS" << endl;
            cout << "===========================================" << endl;
            
            set<unsigned short> seenPorts;
            set<string> seenHostPorts;
            
            for (size_t i = 0; i < servNodes.size(); i++)
            {
                const ServerNode &node = servNodes[i];
                string hostPort = node.hostStr + ":" + toString(node.port);
                
                cout << "Server " << (i + 1) << ":" << endl;
                cout << "  • Host:Port = " << hostPort << endl;
                cout << "  • Port only = " << node.port << endl;
                
                // Check for port conflicts (your current validation)
                if (seenPorts.find(node.port) != seenPorts.end())
                    cout << "  ❌ DUPLICATE PORT DETECTED!" << endl;
                else
                    cout << "  ✅ Port is unique" << endl;
                
                // Check for host:port conflicts (correct validation)
                if (seenHostPorts.find(hostPort) != seenHostPorts.end())
                    cout << "  ❌ DUPLICATE HOST:PORT DETECTED!" << endl;
                else
                    cout << "  ✅ Host:Port combination is unique" << endl;
                
                seenPorts.insert(node.port);
                seenHostPorts.insert(hostPort);
                cout << endl;
            }
            cout << "===========================================" << endl;
        }

        // ✅ ENHANCED: Main method with all new features
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
            
            // Print validation results
            printValidationResults(servNodes);
            
            // Print a summary of all servers by host:port
            cout << "\n📊 SERVER SUMMARY BY HOST:PORT" << endl;
            cout << "==========================================" << endl;
            map<string, vector<size_t> > hostPortMap;
            
            for (size_t i = 0; i < servNodes.size(); i++)
            {
                string hostPort = servNodes[i].hostStr + ":" + toString(servNodes[i].port);
                hostPortMap[hostPort].push_back(i);
            }
            
            for (map<string, vector<size_t> >::const_iterator it = hostPortMap.begin(); it != hostPortMap.end(); ++it)
            {
                cout << "Host:Port " << it->first << " has " << it->second.size() << " server(s)" << endl;
                cout << "  Default server: Server Block [" << it->second[0] + 1 << "]" << endl;
                
                if (it->second.size() > 1)
                {
                    cout << "  ⚠️  MULTIPLE SERVERS ON SAME HOST:PORT!" << endl;
                    cout << "  Additional servers: ";
                    for (size_t i = 1; i < it->second.size(); i++)
                        cout << "Server Block [" << it->second[i] + 1 << "] ";
                    cout << endl;
                }
            }
            cout << "==========================================" << endl;
        }

        // ✅ NEW: Print everything including maps
        static void printCompleteDebugInfo(const vector<ServerNode> &servNodes, 
                                         const map<string, ServerNode> &hostServMap,
                                         const map<string, ServerNode> &servNameServMap)
        {
            printServerConfig(servNodes);
            printHostServMap(hostServMap);
            printServNameServMap(servNameServMap);
        }

        // Helper function for toString since it might not be available in all C++ versions
        template <typename T>
        static string toString(T value)
        {
            ostringstream os;
            os << value;
            return os.str();
        }
};

#endif