#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <limits>
#include <Windows.h>

const long bitrate{ 44100 };
const int bitdepth{ 16 };
const float pi{ 3.1416f };

/* WIP. getting all inital frequency data from the file is done. Then the maths to find the harmonic series should be quite simple, then 
* I can feed that into some form of pattern class which will store the series and have a process to create the waveform vector.
* then I'll just write that to file and Robbie's my uncle. */

void clearCin()
{
	std::cin.clear();
	std::cin.ignore(9999999999, '\n');
}

class Note
{
public:
	double m_freq{};
	char m_name[2];					//using a c-style array here to allow me to use switch statements later
	int m_octave{};					
	short m_number{};				//which number a note is in the octave (0-11); allows skipping through the file much more easily.

	void get_note()
	{
		std::string note_input{};
		std::string file_freq{};

		system("cls");

invalid_input:
		std::cout << "Enter a note: ";
		std::cin >> note_input;

		note_input[0] = std::toupper(note_input[0]);

		system("cls");

		if (note_input.size() == 3)					//if the note has a sharp or flat
		{
			if (note_input[1] == '#')				//if it's sharp
			{
				m_name[0] = static_cast<char>(note_input[0] + 1);	//changes it to be the note above but flattened, as all notes are stored as flattened naturals.
				m_name[1] = 'b';
			}										
			else if (note_input[1] == 'b') 			//if it's flat
			{
				m_name[0] = note_input[0];
				m_name[1] = note_input[1];
			}
			else
			{
				clearCin();							//if an invalid modifier was used
				std::cout << "Invalid modifier on note (i.e. D@ instead of D#). Please try again.\n";
				note_input = "\0";
				goto invalid_input;
			}

			m_octave = note_input[2] - 48;

			switch (m_name[0])
			{
			case 'D':
				m_number = 1;
				break;

			case 'E':
				m_number = 3;
				break;

			case 'G':
				m_number = 6;
				break;

			case 'A':
				m_number = 8;
				break;

			case 'B':
				m_number = 10;
				break;

			default:
				clearCin();							//if the inputted note doesn't exist
				std::cout << "Invalid note (i.e. P instead of a letter from A to G, or E#, Fb, B#, Cb. These notes are other natural notes.). \nPlease try again.\n";
				note_input = "\0";
				goto invalid_input;
			}
		}
		else if (note_input.size() == 2)				//if the note is a natural
		{
			m_name[0] = note_input[0];
			m_octave = note_input[1] - 48;

			switch (m_name[0])
			{
			case 'C':
				m_number = 0;
				break;

			case 'D':
				m_number = 2;
				break;

			case 'E':
				m_number = 4;
				break;

			case 'F':
				m_number = 5;
				break;

			case 'G':
				m_number = 7;
				break;

			case 'A':
				m_number = 9;
				break;

			case 'B':
				m_number = 11;
				break;

			default:
				clearCin();							//if the inputted note doesn't exist
				std::cout << "Invalid note (i.e. P instead of a letter from A to G, or E#, Fb, B#, Cb. These notes are other natural notes.). \nPlease try again.\n";
				note_input = "\0";
				goto invalid_input;
			}
		}
		else
		{
			clearCin();							//if the string contains the wrong number of characters
			std::cerr << "Incorrect note input - wrong number of characters in note name. Please try again.\n";
			note_input = "\0";
			goto invalid_input;
		}

		std::ifstream ifs("frequencies.txt");

		if (!ifs)					//in case the filestream fails to open
		{
			std::cout << "Error opening the file frequencies.txt; please check that the file exists under this name in the same directory as this program.\n";
			std::cout << "If it does not, it can be downloaded from <https://github.com/Weharn/harmonic.series>. The program will close wheen you continue.\n";
			system("pause");
			std::abort();
		}

		ifs.seekg((m_number * 76) + (m_octave * 8) + 3);			//skips to the right collection of octaves (notes are in 76-character chunks)
																	//and the right octave within the collection (individual notes are in 8-character words)
		std::getline(ifs, file_freq, ',');

		m_freq = std::stof(file_freq);								//changes the string input into the float frequency
	}
};

class Sine
{
private:

	float m_angle{};		//the current "angle" of the sine wave
	float m_step{};			//the "step" (increase in angle per iteration)

public:

	int m_postwr_pos{};
	float m_amplitude{};
	float m_frequency{};
	float m_duration{};

