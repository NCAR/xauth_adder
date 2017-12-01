#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////////////////
// split a string into separate substring by separating at characters given by
//  delimiters.
//////////////////////////////////////////////////////////////////////////////////////////
inline size_t splitString( const std::string& str_in, const std::string& delimiters, 
    std::vector<std::string>& tokens)
{
    size_t ntokens(0);
    tokens.clear();
    if (str_in.size()==0) return 0;
    size_t first=str_in.find_first_not_of(delimiters,0);
    size_t last=str_in.find_first_of(delimiters,first);
    while (first!=std::string::npos) {
        ++ntokens;
        tokens.push_back( str_in.substr( first, last-first) );
        first=str_in.find_first_not_of(delimiters,last);
        last=str_in.find_first_of(delimiters,first);
    }
    return ntokens;
}


int main()
{
    FILE *fptr;

    const char *xx = getenv("DISPLAY");

    if (xx==0) {
        std::cerr << "none display variable defined!\n";
        exit(EXIT_FAILURE);         
    }
 
    std::string display_name(xx);
    
    if (display_name.size() == 0) {
        std::cerr << "none display variable defined!\n";
        exit(EXIT_FAILURE); 
    }

#ifdef _NDEBUG_        
    std::cerr << "display name = " << display_name << std::endl;
#endif
    
    size_t colon_place = display_name.find(":");
    
    if (colon_place==0 || colon_place==std::string::npos) {
        std::cerr << "invalid display name " << display_name << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // copy over host name
    
    std::string host = display_name.substr(0,colon_place);
        
    std::string display_no = display_name.substr(colon_place+1,std::string::npos);    
    std::string display_noX = display_no.substr(0,display_no.find("."));

#ifdef _NDEBUG_        
    std::cerr << "host = " << host << "\n";
    std::cerr << "display number = " << display_no << " " << display_noX << "\n";
#endif
    
    // get xauth list //
    
    std::string cmd_xlist = "xauth list " + display_name;
    std::cerr << cmd_xlist << "\n";

    fptr=popen(cmd_xlist.c_str(),"r");
    if (ferror(fptr) || fptr==NULL) {
        std::cerr<< "could not launch xauth\n";
        exit(EXIT_FAILURE);
    }
    char ch;
    std::string buffer_str;
    char *ln;
    size_t ln_len=0;
    
    while (getline(&ln,&ln_len,fptr)!=-1) {
        buffer_str += ln;
        buffer_str += " ";
        free(ln);
        ln_len=0;
    }
#ifdef _NDEBUG_            
    std::cerr << buffer_str <<  std::endl;
#endif
    int e;
    if (e=pclose(fptr)) {
        std::cerr<< "error " << e << " closing stream \n";
        exit(EXIT_FAILURE);
    }
    
    std::vector<std::string> list;
    
    size_t nt = splitString(buffer_str," ",list);
    
    std::vector<std::string>::iterator iter = list.begin();
    std::string key;
    while (iter!= list.end()) {
        size_t px = (*iter).find(host); 
        if (px!=std::string::npos) {
            ++iter;
            ++iter;
            key = *iter;
            break;
        }
        ++iter;
    }
#ifdef _NDEBUG_        
    std::cerr << "key = " << key << std::endl;
#endif
    std::string comm = "xauth add ";
    comm += host;
    comm += ".ucar.edu:";
    comm += display_noX;
    comm += " . ";
    comm += key;    
#ifdef _NDEBUG_        
    std::cerr << comm << std::endl;
    std::cerr << "adding in key \n";
#endif    
    e = system(comm.c_str());
    
    if (e) {
        std::cerr << "xauth add error " << e << " " << strerror(e) << "\n"; 
    }
    
    std::string ndisp = host + ".ucar.edu:" + display_noX;
    setenv("NDISPLAY",ndisp.c_str(),0);
    return EXIT_SUCCESS;
}