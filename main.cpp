#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include <gpiod.h>

namespace {
volatile std::sig_atomic_t keep_running = 1;

void handle_signal(int) { 
      keep_running = 0; 
}
}  // namespace

int main()
{
	constexpr const char* chip_name = "/dev/gpiochip0";
	constexpr unsigned int gpio_line = 13;

	std::signal(SIGINT, handle_signal);
	std::signal(SIGTERM, handle_signal);

	gpiod_chip* chip = gpiod_chip_open(chip_name);
	if (chip == nullptr) {
		std::cerr << "Erreur: impossible d'ouvrir " << chip_name << std::endl;
		return 1;
	}

	gpiod_line* line = gpiod_chip_get_line(chip, gpio_line);
	if (line == nullptr) {
		std::cerr << "Erreur: impossible de recuperer la ligne GPIO " << gpio_line << std::endl;
		gpiod_chip_close(chip);
		return 1;
	}

	if (gpiod_line_request_output(line, "ventilator_controller", 0) < 0) {
		std::cerr << "Erreur: impossible de configurer GPIO " << gpio_line << " en sortie" << std::endl;
		gpiod_chip_close(chip);
		return 1;
	}

	std::cout << "GPIO " << gpio_line << " toggle toutes les 2 secondes. Ctrl+C pour quitter." << std::endl;

	int value = 0;
	while (keep_running) {
		value = (value == 0) ? 1 : 0;
		if (gpiod_line_set_value(line, value) < 0) {
			std::cerr << "Erreur: echec de l'ecriture GPIO" << std::endl;
			break;
		}

		std::cout << "GPIO " << gpio_line << " = " << value << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	gpiod_line_set_value(line, 0);
	gpiod_line_release(line);
	gpiod_chip_close(chip);

	return 0;
}
