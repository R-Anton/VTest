// VTest.cpp : Defines the entry point for the application.
//
#include "VTest.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vulkan/vulkan.h>



int main() {
   HelloTriangleApplication app;

   try {
      app.run();

   }
   catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
      return EXIT_FAILURE;

   }

   return EXIT_SUCCESS;

}