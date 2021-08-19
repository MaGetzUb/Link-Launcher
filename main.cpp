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
#include <format>
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
	std::vector<std::pair<std::string, std::vector<std::string>>> browsers;
	std::vector<std::vector<std::string>> replacements;
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
bool ReplaceSlots(std::string& text, const std::smatch& replacements) {
	//static_assert(std::is_same_v<decltype(replacements[0]), std::string>, "Array element doesn't contain std::string");
	std::size_t next = 0;
	for(std::size_t index = text.find('<', next); index != std::string::npos; index = text.find('<', next)) {
		next = text.find('>', index);
		if(next != std::string::npos) {
			int replaceIndex = std::stoi(text.substr(index+1, next-index-1));
			if(replaceIndex >= std::size(replacements))
				return false;
			text.replace(index, next-index+1, replacements[replaceIndex]);
		} else {
			return false;
		}
	}
	return true;
}


#if defined(NDEBUG) && defined(_WIN32)
//If subsystem is WINDOWS, the console is gotten rid of and the good ol' main stops existing, so we need to fake it.
int main(int argc, char* argv[]);

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* pCmdLine, int nShowCmd) {
	return main(__argc, __argv);
}
#endif 

std::string ReadFile(const std::string& filepath);

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

			JSON_Array* browsers = json_object_get_array(object, "browsers");
			int browserCount = json_array_get_count(browsers);

			JSON_Array* replacements = json_object_get_array(object, "replacements");
			int replacementCount = json_array_get_count(replacements);


			if(!browserCount) {
				logger << '<' << DateTime() << '>' << " at least one browser required!\n";
				return -1;
			}

			if(browserCount) {
				Association assoc;
				for(int j = 0; j < patternCount; j++) {
					assoc.patterns.emplace_back(json_array_get_string(patterns, j));
				}

				for(int j = 0; j < browserCount; j++) {
					JSON_Object* object = json_array_get_object(browsers, j);
					
					JSON_Value* browserValue = json_object_get_value(object, "browser");
					std::string browserName = json_value_get_string(browserValue);

					JSON_Value* argsValue = json_object_get_value(object, "args");
					JSON_Array* argsArray = json_value_get_array(argsValue);


					std::vector<std::string> browserArgs = {};
					int argCount = json_array_get_count(argsArray);

					for(int k = 0; k < argCount; k++) {
						std::string arg = json_array_get_string(argsArray, k);
						browserArgs.push_back(arg);
					}

					assoc.browsers.emplace_back(std::make_pair(browserName, browserArgs));
				}

				for(int k = 0; k < replacementCount; k++) {
					JSON_Array* stringsArray = json_array_get_array(replacements, k);
					std::vector<std::string> strings;
					for(int l = 0; l < json_array_get_count(stringsArray); l++) {
						strings.emplace_back(json_array_get_string(stringsArray, l));
					}
					assoc.replacements.emplace_back(std::move(strings));
				}

				associationMap.insert(std::make_pair(name, std::move(assoc)));
			}
			


		}
	}

	const Association* assoc = &associationMap["default"];

	std::string link = argv[1];

	for(const auto& i: associationMap) {
		std::smatch match;
		auto patterns = i.second.patterns;
		for(int j = 0; j < patterns.size(); j++) {
			const auto& pattern = patterns[j];
			auto replacements = i.second.replacements[j];

			if(std::regex_search(link, match, pattern)) {
				if(!replacements.empty()) {
					std::uniform_int_distribution<int> distrib(0, assoc->replacements.size()-1);
					int index = distrib(randomDevice);
					link = replacements[index];
					ReplaceSlots(link, match);
				}
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
			assoc->browsers[browserIndex].first
		];

		std::string args = "";
		const std::vector<std::string>* argList = &browser.defaultArgs; 

		if(assoc->browsers[browserIndex].second.size()) {
			argList = &assoc->browsers[browserIndex].second;
		}


		if(argList->size()) {
			for(int i = 0; i < argList->size(); i++) {
				args.append('\"'+argList->at(i)+'\"');
				args.push_back(' ');
			}
		}

		args.append(link);

		arguments = args;

		if(browser.path.empty()) {
			logger << '<' << DateTime() << '>' << " ERROR: Cannot open URL: [" << link << "] no browser exist for the url.\n";
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
			logger << '<' << DateTime() << '>' << "INFO: [" << link << "] Opened with: " << browser.path << '\n';
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



extern "C" {
#include "parson.c"
}