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

/**
  The ZgeOuya Android shared library provides interface
  to the OUYA In-App Purchasing (IAP) and Storage
  mechanisms for ZGameEditor.

  Project home
  http://code.google.com/p/zgeouya/
*/

// Definitions

#define export extern "C"

//#define LOGGING
#define LOG_TAG "ZgeOuya"

#ifdef LOGGING
#	define LOG_DEBUG(str) __android_log_write(ANDROID_LOG_DEBUG, LOG_TAG, str);
#else
#	define LOG_DEBUG(str)
#endif

// returned values
#define YES 1
#define NO 0
#define ERROR -1

// Includes

#include <cstdlib>
#include <jni.h>
#include <android/log.h>
#include <map>
#include <vector>
#include <string>

// Types

struct ProductInfo{
	std::string name;
	std::string price;
	std::string description;
};

struct GamerInfo{
	std::string username;
	std::string uuid;
};

struct Purchase{
	int result; // = YES / NO / ERROR
	std::string errorMessage;
};

// Globals

static JNIEnv *env;
static jclass zgeOuyaCls;
static jmethodID isOuyaMethod;
static jmethodID initPurchasingMethod;
static jmethodID stopPurchasingMethod;
static jmethodID requestProductsMethod;
static jmethodID requestPurchaseMethod;
static jmethodID requestGamerInfoMethod;
static jmethodID requestReceiptsMethod;
static jmethodID putGameDataMethod;
static jmethodID getGameDataMethod;

std::string gErrorMsg;

int gProductRequestResult; // = YES / NO / ERROR
std::map<std::string,ProductInfo> gProductInfo;

std::map<std::string,Purchase> gPurchases; // <product id, purchase>

int gReceiptRequestResult; // = YES / NO / ERROR
std::vector<std::string> gReceipts;

int gGamerRequestResult; // = YES / NO / ERROR
GamerInfo gGamerInfo;

// Utils

// converts jstring to std::string
std::string jstring2string(jstring value)
{
	const char *n = env->GetStringUTFChars(value, NULL);
	std::string result = n;
	env->ReleaseStringUTFChars(value, n);

	return result;
}

// Functions

// Initialize JNI; used to obtain pointers to Java functions
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	LOG_DEBUG("JNI OnLoad called");

	int result;

	if(!env){ // if called multiple times, initialize static members just once
		// get JNI environment
		result = vm->GetEnv((void**) &env, JNI_VERSION_1_6);

		// set globals JNI variables
		zgeOuyaCls = env->FindClass("org/zgameeditor/ZgeOuya");
		isOuyaMethod = env->GetStaticMethodID(zgeOuyaCls, "isOuya", "()Z");
		initPurchasingMethod = env->GetStaticMethodID(zgeOuyaCls, "initPurchasing", "(Ljava/lang/String;)I");
		stopPurchasingMethod = env->GetStaticMethodID(zgeOuyaCls, "stopPurchasing", "()V");
		requestProductsMethod = env->GetStaticMethodID(zgeOuyaCls, "requestProducts", "([Ljava/lang/String;)V");
		requestPurchaseMethod = env->GetStaticMethodID(zgeOuyaCls, "requestPurchase", "(Ljava/lang/String;)V");
		requestGamerInfoMethod = env->GetStaticMethodID(zgeOuyaCls, "requestGamerInfo", "()V");
		requestReceiptsMethod = env->GetStaticMethodID(zgeOuyaCls, "requestReceipts", "()V");
		putGameDataMethod = env->GetStaticMethodID(zgeOuyaCls, "putGameData", "(Ljava/lang/String;Ljava/lang/String;)V");
		getGameDataMethod = env->GetStaticMethodID(zgeOuyaCls, "getGameData", "(Ljava/lang/String;)Ljava/lang/String;");
	}

    return JNI_VERSION_1_6;
}

// Generic