	void init(float ampl, float freq, float durn)			//initialises all the variables and the step value
	{
		m_amplitude = ampl;
		m_frequency = freq;
		m_duration = durn;
		m_step = sin(2 * pi * (m_frequency / bitrate));
	}

	float gen_sine()										//generates an indiviual sample (range: -1 to 1), then increments the angle by the step
	{
		float sample{ sin(m_angle) };

		m_angle += m_step;

		return sample;
	}

	void write(std::ofstream& ofs)							//writes the wave to file directly
	{
		auto amplmod{ (pow(2, (bitdepth - 1)) - 1) };		//modifier to take the range from -1 to 1 to -32767 to 32767

		for (int i{}; i < (bitrate * m_duration); ++i)
		{
			int sample{ static_cast<int>(gen_sine() * amplmod) };		//does the final modification and casts the sample to an int as required

			ofs.write((reinterpret_cast<char*>(&sample)), 2);
		}

		m_postwr_pos = ofs.tellp();
	}
};

class Harmonic_Series
{
public:

	float m_frequency{};
	int m_postwr_pos{};	
	std::vector<float> m_series{ 0 };

	void ovt_generate()					//generates the overtone harmonic series
	{
		for (int i{ 1 }; i <= 16; i++)
		{
			m_series.push_back(m_frequency * i);
		}
	}   

	void undt_generate()				//generates the undertone harmonic series
	{
		for (float i{ 1 }; i <= 16.05; i += 1)
		{
			m_series.push_back(m_frequency * (1/i));
		}
	}

	void seq_series(std::ofstream& ofs, float amplitude, float duration)			//writes the series directly to a file
	{
		for (int i{ 1 }; i < m_series.size(); ++i)
		{
			Sine sine{};
			sine.init(amplitude, m_series[i], duration);
			sine.write(ofs);

			m_postwr_pos = sine.m_postwr_pos;
		}
	}

	void lay_series(std::ofstream& ofs, float amplitude, float duration)
	{
		for (int i{ 1 }; i < m_series.size(); ++i)					//this loop runs through every harmonic
		{
			if (i == 1)												//these first two if statements are just to reduce some of the load when it is less necessary
			{
				Sine sine{};
				sine.init(amplitude, m_series[i], duration);
				sine.write(ofs);
			}
			else if (i == 2)
			{
				Sine sine1{};
				sine1.init(amplitude, m_series[i - 1], duration);
				Sine sine2 {};
				sine2.init(amplitude, m_series[i], duration);

				auto amplmod{ (pow(2, (bitdepth - 1)) - 1) };		//modifier to take the range from -1 to 1 to -32767 to 32767

				for (int i{}; i < (bitrate * duration); ++i)
				{	
					//does the final modification and casts the sample to an int as required
					int sample{ static_cast<int>(((sine1.gen_sine() * 0.5) + (sine2.gen_sine() * 0.5)) * amplmod)};		

					ofs.write((reinterpret_cast<char*>(&sample)), 2);
				}
			}
			else if (i > 2 && i < m_series.size())
			{
				auto amplmod{ (pow(2, (bitdepth - 1)) - 1) };		//modifier to take the range from -1 to 1 to -32767 to 32767
				std::vector<Sine> series_vec{};						//a vector of sine objects. The plan is that they will all be .gen_sine 'd simultaneously and added every cycle

				for (int j{ 1 }; j <= i; ++j)						//initialises the correct number of objects to the correct frequencies withing series_vec
				{
					Sine sine{};
					sine.init(amplitude, m_series[j], duration);

					series_vec.push_back(sine);
				}

				double vecsize_recip{ 1 / static_cast<double>(series_vec.size()) };		//saves having to keep redoing some intensive work

				for (int j{}; j < (bitrate * duration); ++j)
				{
					double sum{};														//current sum of all the sine objects within the vector for a given step

					for (int k{}; k < series_vec.size(); ++k)
					{
						sum += vecsize_recip * series_vec.at(k).gen_sine();				//adds the balanced amount of whatever the value of one of the sine objects in series_vec is to sum
					}

					int sample{ static_cast<int>(sum * amplmod) };						//prepares sum for writing to file

					ofs.write((reinterpret_cast<char*>(&sample)), 2);					//writes to file
				}
			}
			else
			{
				std::cerr << "i out of range. wth even happened to do that. it's 1:48am.";
				std::abort();
			}
		}

		m_postwr_pos = ofs.tellp();
	}

};

class Header
{
public:
	int m_preaudiop{};									//records the position in the file before audio is written
	std::ofstream& ofs;									//creates the filestream reference

