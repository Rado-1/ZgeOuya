# v0.91 (2014-04-16) #

  * `applicationKey` parameter of the `ouya_InitPurchasing()` function was removed. The `key.der` file is read directly from the `assets` folder.
  * Requesting of the product info was removed; namely the functions `ouya_RequestProducts`, `ouya_IsProductRequestDone`, and `ouya_GetProductInfo`.

> Uses ODK Version 1.0.11 - Feb 04, 2014.

# v0.9 (2014-04-13) #

Initial version with IAP ans storage functionality:
  * initialization with developer uuid and game key,
  * obtaining product information
  * purchasing products
  * obtaining purchase receipts
  * obtaining gamer information
  * putting and getting game data

> Uses ODK Version 1.0.11 - Feb 04, 2014.