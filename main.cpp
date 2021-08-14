#ifdef _WIN32
#	define _WIN32_LEAN_AND_MEAN
#	include <windows.h>
#elif defined(__linux__)
#	include <unistd.h>
#	if __has_include( <gtk/gtk.h> )
#		include <gtk/gtk.h>
#		if defined(GDK_WINDOWING_X11)
#			include <gdk/gdkx.h>
#		endif
#	endif 
#endif 

#include <iostream>
#include <shellapi.h>
#include <regex>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <random>
#include <stdexcept>
#include "parson.h"

#if defined(_WIN32) && (defined(__MINGW32__) || defined(__MINGW__))
extern int __argc;
extern char ** __argv;
#endif 

void ErrorDLG(const std::string& message) {
#ifdef _WIN32
	MessageBoxA(NULL, message.c_str(), "Link Launcher", MB_OK);
#else 
	GtkWidget* dlg = gtk_message_dialog(NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, message.c_str());
	gtk_window_set_title(GTK_WINDOW(dlg), "Link Launcher");
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(GTK_WIDGET(dlg));
#endif 
}

struct Association {
	std::vector<std::regex> patterns;
	std::vector<std::string> arguments;
	std::vector<std::string> browsers;
};

struct Browser {
	std::string path;
	std::vector<std::string> defaultArgs; 
};
	

std::string DateTime() {
	time_t now = time(0);
	std::string dateTime = ctime(&now);
	dateTime.pop_back();
	return dateTime;
}

std::ofstream gLogger;

// Haxx over 9000 starts :D
struct Logger {};

template<typename T>
Logger operator<<(Logger logger, const T& in) {
	std::cout << in;
	gLogger << in;
	return logger;
}
//End of haxx

#if defined(NDEBUG) && defined(_WIN32)
//If subsystem is WINDOWS, the console is gotten rid of and the good ol' main stops existing, so we need to fake it.
int main(int argc, char* argv[]);

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* pCmdLine, int nShowCmd) {
	return main(__argc, __argv);
}
#endif 

std::string ReadFile(const std::string& filepath) {
	if(std::ifstream file{filepath, std::ios::in | std::ios::ate}) {
		std::string str{};
		str.resize(file.tellg(), '\0');
		file.seekg(0, std::ios::beg);
		file.read(str.data(), str.size());
		return str;
	} 
	//Let's just use these nasty exceptions this once :D
	throw std::runtime_error{"Failed to open file! " + filepath};
	return {};
}


