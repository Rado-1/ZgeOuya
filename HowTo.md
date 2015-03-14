**Important: this page is applicable for the latest version of the library, compatibility to the previous versions is not guaranteed.**

# Usage of OUYA IAP in ZGameEditor project #

**Important: To use the OUYA IAP functionality in your application, you should be an registered OUYA developer and should create game(s) and product(s) to purchase. More information about how to become OUYA developer can be found [here](http://www.ouya.tv/develop/).**

Since all IAP functionality includes Internet communication with OUYA backend server, it is realized by asynchronous request-response communication pattern. The common call pattern is used:
  1. Include `ZExternalLibrary` component for ZgeOuya library to the `OnLoaded` section of `ZApplication`. It can be found in the context menu _Add from library... > External libraries > OUYA library_ of the Project tree.
  1. Initialize IAP with `ouya_InitPurchasing()`. This call is usually placed to the `OnLoaded` section of `ZApplication` or `OnStart` section of `AppState` component. You must provide your developer UUID downloaded from the [OUYA Developer Portal](http://devs.ouya.tv/developers).
  1. In order to request the OUYA server, first, the `ouya_Request*()` function is called to initialize communication. This call can be placed at any position in your code where appropriate.
  1. Then, the `ouya_Is*RequestDone()` is used to check the availability of the response. If this function returns`ouya_YES`, the response is available and you can read the result. If the function returns `ouya_NO` the request is still in progress. If the `ouya_ERROR` was returned, the request failed and the output `errorMsg` parameter contains the error message. The `ouya_Is*RequestDone()` function is usually called in `OnUpdate` section of `ZApplication`, `AppState`, or `Model` components.
  1. When the response is available, the results can be read by the `ouya_Get*()` function.
  1. To close the IAP processing, call the `ouya_StopPurchasing()` function. This call is usually placed to the `OnClose` section of `ZApplication`, `OnLeave` section of `AppState`, or `OnRemove` section of the `Model` component respectively.

_Note: the previous description uses `*` as a placeholder for the type of requested object; it can be "Purchase" for purchasing a product, "Receipts" for a list of already purchased products, and "GamerInfo" for information about the current gamer._

**Important: Android package name of your game, as specified on the OUYA Development Portal, must be identical to the `ZApplication.AndroidPackageName` property. If not, your game cannot perform purchases or obtain purchase receipts; the "Invalid game Uuid was provided" error is returned.**

Examples of usage ZgeOuya IAP in ZGE can be found at the [download page](http://googledrive.com/host/0BxwfQ8la88ouc2t2TWJ1NUtCeUk/).

# Usage of OUYA Controller in ZGameEditor project #

  1. Put the `Android Gamepad Library` ZLibrary to the `OnLoaded` section of `ZApplication`. It can be found in the context menu _Add from library... > Libraries > Android Gamepad Library_ of the Project tree.
  1. Ask for status of the controller buttons with `joyGetButton()` function and status of the joystick axis with the `joyGetAxis()` functions. These functions are usually called from`OnUpdate` section of `ZApplication`, `AppState`, or `Model` components.
  1. To identify OUYA buttons and axes, use the `OUYA_*` codes defined in the library.

<a href='http://www.youtube.com/watch?feature=player_embedded&v=99y2JBYSbD0' target='_blank'><img src='http://img.youtube.com/vi/99y2JBYSbD0/0.jpg' width='425' height=344 /></a>

# Building application for OUYA IAP #

  1. Set the ZApplication.AndroidSdk to "4.1 (API Level 16)".
  1. Compile your Android application, e.g., use _Project / Android: Build APK (debug)_ menu item.
  1. Download the OUYA Development Kit form [this link](https://devs.ouya.tv/developers/odk), extract it, and the included `ouya-sdk.jar` file place to the `libs` folder of your application.
  1. Place `libZgeOuya.so` file to the `libs/armeabi` folder.
  1. Place the `ZgeOuya.java` file to the `src/org/zgameeditor` folder.
  1. Place the game signing key file `key.der` (obtained from the [OUYA Developer Portal](http://devs.ouya.tv/developers)) to the `assets` folder.
  1. Compile the APK again as in step 1.
  1. Deploy the APK file to your OUYA device and install it. You can use, for instance, USB transfer from PC to OUYA and FilePwn OUYA application for installing the application at the device.

_Note 1: Seps from 1 to 6 are required only when the application is built the first time. After that, a single compilation produces the correct APK._

_Note 2: Use steps 4 and 5 also in the case when updating to a newer version of the ZgeOuya library._

# Building application for OUYA Controller, without usage of IAP #

To build your application just to support OUYA controller, without making use of the IAP mechanism, perform the steps 1, 2, and 8 from the process described in the previous section.