# Link Launcher
*Filter links with regular expressions and launch them into your favourite browsers*

Have you ever wanted to open a youtube link from other software (Discord perhaps) into VLC player? Well now you can. When you build this executable and register it as a web browser in Windows registry, this program intercepts any links, pattern matches them with regular expression and launches them in an application of your choosing.

A new feature I added recently is that now you can redirect link onto a custom portal service, for example youtube can be directed to invidious instance. Just capture the video id in the regular expression with parenthesis and add a replacement link with placeholders (`https://www.invidious.../watch?v=<1>`) into replacements array of link-arrays. The reason why there's array of link arrays is that each matching pattern has its own array of replacements, so they are directly mapped to each other, this might change in the future because it's bit nasty solution. 

Big thanks for [kgabis](https://github.com/kgabis) for the [MIT](https://github.com/kgabis/parson/blob/master/LICENSE)-licensed [parson](https://github.com/kgabis/parson) (json parsing) library.

**Use anything in this repo, especially those .bat files under Windows folder, on your own risk. I\'m not responsible of any explicit or implicit harm or damage this repo's contents may cause. I strongly recommend to take backup of your registry first before applying the registry changes.** 

## Setup instructions
### 1. How to build
On windows, open Visual Studio 2019 development console, navigate into LinkLauncher folder, and run `build.bat`. 

PS. I haven\'t tested building this project any Linux distro yet, although there\'s untested code for Linux too.

### 2. How to un/install (do it on your own risk)
On Windows, open a console with admin privileges, navigate into LinkLauncher\Windows\ folder, to install run register_linklauncher.bat to uninstall run unregister_linklauncher.bat

### 3. associations.json
The associations.json must be found from the folder where LINK_LAUNCHER points in the user environment variables, in other words it must lie in the same folder as LinkLauncher.exe. The associations.json is structured like this:
```json
{
	"browsers": {
		"firefox": {
			"app": "C:/Program Files/Mozilla Firefox/firefox.exe",
			"default_args": ["-new-tab"]
		},
		"vlc": {
			"app": "C:/Program Files/VideoLAN/VLC/vlc.exe",
			"default_args": []
		}
	},
	"associations": {
		"default": {
			"patterns":  [],
			"browsers": [
				{
					"browser": "firefox",
					"args": []
				}
			],
		},
		"youtube": {
			"patterns": [
				"youtube\\.com\\/watch\\?.*&?v=(.*)",
				"youtu\\.be\\/(.*)"
			],
			"browsers": [
				{
					"browser": "vlc",
					"args": []
				}
			],
			"replacements": [
				["https://invidious.silkky.cloud/watch?v=<1>"],
				["https://invidious.silkky.cloud/watch?v=<1>"]
			]
		}
	}
}
```
*Notice how backslashes* `\` *are escaped with double backslash* `\\`*.*