int main(int argc, char* argv[]) {

	Logger logger;


	std::string installLocation = "";

	std::random_device randomDevice;
	
	//Figure out "Install location" from the system environment"
	if(const char* path = nullptr; (path = std::getenv("LINK_LAUNCHER")) != nullptr) {
		installLocation = path;
		char lastCh = installLocation.back();
		//Add last slash to the path, if it's missing
		if(!(lastCh == '/' || lastCh == '\\')) 
			installLocation.push_back('/');
	}


	gLogger.open(installLocation+"linklauncher.log", std::ios::app);

	if(argc < 2) {
		logger << '<' << DateTime() << '>' << " ERROR: Not enough arguments!\n";
		return 0;
	}

	std::unordered_map<std::string, Browser> browserMap;
	std::unordered_map<std::string, Association> associationMap; 

	std::string associationConfig = installLocation + "associations.json";
	std::cout << associationConfig << '\n';

	std::string configfile;
	try {
		configfile = ReadFile(associationConfig);		
	} catch(const std::runtime_error& err) {
		ErrorDLG(err.what());
		return -1;
	}

	JSON_Value* serialized = json_parse_string(configfile.c_str());

	if(!serialized) {
		ErrorDLG("Failed to parse associations.json!");
		logger << '<' << DateTime() << '>' << " ERROR: Failed to parse associations.json!\n";
		return -1; 
	}
	
	JSON_Object* serialized_object = json_object(serialized); 

	{
		JSON_Object* browsers = json_object_get_object(serialized_object, "browsers");
		int numBrowsers = json_object_get_count(browsers);
	
		for(int i = 0; i < numBrowsers; i++) {
			JSON_Value* value = json_object_get_value_at(browsers, i);
			JSON_Object* object = json_object(value);

			const char* name = json_object_get_name(browsers, i);

			const char* app = json_object_get_string(object, "app");

			JSON_Array* defaultArgs = json_object_get_array(object, "default_args");

			if(app && defaultArgs) {
				
				Browser browser;
				browser.path = app;
				
				int numDefaultArgs = json_array_get_count(defaultArgs);

				for(int i = 0; i < numDefaultArgs; i++) {
					browser.defaultArgs.emplace_back(json_array_get_string(defaultArgs, i));
				}

				browserMap.insert(std::make_pair(name, std::move(browser)));
			} 

		}
	}

	{
		JSON_Object* assocs = json_object_get_object(serialized_object, "associations");
		int numAssociations = json_object_get_count(assocs);

		for(int i = 0; i < numAssociations; i++) {
			JSON_Value* value = json_object_get_value_at(assocs, i);	
			JSON_Object* object = json_object(value);
			
			const char* name = json_object_get_name(assocs, i);

			JSON_Array* patterns = json_object_get_array(object, "patterns");
			int patternCount = json_array_get_count(patterns);

			JSON_Array* browsers = json_object_get_array(object, "browser");
			int browserCount = json_array_get_count(browsers);

			if(!browserCount) {
				logger << '<' << DateTime() << '>' << " at least one browser required!\n";
				return -1;
			}
			
			JSON_Array* arguments = json_object_get_array(object, "args");
			int argumentCount = json_array_get_count(arguments);

			if(patterns && browsers && arguments && browserCount) {
				Association assoc;
				for(int j = 0; j < patternCount; j++) {
					assoc.patterns.emplace_back(json_array_get_string(patterns, j));
				}
				for(int j = 0; j < browserCount; j++) {
					assoc.browsers.emplace_back(json_array_get_string(browsers, j));
				}
				for(int j = 0; j < argumentCount; j++) {
					assoc.arguments.emplace_back(json_array_get_string(arguments, j));
				}
				associationMap.insert(std::make_pair(name, std::move(assoc)));
			}
			

		}
	}

	const Association* assoc = &associationMap["default"];


	for(const auto& i: associationMap) {
		std::cmatch match;
		for(const auto& pattern: i.second.patterns) {
			if(std::regex_search(argv[1], match, pattern)) {
				assoc = &i.second;
				break;
			}
		}
	}

	
	//Combine the default-arguments and the link into one string, 
	//that's passed to the actual browser   
	std::string arguments = "";
	{
		int browserIndex =  0;
		
		if(assoc->browsers.size()) {
			std::uniform_int_distribution<int> distrib(0, assoc->browsers.size()-1);
			browserIndex = distrib(randomDevice);
		}
		
		const Browser& browser = browserMap[
			assoc->browsers[browserIndex]
		];

		std::string args = "";
		const std::vector<std::string>* argList = &browser.defaultArgs; 

		if(assoc->arguments.size()) {
			argList = &assoc->arguments;
		}


		if(argList->size()) {
			for(int i = 0; i < argList->size(); i++) {
				args.append('\"'+argList->at(i)+'\"');
				args.push_back(' ');
			}
		}

		args.append(argv[1]);

		arguments = args;

		if(browser.path.empty()) {
			logger << '<' << DateTime() << '>' << " ERROR: Cannot open URL: [" << argv[1] << "] no browser exist for the url.\n";
			return -1;
		}

#if defined(_WIN32)

		SHELLEXECUTEINFOA info = {};
		info.cbSize = sizeof(info);
		info.fMask = 0x0;
		info.hwnd = nullptr;
		info.lpVerb = "open";
		info.lpFile = browser.path.data();
		info.lpParameters = arguments.data();
		info.nShow = SW_NORMAL;
		info.hInstApp = nullptr;
		BOOL res = ShellExecuteExA(&info);
			
		if(res == TRUE) {
			logger << '<' << DateTime() << '>' << "INFO: [" << argv[1] << "] Opened with: " << browser.path << '\n';
		}

#elif defined(__linux__)
	pid_t pid = fork();
	char* args[] = {arguments.data(), "\0"};
	if(pid == 0 && execv(browsers.path.data(), args)) {
	}
#elif defined(__MAC_OS_X__)

#endif 
	}

	return 0;
}

extern "C" {
#include "parson.c"
}