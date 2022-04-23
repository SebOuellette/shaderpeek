#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Glsl.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <string>
#include <time.h>
#include <chrono>
#include <thread>
#include <vector>

using std::chrono::milliseconds;

// Create a a new window
void openWindow(std::string fragName, std::string vertexName) {
	sf::RenderWindow window(sf::VideoMode(700, 700), std::string("FragViewer - ") + fragName);
	window.setFramerateLimit(144);
	window.setVerticalSyncEnabled(false);

	// Create a shape as the shader's canvas
	sf::RectangleShape canvas((sf::Vector2f)window.getSize());
	canvas.setFillColor(sf::Color::Green);

	sf::Shader shader;
	// load only the fragment shader
	if (!shader.loadFromFile(vertexName, fragName))
		exit(1);

	// Active loop
	while (window.isOpen()) {
		// Poll for any events (clicking the window close button in this case)
		sf::Event event;
		while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed)
					window.close();
		}

		// Pass variables to shaders for them to use
		float time = std::chrono::duration_cast<milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % 10000000 / 1000.f;

		shader.setUniform("time", time);
		shader.setUniform("resolution", sf::Glsl::Vec2(window.getSize()));
		shader.setUniform("cursor", sf::Glsl::Vec2(sf::Mouse::getPosition()));

		// Clear the last frame, and draw the current one
		window.clear();
		window.draw(canvas, &shader);
		window.display();
    }
}

// Main program entry point
int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cerr << "Expected at least a fragment shader filepath" << std::endl;
		exit(1);
	}
	

	//std::vector<std::thread> threads;

	// Open all the windows in their own threads
	// for (int i=1;i<argc;i++) {
    // 	//threads.push_back(std::thread(&openWindow, argv[i])); // 
		
    // }

	// // Wait for all the threads to close
	// for (int i=1;i<argc;i++) {
    // 	threads.back().join();
	// 	threads.pop_back();
    // }

	std::string vertexShader = "BaseVertShader.vert";

	if (argc >= 3) {
		vertexShader = argv[2];
	}

	openWindow(argv[1], vertexShader);

    return 0;
}