export bool ouya_IsOuya(){
	LOG_DEBUG("ouya_IsOuya called");

	return (bool)env->CallStaticBooleanMethod(zgeOuyaCls, isOuyaMethod);
}

// In-App Purchasing (IAP)

export int ouya_InitPurchasing(char* developerId){
	LOG_DEBUG("ouya_InitPurchasing called");

	return env->CallStaticIntMethod(zgeOuyaCls, initPurchasingMethod,
							env->NewStringUTF(developerId));
}

export void ouya_StopPurchasing(){
	LOG_DEBUG("ouya_StopPurchasing called");

	env->CallStaticVoidMethod(zgeOuyaCls, stopPurchasingMethod);
}

export void ouya_RequestProducts(char** productIds, int productNum){
	LOG_DEBUG("ouya_RequestProducts called");

	jobjectArray productIdArray = env->NewObjectArray(productNum, env->FindClass("java/lang/String"),0);
	jstring id;

	for(int i=0; i < productNum; ++i){
		id = env->NewStringUTF(productIds[i]);
		env->SetObjectArrayElement(productIdArray, i, id);
	}

	env->CallStaticVoidMethod(zgeOuyaCls, requestProductsMethod, productIdArray);

	gProductInfo.clear();
	gProductRequestResult = NO;
}

export int ouya_IsProductRequestDone(const char* &errorMessage){
	LOG_DEBUG("ouya_IsProductRequestDone called");

	if(gProductRequestResult == ERROR) errorMessage = gErrorMsg.c_str();
	return gProductRequestResult;
}

export int ouya_GetProductInfo(char* productId, const char* &name, const char* &price, const char* &description){
	LOG_DEBUG("ouya_GetProductInfo called");

	if(gProductRequestResult == YES){

		name = gProductInfo[productId].name.c_str();
		price = gProductInfo[productId].price.c_str();
		description = gProductInfo[productId].description.c_str();
	}
	return gProductRequestResult;
}

export JNIEXPORT void JNICALL
Java_org_zgameeditor_ZgeOuya_NativeSetProductInfo(JNIEnv *env, jclass cls,
	jboolean success, jstring productId, jstring name, jstring price,
	jstring description, jstring errorMessage){

	LOG_DEBUG("NativeSetProductInfo called");

	if((bool)success){

		ProductInfo newProductInfo;

		newProductInfo.name = jstring2string(name);
		newProductInfo.price = jstring2string(price);
		newProductInfo.description = jstring2string(description);

		gProductInfo.insert(std::pair<std::string,ProductInfo>
							(jstring2string(productId), newProductInfo));

		gProductRequestResult = YES;
	} else {

		gErrorMsg = jstring2string(errorMessage);
		gProductRequestResult = ERROR;
	}
}

export void ouya_RequestPurchase(char* productId){
	LOG_DEBUG("ouya_RequestPurchase called");

	Purchase newPurchase;
	newPurchase.result = NO;

	gPurchases.insert(std::pair<std::string,Purchase>(productId, newPurchase));

	env->CallStaticVoidMethod(zgeOuyaCls, requestPurchaseMethod,
							env->NewStringUTF(productId));
}

export int ouya_IsPurchaseRequestDone(char* productId, const char* &errorMessage){
	LOG_DEBUG("ouya_IsPurchaseRequestDone called");

	if(gPurchases.count(productId) > 0){
		// product purchase request exists
		if(gPurchases[productId].result == ERROR)
			errorMessage = gPurchases[productId].errorMessage.c_str();

		return gPurchases[productId].result;

	} else {
		// product has not been requested for purchase
		errorMessage = "Product has not been requested for purchase";
		return ERROR;
	}
}

export void ouya_ClearPurchases(){
	LOG_DEBUG("ouya_ClearPurchases called");

	gPurchases.clear();
}

