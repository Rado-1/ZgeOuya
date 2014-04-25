/*
ZgeOuya Library
Copyright (c) 2014 Radovan Cervenka

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
*/

package org.zgameeditor;

import java.io.InputStream;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.X509EncodedKeySpec;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import org.json.JSONObject;

import tv.ouya.console.api.CancelIgnoringOuyaResponseListener;
import tv.ouya.console.api.GamerInfo;
import tv.ouya.console.api.OuyaEncryptionHelper;
import tv.ouya.console.api.OuyaFacade;
import tv.ouya.console.api.OuyaResponseListener;
import tv.ouya.console.api.Purchasable;
import tv.ouya.console.api.Receipt;
import android.annotation.SuppressLint;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;

public class ZgeOuya {
  
  private static final String LOG_TAG = "ZgeOuya";
  private static final String CANCEL_PURCHASE_MESSAGE = "User cancelled purchase";
  private static final int YES = 1;
  private static final int NO = 0;
  private static final int ERROR = -1;
  
  private static final OuyaFacade facade = OuyaFacade.getInstance();
  private static PublicKey mPublicKey;

  static {
    System.loadLibrary("ZgeOuya");
  }

  private static native void NativeSetGamerInfo(boolean success,
      String username, String uuid, String errorMessage);
  
  private static native void NativeSetPurchaseInfo(boolean success,
      String productId, String errorMessage);

  private static native void NativeSetReceiptInfo(boolean success,
      boolean finished, String productId, String errorMessage);

  // Generic
  
  public static boolean isOuya() {
    return facade.isRunningOnOUYAHardware();
  }

  // In-App Purchasing

  public static synchronized int initPurchasing(String developerId) {
    facade.init(ZgeActivity.zgeActivity, developerId);
 
    // create a public key object from the key data file
    try {
      // read the key.der file (downloaded from the developer portal)
      AssetManager assets = ZgeActivity.zgeActivity.getAssets();
      InputStream inputStream = assets.open("key.der", AssetManager.ACCESS_BUFFER);
      byte[] applicationKey = new byte[inputStream.available()];
      inputStream.read(applicationKey);
      inputStream.close();

      // create public key
      X509EncodedKeySpec keySpec = new X509EncodedKeySpec(applicationKey);
      KeyFactory keyFactory = KeyFactory.getInstance("RSA");
      mPublicKey = keyFactory.generatePublic(keySpec);
    } catch (Exception e) {
      Log.e(LOG_TAG, "Unable to create encryption key", e);
      return ERROR;
    }
    return YES;
  }

  public static void stopPurchasing() {
    facade.shutdown();
  }
  
  // Making purchases

  private static final Map<String, String> mOutstandingPurchaseRequests = new HashMap<String, String>();

  // purchase request callback
  private static class PurchaseListener implements OuyaResponseListener<String> {

    private String mProductId;

    PurchaseListener(final String productId) {
      mProductId = productId;
    }

    @Override
    public void onSuccess(String result) {

      try {
        String id;
        OuyaEncryptionHelper helper = new OuyaEncryptionHelper();

        JSONObject response = new JSONObject(result);
        id = helper.decryptPurchaseResponse(response, mPublicKey);
        String storedProductId;
        synchronized (mOutstandingPurchaseRequests) {
          storedProductId = mOutstandingPurchaseRequests.remove(id);
        }

        if(storedProductId == null) {
          NativeSetPurchaseInfo(false, mProductId,
              "No purchase outstanding for the given purchase request");
          return;
        }

        NativeSetPurchaseInfo(true, storedProductId, null);

      } catch (Exception e) {
        NativeSetPurchaseInfo(false, mProductId, e.getMessage());
      }
    }

    @Override
    public void onFailure(int errorCode, String errorMessage, Bundle optionalData) {
      NativeSetPurchaseInfo(false, mProductId, errorMessage);
    }

    @Override
    public void onCancel() {
      NativeSetPurchaseInfo(false, mProductId, CANCEL_PURCHASE_MESSAGE);
    }
  }

  @SuppressLint("TrulyRandom")
  public static void requestPurchase(String productId) {

    try {
      SecureRandom sr = SecureRandom.getInstance("SHA1PRNG");
      String uniqueId = Long.toHexString(sr.nextLong());
  
      JSONObject purchaseRequest = new JSONObject();
      purchaseRequest.put("uuid", uniqueId);
      purchaseRequest.put("identifier", productId);
      String purchaseRequestJson = purchaseRequest.toString();
  
      byte[] keyBytes = new byte[16];
      sr.nextBytes(keyBytes);
      SecretKey key = new SecretKeySpec(keyBytes, "AES");
  
      byte[] ivBytes = new byte[16];
      sr.nextBytes(ivBytes);
      IvParameterSpec iv = new IvParameterSpec(ivBytes);
  
      Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding", "BC");
      cipher.init(Cipher.ENCRYPT_MODE, key, iv);
      byte[] payload = cipher.doFinal(purchaseRequestJson.getBytes("UTF-8"));
  
      cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding", "BC");
      cipher.init(Cipher.ENCRYPT_MODE, mPublicKey);
      byte[] encryptedKey = cipher.doFinal(keyBytes);
  
      Purchasable purchasable =
          new Purchasable(
              productId,
              Base64.encodeToString(encryptedKey, Base64.NO_WRAP),
              Base64.encodeToString(ivBytes, Base64.NO_WRAP),
              Base64.encodeToString(payload, Base64.NO_WRAP) );
  
      synchronized (mOutstandingPurchaseRequests) {
        mOutstandingPurchaseRequests.put(uniqueId, productId);
      }
      
      facade.requestPurchase(purchasable, new PurchaseListener(productId));
    } catch (Exception e) {
      Log.e(LOG_TAG, e.getMessage(), e);
    }
  }
  
  // Purchase receipt

  // receipt info request callback
  private static final CancelIgnoringOuyaResponseListener<String> receiptListListener =
      new CancelIgnoringOuyaResponseListener<String>() {
          @Override
          public void onSuccess(String receiptResponse) {
              OuyaEncryptionHelper helper = new OuyaEncryptionHelper();
              List<Receipt> receipts = null;
              try {
                  JSONObject response = new JSONObject(receiptResponse);
                  receipts = helper.decryptReceiptResponse(response, mPublicKey);
              } catch (Exception e) {
                  NativeSetReceiptInfo(false, false, null, e.getMessage());
                  return;
              }

              for (Receipt r : receipts) {
                NativeSetReceiptInfo(true, false, r.getIdentifier(), null);  
              }
              NativeSetReceiptInfo(true, true, null, null);
          }

          @Override
          public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
            NativeSetReceiptInfo(false, false, null, errorMessage);
          }
      };

  public static void requestReceipts() {
    facade.requestReceipts(receiptListListener);
  }
 
  // Gamer info

  // gamer info request callback
  private static final CancelIgnoringOuyaResponseListener<GamerInfo> gamerInfoListener =
      new CancelIgnoringOuyaResponseListener<GamerInfo>() {

    @Override
    public void onSuccess(GamerInfo result) {
      NativeSetGamerInfo(true, result.getUsername(), result.getUuid(), null);        
    }
  
    @Override
    public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
      NativeSetGamerInfo(false, null, null, errorMessage);
    }
  };

  public static void requestGamerInfo() {
    facade.requestGamerInfo(gamerInfoListener);
  }
  
  // Storage
  
  public static void putGameData(String name, String value) {
    facade.putGameData(name, value);
  }
  
  public static String getGameData(String name) {
    return facade.getGameData(name);
  }
}