	Header(std::ofstream& tmpofs) : ofs{ tmpofs }		//initialises the filestream reference
	{
	}

	template <typename T>								//template for writing general information to the filstream
	void writeofs(std::ofstream& ofs, T value, int size)
	{
		ofs.write(reinterpret_cast<const char*> (&value), size);
	}

	void header_i()
	{
		//Header chunk
		ofs << "RIFF";
		ofs << "----";
		ofs << "WAVE";

		// Format chunk
		ofs << "fmt ";
		writeofs(ofs, 16, 4); // Size
		writeofs(ofs, 1, 2); // Compression code
		writeofs(ofs, 1, 2); // Number of channels
		writeofs(ofs, bitrate, 4); // Sample rate
		writeofs(ofs, bitrate * bitdepth / 8, 4); // Byte rate
		writeofs(ofs, bitdepth / 8, 2); // Block align
		writeofs(ofs, bitdepth, 2); // Bit depth

		//Data chunk
		ofs << "data";
		ofs << "----";

		m_preaudiop = ofs.tellp();						//records the position in the file before audio is written
	}

	void header_c(int postaudiop)						//writes in the footer
	{
		ofs.seekp(m_preaudiop - 4);
		writeofs(ofs, postaudiop - m_preaudiop, 4);

		ofs.seekp(4, std::ios::beg);
		writeofs(ofs, postaudiop - 8, 4);
	}

};

void run()
{
	double duration{};
	float amplitude{};
	int postwr_pos{};
	char over_under{};
	char seq_lay{};

invalid_dur:
	std::cout << "Enter duration per note (seconds): ";
	std::cin >> duration;

	if (duration <= 0)				//in case duration has an invalid value
	{
		duration = 0;
		clearCin();
		std::cout << "You gave duration as zero (no sound will play) or a negative number (impossible). Please try again.\n";
		goto invalid_dur;
	}

	system("cls");

invalid_amp:
	std::cout << "Enter amplitude (zero to one scale): ";
	std::cin >> amplitude;

	if (amplitude <= 0 || amplitude > 1)				//in case amplitude has an invalid value
	{
		amplitude = 0;
		clearCin();
		std::cout << "Invalid value for amplitude. Please try again.\n";
		goto invalid_amp;
	}

	system("cls");

	Note note{};
	note.get_note();
	Harmonic_Series series{ note.m_freq };

invalid_ou:
	std::cout << "Would you like to generate the overtone or undertone series? (O/U): ";
	std::cin >> over_under;

	over_under = std::toupper(over_under);

	if (over_under == 'O')
	{
		series.ovt_generate();
	}
	else if (over_under == 'U')
	{
		series.undt_generate();
	}
	else									//in case the choice was invalid
	{
		std::cout << "Invalid choice. Enter O to generate the overtone series, or U to generate the undertone series. Please try again.\n";
		clearCin();
		over_under = '\0';
		goto invalid_ou;
	}

	system("cls");

	wchar_t path[256];
	int path_terminus{ -1 };
	std::string str_path{};

	GetModuleFileName(NULL, path, 256);

	for (int i{}; (i < 256 && i != path_terminus); ++i)
	{
		if (path[i] == '\0')
		{
			path_terminus = i;
		}
	}

	for (int i{ 0 }; i < (path_terminus - 20); ++i)
	{
		str_path.push_back(path[i]);
	}

	str_path += "\\harmonic_series.wav";

	std::ofstream ofs(str_path, std::ios::binary);

	if (!ofs.is_open())
	{
		std::cerr << "failed to open the ofs";
		std::abort();
	}

	Header header(ofs);
	header.header_i();

invalid_sl:
	std::cout << "Would you like create the series in a sequential or layered form (S/L): ";
	std::cin >> seq_lay;

	seq_lay = std::toupper(seq_lay);

	if (seq_lay == 'S')
	{
		series.seq_series(ofs, amplitude, duration);
	}
	else if (seq_lay == 'L')
	{
		series.lay_series(ofs, amplitude, duration);
	}
	else									//in case the choice was invalid
	{
		std::cout << "Invalid choice. Enter S to generate the sequential form, or L to generate the layered form. Please try again.\n";
		clearCin();
		seq_lay = '\0';
		goto invalid_sl;
	}

	system("cls");

	header.header_c(postwr_pos);

	std::cout << "Done!\n\n";
}

int main()
{
	run();

	system("pause");

	return 0;
}