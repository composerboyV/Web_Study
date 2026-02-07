#ifndef LocationManager_HPP
# define LocationManager_HPP

# include <string>
# include <vector>
# include <map>

# include "../enum.h"

class LocationManager
{
    private :
    
        bool                                AutoIndex;          // 실제 설정된 value
        bool                                HasAutoIndex;       // 실제 로케이션 블록에 설정된 건지 확인하는 플래그 equalModifierSet
        bool                                uploadEnable;       // 실제 설정된 value
        bool                                equalModifier;      // 실제 설정된 value
        bool                                hasUploadEnable;    // 실제 로케이션 블록에 설정된 건지 확인하는 플래그 equalModifierSet
        bool                                hasEqualModifier;   // 실제 로케이션 블록에 설정된 건지 확인하는 플래그 equalModifierSet
    
        long long                           clientMaxBodySize;
        
        std::string                         root;
        std::string                         route;
        std::string                         uploadStore;
        
        std::vector <std::string>           index;
        std::vector <std::string>           methods;
        
        std::map <int, std::string>         redirect;
        std::map <int, std::string>         errorPage; 
        std::map <std::string, std::string> cgiInfo;
        
    public :
        LocationManager();
        LocationManager(std::string _route);
        LocationManager(const LocationManager &other);
        ~LocationManager();

        LocationManager     &operator = (const LocationManager &other);

        //getter

        bool                        getUploadEnable();
        bool                        getEqualModifier();
        bool                        getHasUploadEnable();
        bool                        getHasEqualModifier();

        std::string                 &getRoot();
        std::string                 getRoute();
        std::string                 getUploadStore();
        std::string                 getMethodsIdx(int idx);
        std::string                 getErrorPageWithKey(int key);
        std::string                 getIndexPageWithIdx(int idx);
        std::string                 getRedirectPageWithKey(int val);
        std::string                 getCgiInfoWithKey(std::string val);
        
        std::vector <std::string>   getIndex();
		std::vector <std::string>	getMethods();

		int							getRedirectStatusCode();
        
        bool                        getAutoIndex();
        bool                        getHasAutoIndex();
        
        long long                   getClientMaxBodySize();
        
        std::map <int, std::string> getErrorPage();

        //setter

        void                        setAutoIndex(bool _val);
        void                        setUploadEnable(bool _val);
        void                        setRoot(std::string _root);
        void                        setHasAutoIndex(bool _val);        
        void                        setEqualModifier(bool _val);
        void                        setRoute(std::string _route);
        void                        setHasUploadEnable(bool _val);
        void                        setHasEqualModifier(bool _val);
        void                        setUploadStore(std::string _val);
        void                        setClientMaxBodySize(long long _val);
        void                        addCgiInfo(std::vector <std::string> token);
        void                        setRedirect(std::vector <std::string> token);
        
        void                        setMethods(std::vector <std::string> token);
        void                        setIndexHtml(std::vector <std::string> token);
        void                        addNewErrorHtml(int errCode, std::string page);
        void                        setErrorPage(std::map <int, std::string> _errorPage);
        void                        fillMissingErrorPages(std::map <int, std::string> parents);
       
        // check

        bool                        isMethodSet();
        bool                        isRedirectSet();
        bool                        isAutoIndexSet();
        bool                        isUploadEnableSet();
		bool						isExistingCGI(std::string &extension);

        // print

        void                        printErrorPage();
        void                        printParsedLocation();

        // exception

        class       ConflictingDirectiveException : public std::exception {
            public :
                virtual const char          *what() const throw();
        };

        class       OutofRangeException : public std::exception {
            public :
                virtual const char      *what() const throw();
        };
};

#endif