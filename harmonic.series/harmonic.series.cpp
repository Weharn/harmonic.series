#include <iostream>
#include <vector>
#include <fstream>
#include <string>

const long bitrate{ 44100 };
const int bitdepth{ 16 };
const float pi{ 3.1416f };

/* WIP. getting all inital frequency data from the file is done. Then the maths to find the harmonic series should be quite simple, then 
* I can feed that into some form of pattern class which will store the series and have a process to create the waveform vector.
* then I'll just write that to file and Robbie's my uncle. */

class Note
{
public:
	double m_freq{};
	char m_name[2];
	int m_octave{};
	short m_number{};

	void get_note()
	{
		std::string note_input{};
		std::string file_freq{};

		std::cout << "Enter a note: ";
		std::cin >> note_input;

		if (note_input.size() == 3)
		{
			if (note_input[1] == '#')
			{
				m_name[0] = static_cast<char>(note_input[0] + 1);
				m_name[1] = 'b';
			}
			else if (note_input[1] = 'b')
			{
				m_name[0] = note_input[0];
				m_name[1] = note_input[1];
			}
			else
			{
				std::cerr << "Invalid modifier on note (i.e. D@ instead of D#)";
				std::abort();
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
				std::cerr << "Incorrect note input (i.e. F-flat, or Q).";
				std::abort();
			}
		}
		else if (note_input.size() == 2)
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
				std::cerr << "Invalding note inputted (i.e. Q)";
				std::abort();
			}
		}
		else
		{
			std::cerr << "Incorrect note input (i.e. F-flat, or Q). Specifially, wrong number of characters in note name.";
			std::abort();
		}

		std::ifstream ifs("frequencies.txt");

		if (!ifs)
		{
			std::cerr << "Error opening the file frequencies.txt; please check the path.";
			std::abort();
		}

		ifs.seekg((m_number * 76) + (m_octave * 8) + 3);

		std::getline(ifs, file_freq, ',');

		m_freq = std::stof(file_freq);
	}
};

class Harmonic_Series
{
public:

	float frequency{};
	std::vector<float> m_series{};

	void generate(float freq)
	{
		frequency = freq;

		for (int i{}; m_series.at(i) <= 200000; i++)
		{
			m_series.push_back(frequency * i);
		}
	}

};

class Sine
{
private:

	float m_angle{};
	float m_step{};

public:

	std::vector<float> m_amplitude{};
	std::vector<float> m_sinevec{};
	int m_postwr_pos{};
	float m_frequency{};
	float m_duration{};

	void init(std::vector<float> ampl, float freq, float durn)
	{
		m_step = sin(2 * pi * (m_frequency / bitrate));
		m_amplitude = ampl;
		m_frequency = freq;
		m_duration = durn;
	}

	void init(float durn)
	{
		m_step = sin(2 * pi * (m_frequency / bitrate));
		m_duration = durn;
	}

	float gen_sine()
	{
		float sample{ sin(m_angle) };

		m_angle += m_step;

		return sample;
	}

	void sine_vec()
	{
		float cycles{ m_duration * bitrate };

		for (int i{}; i < cycles; ++i)
		{
			m_sinevec.push_back(gen_sine());
		}
	}

	void write(std::ofstream& ofs)
	{
		auto amplmod{ (pow(2, (bitdepth - 1)) - 1) };

		for (int i{}; i < (bitrate * m_duration); ++i)
		{
			auto fsample{ m_sinevec[i] * amplmod };
			int isample{ static_cast<int>(fsample) };

			ofs.write((reinterpret_cast<char*>(&isample)), 2);
		}

		m_postwr_pos = ofs.tellp();
	}
};

class Master
{
public:

	int m_postwr_pos{};
	std::vector<float> m_master_vec{};

	void add(Sine& wave)															//adds a Sine object to m_master_vec
	{
		for (int i{}; i < wave.m_sinevec.size(); ++i)
		{
			m_master_vec.push_back(wave.m_sinevec[i]);
		}
	}

	void merge(Master& master1, Master& master2)													//merges two Master objects into m_master_vec
	{
		int i{};
		int larger_c{};

		if (master1.m_master_vec.size() < master2.m_master_vec.size())
		{
			larger_c = 0;
		}
		else if (master1.m_master_vec.size() > master2.m_master_vec.size())
		{
			larger_c = 1;
		}
		else
		{
			larger_c = 2;
		}

		size_t m1size{ master1.m_master_vec.size() };
		size_t m2size{ master2.m_master_vec.size() };

		if (larger_c == 0)
		{
			while (i < m1size)
			{
				m_master_vec.push_back((master1.m_master_vec[i] + master2.m_master_vec[i]) * 0.5);

				++i;
			}

			while (i < m2size)
			{
				m_master_vec.push_back(master2.m_master_vec[i]);

				++i;
			}

		}
		else if (larger_c == 1)
		{
			while (i < m2size)
			{
				m_master_vec.push_back((master1.m_master_vec[i] + master2.m_master_vec[i]) * 0.5);

				++i;
			}

			while (i < m1size)
			{
				m_master_vec.push_back(master1.m_master_vec[i]);

				++i;
			}
		}
		else
		{
			for (int i{}; i < m1size; ++i)
			{
				m_master_vec.push_back((master1.m_master_vec[i] + master2.m_master_vec[i]) * 0.5);
			}
		}
	}

	void write(std::ofstream& ofs)
	{
		auto amplmod{ pow(2, (bitdepth - 1)) - 1 };
		int mvec_size{ static_cast<int>(m_master_vec.size()) };

		for (int i{}; i < mvec_size; ++i)
		{
			auto fsample{ m_master_vec[i] * amplmod };
			int isample{ static_cast<int>(fsample) };

			ofs.write((reinterpret_cast<char*>(&isample)), 2);
		}

		m_postwr_pos = ofs.tellp();
	}
};

class Header
{
public:
	int m_preaudiop{};
	std::ofstream& ofs;

	Header(std::ofstream& tmpofs) : ofs{ tmpofs }
	{
	}

	template <typename T>
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

		m_preaudiop = ofs.tellp();
	}

	void header_c(int postaudiop)
	{
		ofs.seekp(m_preaudiop - 4);
		writeofs(ofs, postaudiop - m_preaudiop, 4);

		ofs.seekp(4, std::ios::beg);
		writeofs(ofs, postaudiop - 8, 4);
	}

};

int main()
{
	Note note{};

	note.get_note();

	std::cout << "The frequency of the specified note is: " << note.m_freq << "Hz\n";

	return 0;

}