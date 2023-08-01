#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Glsl.hpp>
//#include <SFML/RenderTexture.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <time.h>
#include <cmath>
#include <chrono>

#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX

using std::chrono::milliseconds;

#define CLICK_CACHE_COUNT 10

// Create a a new window
void openWindow(std::string fragName, std::string vertexName) {
	sf::RenderWindow window(sf::VideoMode(700, 700), std::string("Shaderpeek - ") + fragName);
	window.setKeyRepeatEnabled(false);
	window.setFramerateLimit(144);
	//window.setVerticalSyncEnabled(true);
	

	// Create a shape as the shader's canvas
	sf::RectangleShape canvas((sf::Vector2f)window.getSize());
	canvas.setFillColor(sf::Color::Green);

	sf::Shader shader;
	// load only the fragment shader
	if (!shader.loadFromFile(vertexName, fragName))
		exit(1);

	int frame = 0;

	// Create the array to store the previous CLICK_CACHE_COUNT clicks
	sf::Glsl::Vec2 lastclicks[CLICK_CACHE_COUNT] = { sf::Glsl::Vec2(-1, -1) };
	int clickIndex = 0;

	
	sf::RenderTexture* texture = new sf::RenderTexture();
	if (!texture->create(window.getSize().x, window.getSize().y)) {
		std::cerr << "Could not create render texture? Idk man." << std::endl;
		exit(-1);
	}

	sf::RenderTexture* backBuffer = new sf::RenderTexture();
	if (!backBuffer->create(window.getSize().x, window.getSize().y)) {
		std::cerr << "Could not create back buffer texture." << std::endl;
		exit(-1);
	}

	sf::Vector2 size = window.getSize();


	// Active loop
	while (window.isOpen()) {
		
		
		// Generate a cursor position to use in a shader (origin is bottom left)
		sf::Vector2i cursorPos = sf::Mouse::getPosition() - window.getPosition();
		cursorPos.y = size.y - cursorPos.y;

		// Generate a window position to use in a shader (origin is bottom left)
		sf::Vector2i windowPos = window.getPosition();
		windowPos.y = sf::VideoMode::getDesktopMode().height - windowPos.y - size.y;

		float scrolldelta = 0;

		// Poll for any events (clicking the window close button in this case)
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {

				delete backBuffer;
				delete texture;

				window.close();
			} else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					lastclicks[clickIndex++] = sf::Glsl::Vec2(cursorPos);
					clickIndex %= CLICK_CACHE_COUNT;

					//std::cout << "Clicked at (" << cursorPos.x << ", " << cursorPos.y << ")" << std::endl;
				}
			} else if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
				scrolldelta = event.mouseWheelScroll.delta;


			} else if (event.type == sf::Event::Resized) {
				size.x = event.size.width;
				size.y = event.size.height;


				sf::FloatRect view(0, 0, size.x, size.y);
        			window.setView(sf::View(view));

				canvas.setSize(sf::Vector2f(size));

				// Resize texture
				delete texture;
				texture = new sf::RenderTexture();
				texture->create(size.x, size.y);

			
				
				// Resize backBuffer
				delete backBuffer;
				backBuffer = new sf::RenderTexture();
				backBuffer->create(size.x, size.y);
			}
		}

		// Pass variables to shaders for them to use
		float time = std::chrono::duration_cast<milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % 10000000 / 1000.f;

		shader.setUniform("time", time);
		shader.setUniform("resolution", sf::Glsl::Vec2(size));
		shader.setUniform("cursor", sf::Glsl::Vec2(cursorPos));
		shader.setUniform("frame", frame++);
		shader.setUniform("scroll", scrolldelta);
		shader.setUniform("spacebar", sf::Keyboard::isKeyPressed(sf::Keyboard::Space));
		shader.setUniform("mouseL", sf::Mouse::isButtonPressed(sf::Mouse::Left));
		shader.setUniform("mouseR", sf::Mouse::isButtonPressed(sf::Mouse::Right));
		shader.setUniformArray("lastClicks", lastclicks, CLICK_CACHE_COUNT);
		shader.setUniform("windowPos", sf::Glsl::Vec2(windowPos));
		shader.setUniform("lastFrame", backBuffer->getTexture());
			

		// Draw the shader to the texture
		texture->clear();
		texture->draw(canvas, &shader);
		texture->display();

		// Clear the last frame, and draw the current texture
		window.clear();
		
		sf::Sprite sprite(texture->getTexture()); // Convert the texture to a sprite to draw it
		window.draw(sprite);
		
		window.display();

		// Set backbuffer
		backBuffer->clear();
		backBuffer->draw(sprite);
		backBuffer->display();
    }
}

// Main program entry point
int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cerr << "Fragment shader path is REQUIRED" << std::endl;
		std::cerr << "Example format: shaderpeek fragmentShader.frag vertexShader.vert" << std::endl;
		exit(1);
	}
	

	// Get the file path for this current program
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	std::string path;
	if (count != -1) {
		path = dirname(result);
	}

	std::string vertexShader = path + "/BaseVertShader.vert";

	if (argc >= 3)
		vertexShader = argv[2];

	openWindow(argv[1], vertexShader);

    return 0;
}
