#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector> 

#include <SDL.h>
#undef main // "Hack" because else we get an error using SDL
#include <glad/glad.h>

// We'll use windows with OpenGL for this
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_OPENGL
#define XR_KHR_OPENGL_ENABLE_EXTENSION_NAME
#include "openxr/openxr.h"

bool initOpenXr();

int main() {
	if (!initOpenXr()) {
		return -1;
	}

	return 0;
}

// --------------------------------------------------------------------------------------------------------
// Initialize Open XR
// --------------------------------------------------------------------------------------------------------
bool initOpenXr() {
	XrResult result;

	// --------------------------------------------------------------------------------------------------------
	// Check that runtime supports OpenGL
	// --------------------------------------------------------------------------------------------------------
	unsigned int extensionCount = 0;

	result = xrEnumerateInstanceExtensionProperties(NULL, 0, &extensionCount, NULL);
	if (result != XR_SUCCESS) {
		std::cout << "Failed to enumerate number of extension properties" << std::endl;
		return false;
	}
	std::cout << "Runtime supports " << extensionCount << " extensions" << std::endl;

	XrExtensionProperties* extensionProperties = new XrExtensionProperties[extensionCount];
	for (uint16_t i = 0; i < extensionCount; i++) {
		// we usually have to fill in the type (for validation) and set
		// next to NULL (or a pointer to an extension specific struct)
		extensionProperties[i].type = XR_TYPE_EXTENSION_PROPERTIES;
		extensionProperties[i].next = NULL;
	}


	result = xrEnumerateInstanceExtensionProperties(NULL, extensionCount, &extensionCount, extensionProperties);
	if (result != XR_SUCCESS) {
		std::cout << "Failed to enumerate number of extension properties" << std::endl;
		return false;
	}

	bool openGlSupported = false;
	for (unsigned int i = 0; i < extensionCount; i++) {
		std::cout << extensionProperties[i].extensionName << std::endl;
		if (strcmp("XR_KHR_opengl_enable", extensionProperties[i].extensionName) == 0) {
			openGlSupported = true;
			break;
		}
	}

	if (!openGlSupported) {
		std::cout << "OpenGL is not supported by the runtime" << std::endl;
		return false;
	}


	return true;
};