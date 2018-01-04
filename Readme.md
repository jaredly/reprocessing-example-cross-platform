# Setup

First go to https://github.com/jaredly/ocaml-cross-mobile/ to set up your cross-compilers. (currently macOS is the only platform with prebuilt compilers)

Check out `package.json` to see what commands are available.

# Development

I recommend using native built + hot reloading for most of your development, and only build to web or mobile to check things out.

`npm start` will get your bsb build going, and start the app. It will try to keep the app alive, by checking every second if it's died, and restarting it if it has.


# iOS

`npm run ios`, and then open the `./ios/App.xcodeproj` in Xcode to finish the build.

# Android

`npm run android` to build, or `npm run android:run` to build & install to the currently running emulator / attached device.

