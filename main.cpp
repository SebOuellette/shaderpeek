#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Glsl.hpp>
#include <iostream>
#include <string>
#include <time.h>
#include <chrono>

#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX

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

		// Generate a cursor position to use in a shader (origin is bottom left)
		sf::Vector2f cursorPos = sf::Vector2f(sf::Mouse::getPosition() - window.getPosition());
		cursorPos.y = window.getSize().y - cursorPos.y;


		shader.setUniform("time", time);
		shader.setUniform("resolution", sf::Glsl::Vec2(window.getSize()));
		shader.setUniform("cursor", sf::Glsl::Vec2(cursorPos));

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
	

	// Get the file path for this current program
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	std::string path;
	if (count != -1) {
		path = dirname(result);
	}

	std::string vertexShader = path + "/BaseVertShader.vert";

	if (argc >= 3) {
		vertexShader = argv[2];
	}

	openWindow(argv[1], vertexShader);

    return 0;
}