export JNIEXPORT void JNICALL
Java_org_zgameeditor_ZgeOuya_NativeSetPurchaseInfo(JNIEnv *env, jclass cls,
	jboolean success, jstring productId, jstring errorMessage){

	LOG_DEBUG("NativeSetPurchaseInfo called");

	const char *id = env->GetStringUTFChars(productId, NULL);

	if((bool)success){
		gPurchases[id].result = YES;
	} else {
		gPurchases[id].errorMessage = jstring2string(errorMessage);
		gPurchases[id].result = ERROR;
	}

	env->ReleaseStringUTFChars(productId, id);
}

export void ouya_RequestReceipts(){
	LOG_DEBUG("ouya_RequestReceipts called");

	env->CallStaticVoidMethod(zgeOuyaCls, requestReceiptsMethod);

	gReceipts.clear();
	gReceiptRequestResult = NO;
}

export int ouya_IsReceiptRequestDone(int &numberOfReceipts, const char* &errorMessage){
	LOG_DEBUG("ouya_IsReceiptRequestDone called");

	switch(gReceiptRequestResult){
		case YES:
			numberOfReceipts = gReceipts.size();
			break;

		case ERROR:
			errorMessage = gErrorMsg.c_str();
	}

	return gReceiptRequestResult;
}

export int ouya_GetReceiptInfo(int id, const char* &productId){
	LOG_DEBUG("ouya_GetReceiptInfo called");

	if(id >= 0 && id < gReceipts.size()){
		productId = gReceipts[id].c_str();
		return YES;
	} else
		return NO;
}

export JNIEXPORT void JNICALL
Java_org_zgameeditor_ZgeOuya_NativeSetReceiptInfo(JNIEnv *env, jclass cls,
	jboolean success, jboolean finished, jstring productId, jstring errorMessage){

	LOG_DEBUG("NativeSetReceiptInfo called");

	if((bool)success){
		if((bool)finished)
			// all receipts received
			gReceiptRequestResult = YES;
		else
			// next receipt
			gReceipts.push_back(jstring2string(productId));

	} else {

		gErrorMsg = jstring2string(errorMessage);
		gReceiptRequestResult = ERROR;
	}
}

export void ouya_RequestGamerInfo(){
	LOG_DEBUG("ouya_RequestGamerInfo called");

	env->CallStaticVoidMethod(zgeOuyaCls, requestGamerInfoMethod);
	gGamerRequestResult = NO;
}

export int ouya_IsGamerInfoRequestDone(const char* &username,
	const char* &uuid, const char* &errorMessage){

	LOG_DEBUG("ouya_IsGamerInfoRequestDone called");

	switch(gGamerRequestResult){
		case YES:
			username = gGamerInfo.username.c_str();
			uuid = gGamerInfo.uuid.c_str();
			break;

		case ERROR:
			errorMessage = gErrorMsg.c_str();
	}

	return gGamerRequestResult;
}

export JNIEXPORT void JNICALL
Java_org_zgameeditor_ZgeOuya_NativeSetGamerInfo(JNIEnv *env, jclass cls,
	jboolean success, jstring username, jstring uuid, jstring errorMessage){

	LOG_DEBUG("NativeSetGamerInfo called");

	if((bool)success){
		gGamerInfo.username = jstring2string(username);
		gGamerInfo.uuid = jstring2string(uuid);
		gGamerRequestResult = YES;
	} else {
		gErrorMsg = jstring2string(errorMessage);
		gGamerRequestResult = ERROR;
	}
}

// Storage

export void ouya_PutGameData(char* name, char* value){
	LOG_DEBUG("ouya_PutGameData called");

	env->CallStaticVoidMethod(zgeOuyaCls, putGameDataMethod,
			env->NewStringUTF(name), env->NewStringUTF(value));
}

export const char* ouya_GetGameData(char* name){
	LOG_DEBUG("ouya_GetGameData called");

	jstring data = (jstring) env->CallStaticObjectMethod(zgeOuyaCls, getGameDataMethod,
										env->NewStringUTF(name));
	return data == NULL ? "" : env->GetStringUTFChars(data, NULL);
}
