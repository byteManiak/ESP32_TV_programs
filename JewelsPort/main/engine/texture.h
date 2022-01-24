#pragma once

#include <string>

/**
 * @brief Create a texture from a file
 * 
 * @param path Path to the texture file
 * @param name Name given to keep track of the texture
 */
void createTexture(std::string path, std::string name);

/**
 * @brief Delete a texture object from memory
 * 
 * @param name Name given to the texture in createTexture() 
 */
void deleteTexture(std::string name);

void drawTexture(std::string name, int x, int y, int w, int h, int sx, int sy, int scaleFactor = 1);